#include <stddef.h>
#include <stdint.h>
#include "./../include/port.h"
#include "./../include/keyboard.h"
#include "./../include/timing.h"


size_t retAddr;
uint32_t EIP;
uint32_t CS;



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