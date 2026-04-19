#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "port.h"
#include "keyboard.h"
#include "vga.h"
#include "paging.h"
#include "multiboot2.h"
#include "string.h"


extern uint64_t max_addr;
extern uint32_t total_pages;
// Annoying stuff
void multiboot_shit(uint32_t magic, uint32_t mb2_addr) {
    struct multiboot_tag *tag;
	unsigned size;

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        while (true) {
            __asm__ volatile  ("hlt");
        } // Keep the CPU trapped forever MWA HA HA HA HA >:3
    }
    size = *(unsigned *) (mb2_addr + KERNEL_OFFSET);
	for (tag = (struct multiboot_tag *) (mb2_addr + 8 + KERNEL_OFFSET);
       tag->type != MULTIBOOT_TAG_TYPE_END;
       tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag 
                                       + ((tag->size + 7) & ~7))) 
		{
			switch (tag->type) {
				case MULTIBOOT_TAG_TYPE_CMDLINE:
					break;
				case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
					break;
				case MULTIBOOT_TAG_TYPE_MODULE:
					break;
				case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
					break;
				case MULTIBOOT_TAG_TYPE_BOOTDEV:
					break;
				case MULTIBOOT_TAG_TYPE_MMAP:
					{

						multiboot_memory_map_t *mmap;
						for (mmap = ((struct multiboot_tag_mmap *) tag)->entries;
                 (multiboot_uint8_t *) mmap 
                   < (multiboot_uint8_t *) tag + tag->size;
                 mmap = (multiboot_memory_map_t *) 
                   ((unsigned long) mmap
                    + ((struct multiboot_tag_mmap *) tag)->entry_size)) {
						if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
							free_regions[free_region_count].base = mmap->addr;
							free_regions[free_region_count++].length = mmap->len;
						}
					}
					}
                    {
                        for (uint32_t i = 0; i < free_region_count; i++) {
                            uint64_t end_addr = free_regions[i].base + free_regions[i].length;
                            if (end_addr > max_addr) max_addr = end_addr;
                        }
                        total_pages = (uint32_t) (max_addr / 4096);
                    }
					break;
				case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
					break;
			}
		}
}

#define GDT_LOCATION   0xC0050000
#define IDT_MAX_DESCRIPTORS 49


struct GDT_entry {
    uint64_t base;
    uint32_t limit;
    uint8_t access_byte;
    uint8_t flags;
};

typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) tss_t;

tss_t tss;

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

extern uint32_t stack_top;
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

    // User Mode Code Segment
    entry.base = 0x0;
    entry.limit = 0xFFFFF;
    entry.access_byte = 0xFA;
    entry.flags = 0xC;
    encodeGdtEntry(gdt_pointer + 0x18, entry);

    // User Mode Data Segment
    entry.base = 0x0;
    entry.limit = 0xFFFFF;
    entry.access_byte = 0xF2;
    entry.flags = 0xC;
    encodeGdtEntry(gdt_pointer + 0x20, entry);

    // TSS
    entry.base = (uint32_t) &tss;
    entry.limit = sizeof(tss_t) - 1;
    entry.access_byte = 0x89;
    entry.flags = 0x0;
    encodeGdtEntry(gdt_pointer + 0x28, entry);
    tss.esp0 = (uint32_t) &stack_top;
    tss.ss0 = 0x10; // kernel data segment

    setGdt();
    __asm__ volatile ("ltr %%ax" : : "a"(0x28));
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
void exception_handler(uint32_t vector);
void exception_handler(uint32_t vector) {
    uint32_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    terminal_writestring("EXCEPTION: ");
    char buf[32];
    itoa(vector, buf);
    terminal_writestring(buf);
    terminal_writestring(" CR2: 0x");
    itoa_hex(cr2, buf);
    terminal_writestring(buf);
    terminal_writestring("\n");
    __asm__ volatile ("cli; hlt");
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

    idt_set_descriptor(0x30, isr_stub_table[0x30], 0xEE); // Let the userspace call 0x30

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
// March 15, 2026. I finally understand this fucking comment.
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
        "mov $0x36, %%al;"
        "out %%al, $0x43;"
        "mov $0xA9, %%al;"
        "out %%al, $0x40;"
        "mov $0x04, %%al;"
        "out %%al, $0x40;"
        :
        :
        : "%ax", "%dx", "%al"
    );
}

#define kernel_page_tables 128
#define user_page_tables 256

extern uint32_t boot_page_directory[];

uint32_t* page_directory;
void setup_paging(void) {
    setup_bitmap();
    page_directory = boot_page_directory;
    // Time do to some sketchy shit.
    // :3

    static uint32_t* kernel_pt[kernel_page_tables];
        for (int i = 0; i < kernel_page_tables; i++) {
        kernel_pt[i] = (uint32_t*) alloc_frame();
    }    
    static uint32_t* user_pt[user_page_tables];
    for (int i = 0; i < user_page_tables; i++) {
        user_pt[i] = (uint32_t*) alloc_frame();
    }
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < kernel_page_tables; j++) {
            uint32_t* kernel_pt_virt = (uint32_t*)((uint32_t)kernel_pt[j] + KERNEL_OFFSET);
            kernel_pt_virt[i] = ((j * 0x400000) + (i * 0x1000)) | 0x3; // Kernel
        }
        for (int j = 0; j < user_page_tables; j++) {
            uint32_t* user_pt_virt = (uint32_t*)((uint32_t)user_pt[j] + KERNEL_OFFSET);
            user_pt_virt[i] = (((j + 1) * 0x400000) + (i * 0x1000)) | 0x7; // User
        }
        
    }
    for (int i = 0; i < user_page_tables; i++) {
        page_directory[i+1] = (uint32_t) user_pt[i] | 0x7;
    }
    for (int i = 0; i < kernel_page_tables; i++) {
        page_directory[i+768] = (uint32_t) kernel_pt[i] | 0x3;
    }
    
    // map the page directory to itself
    page_directory[1023] = ((uint32_t)page_directory - KERNEL_OFFSET) | 0x3;
    // The snake is eating its own tail.

    __asm__ volatile(
        "mov %0, %%eax;"
        "mov %%eax, %%cr3;"
        "mov %%cr0, %%eax;"
        "or $0x80000001, %%eax;"
        "mov %%eax, %%cr0;"
        :
        : "r"(((uint32_t) page_directory)-KERNEL_OFFSET)
        : "%eax"
    );
}

// Start setting up everything
void boot_main(void) {
    terminal_initialize();
    terminal_writestring("Creating the GDT...\n");
    createGdt();
    terminal_writestring("Remapping the PIC...\n");
    PIC_remap(0x20, 0x28);
    terminal_writestring("Setting up the IDT...\n");
    idt_init();
    terminal_writestring("Setting up the PIT...\n");
    pit_init();
    terminal_writestring("Setting up paging...\n");
    setup_paging(); // NOTE TO SELF : DO NOT DISABLE THIS, THIS KILLS USERSPACE
    return;
}