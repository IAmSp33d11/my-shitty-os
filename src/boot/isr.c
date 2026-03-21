#include <stddef.h>
#include <stdint.h>
#include "port.h"
#include "keyboard.h"
#include "timing.h"
#include "vga.h"
#include "string.h"


size_t retAddr;
uint32_t EIP;
uint32_t CS;


extern void kernel_main(void);


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

extern void kernel_return(void);


void syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {

    if (eax == 1) { // Print
        terminal_writestring((char*) ebx);
    }
    if (eax == 2) { // Sleep
        sleep(ebx);
    }
    if (eax == 3) { // Sleep (seconds)
        sleep_seconds(ebx); 
    }
    if (eax == 4) { // Finished
        key_buffer_read = 0;
        key_buffer_length = 0;
        terminal_writestring("\n");
        kernel_return();

    }
    if (eax == 5) { // Clear Screen
        terminal_initialize();
    }
}