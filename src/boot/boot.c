#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "./../include/port.h"
#include "./../include/keyboard.h"
#include "./../include/vga.h"

#define GDT_LOCATION   0x50000
#define IDT_MAX_DESCRIPTORS 34
#define ERROR_LOCATION_ADDR 0xEEEE

uint8_t* error_location = (uint8_t*) ERROR_LOCATION_ADDR;

struct GDT_entry {
    uint64_t base;
    uint32_t limit;
    uint8_t access_byte;
    uint8_t flags;
};

void encodeGdtEntry(uint8_t *target, struct GDT_entry source)
{
    // Check the limit to make sure that it can be encoded
    if (source.limit > 0xFFFFF) {return;}
    
    // Encode the limit
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    
    // Encode the base
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4] = (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF;
    
    // Encode the access byte
    target[5] = source.access_byte;
    
    // Encode the 6th byte weirdly.
    target[6] = ((source.limit >> 16) & 0xF) | (source.flags << 4);
}


extern void setGdt();


void createGdt(void) {
    uint8_t* gdt_pointer = (uint8_t*) GDT_LOCATION;
    struct GDT_entry entry;


    // Null Descriptor
    entry.base = 0x0;
    entry.limit = 0x00000;
    entry.access_byte = 0x00;
    entry.flags = 0x0;
    encodeGdtEntry(gdt_pointer, entry);

    // Kernel Mode Code Segment
    entry.base = 0x0;
    entry.limit = 0xFFFFF;
    entry.access_byte = 0x9A;
    entry.flags = 0xC;
    encodeGdtEntry(gdt_pointer + 0x08, entry);

    // Kernel Mode Data Segment
    entry.base = 0x0;
    entry.limit = 0xFFFFF;
    entry.access_byte = 0x93;
    entry.flags = 0xC;
    encodeGdtEntry(gdt_pointer + 0x10, entry);



    setGdt();
    return;
}


// Interrupts Shit Below
typedef struct {
    uint16_t isr_low;
    uint16_t kernel_cs;
    uint8_t reserved;
    uint8_t attributes;
    uint16_t isr_high;
} __attribute__((packed)) idt_entry_t;

__attribute__((aligned(0x10))) 
static idt_entry_t idt[256];

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

static idtr_t idtr;

// ISR shit
__attribute__((noreturn))
void exception_handler(void);
void exception_handler() {
    __asm__ volatile ("cli; hlt"); // Completely hangs the computer
}


void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low = (uint32_t)isr & 0xFFFF;
    descriptor->kernel_cs = 0x08; // Offset of kernel code selector
    descriptor->attributes = flags;
    descriptor->isr_high = (uint32_t)isr >> 16;
    descriptor->reserved = 0;
}


static bool vectors[IDT_MAX_DESCRIPTORS];

extern void* isr_stub_table[];

void idt_init(void);
void idt_init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

    for (uint8_t vector = 0; vector < IDT_MAX_DESCRIPTORS; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}

// Reprogram the PIC???
#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2+1)

#define PIC_EOI 0x20

void PIC_sendEOI(uint8_t irq)
{
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);
    
    outb(PIC1_COMMAND, PIC_EOI);
}

// Reinitialize the PIC???
#define ICW1_ICW4 0x01
#define ICW1_SINGLE 0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL 0x08
#define ICW1_INIT 0x10

#define ICW4_8086 0x01
#define ICW4_AUTO 0x02
#define ICW4_BUF_SLAVE 0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM 0x10

#define CASCADE_IRQ 2

// Offset 1 is vector offset for master
// Offset1..offset1+7 or something
// Offset 2 is vector offset for slave
// Offset2..offset2+7 or something
// Maybe later I'll actually understand this comment lol
void PIC_remap(int offset1, int offset2) {
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); // starts initialization (in cascade mode)
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, offset1); // ICW2: Master PIC vector offset
    io_wait();
    outb(PIC2_DATA, offset2); // ICW2: Slave PIC vector offset
    io_wait();
    outb(PIC1_DATA, 1 << CASCADE_IRQ); // ICW3: Tell Master PIC that there is a slave PIC at IRQ2 (0000 0010)
    io_wait();
    outb(PIC2_DATA, 2); // ICW3: Tell Slave Pic its cascade identity (0000 0010)
    io_wait();

    outb(PIC1_DATA, ICW4_8086); // ICW4: have the PICs use 8086 mode (and not 8088 mode)
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // Unmask both PICS.
    outb(PIC1_DATA, 0);
    outb(PIC2_DATA, 0);
}

void PIC_disable(void) {
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

void IRQ_set_mask(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;

    if (IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);
}

void IRQ_clear_mask(uint8_t IRQline) {
    uint16_t port; 
    uint8_t value;

    if (IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);
}

#define PIC_READ_IRR                0x0a    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR                0x0b    /* OCW3 irq service next CMD read */

/* Helper func */
static uint16_t __pic_get_irq_reg(int ocw3)
{
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

/* Returns the combined value of the cascaded PICs irq request register */
uint16_t pic_get_irr(void)
{
    return __pic_get_irq_reg(PIC_READ_IRR);
}

/* Returns the combined value of the cascaded PICs in-service register */
uint16_t pic_get_isr(void)
{
    return __pic_get_irq_reg(PIC_READ_ISR);
}

void pit_init(void) {
    __asm__ volatile(
        "mov $1193, %%dx;"
        "mov $0b110110, %%al;"
        "out %%al, $0x43;"
        "mov %%dx, %%ax;"
        "out %%al, $0x40;"
        "xchg %%al, %%ah;"
        "out %%al, $0x40;"
        :
        :
        : "%ax", "%dx", "%al"
    );
}

// Start setting up everything
void boot_main(void) {
    createGdt();
    PIC_remap(0x20, 0x28);
    idt_init();
    pit_init();
    return;
}