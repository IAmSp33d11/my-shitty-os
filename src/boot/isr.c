#include <stddef.h>
#include <stdint.h>
#include "port.h"
#include "keyboard.h"
#include "timing.h"
#include "vga.h"
#include "string.h"
#include "paging.h"
#include "iso.h"


size_t retAddr;
uint32_t EIP;
uint32_t CS;


extern void kernel_main(void);

typedef struct {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} interrupt_frame_t;

void divide_error(void) {
    // TODO : Actually make it do shit

}

void keyboard_handler(void) {
    uint8_t scancode = inb(0x60); // Get the scancode

    key_buffer[key_buffer_read + key_buffer_length] = scancode;
    key_buffer_length++;

    outb(0x20, 0x20);  // EOI to master PIC
}

void pit_handler(void) {
    pit_ticks++;
    outb(0x20, 0x20);
}

void irq_handler(void) {
    
}

void gpf_handler(interrupt_frame_t* frame) {
    char buf[32];
    itoa_hex(frame->eip, buf);
    terminal_writestring("EIP: ");
    terminal_writestring(buf);
    __asm__ volatile ("cli; hlt");
}

extern void kernel_return(void);


uint32_t syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
        if (eax == 1) { // exit
        key_buffer_read = 0;
        key_buffer_length = 0;
        terminal_writestring("\n");
        kernel_return();
    }
    if (eax == 2) { // malloc
        //return (uint32_t) umalloc(ebx);
    }
    if (eax == 3) { // temp (write_to_screen)
        terminal_writestring((char*) ebx);
    }
}