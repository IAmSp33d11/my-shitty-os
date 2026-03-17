#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "port.h"

#define DATA_PORT   0x60 // Read/Write
#define STATUS_REG  0x64 // Read
#define COMMAND_REG 0x64 // Write

#define KEY_BUFFER_VAL 0xBBBBB
uint8_t* key_buffer = (uint8_t*) KEY_BUFFER_VAL;

volatile uint8_t key_buffer_length = 0;
volatile uint8_t key_buffer_read = 0;


bool setup_PS2(void) {
    // Disable the port(s)
    outb(COMMAND_REG, 0xAD);
    io_wait();
    outb(COMMAND_REG, 0xA7);
    io_wait();
    // Flush the output buffer
    inb(DATA_PORT);
    io_wait();
    // Get the controller config byte
    outb(COMMAND_REG, 0x20);
    io_wait();
    uint8_t config_byte = inb(DATA_PORT);
    io_wait();
    config_byte &= 0b10100110;
    outb(COMMAND_REG, 0x60);
    io_wait();
    outb(DATA_PORT, config_byte);
    io_wait();
    // Controller self test
    outb(COMMAND_REG, 0xAA);
    io_wait();
    if (inb(DATA_PORT) != 0x55) return false;
    io_wait();
    // Check if 2 channel PS/2
    bool dual_channel = false;
    outb(COMMAND_REG, 0xA8); // enable port 2 temporarily
    io_wait();
    outb(COMMAND_REG, 0x20);
    io_wait();
    config_byte = inb(DATA_PORT);
    io_wait();
    if ((config_byte & 0b00100000) == 0) {
        dual_channel = true; // bit 5 clear means port 2 clock got enabled = 2 channel controller
        outb(COMMAND_REG, 0xA7); // disable port 2 again for now
        io_wait();
    }
    // Test port 1
    outb(COMMAND_REG, 0xAB);
    io_wait();
    if (inb(DATA_PORT) != 0x00) return false;
    io_wait();
    // Test port 2 if present
    if (dual_channel) {
    outb(COMMAND_REG, 0xA9);
    io_wait();
    if (inb(DATA_PORT) != 0x00) dual_channel = false; // port 2 broken, ignore it
        io_wait();
    }
    // Enable port 1
    outb(COMMAND_REG, 0xAE);
    io_wait();
    // Enable port 2 if present
    if (dual_channel) {
        outb(COMMAND_REG, 0xA8);
        io_wait();
    }
    // Update config byte - enable interrupts for available ports
    outb(COMMAND_REG, 0x20);
    io_wait();
    config_byte = inb(DATA_PORT);
    io_wait();
    config_byte |= 0b00000001;          // port 1 interrupt enable
    config_byte &= ~0b00010000;         // port 1 clock enable
    if (dual_channel) {
        config_byte |= 0b00000010;      // port 2 interrupt enable
        config_byte &= ~0b00100000;     // port 2 clock enable
    }
    outb(COMMAND_REG, 0x60);
    io_wait();
    outb(DATA_PORT, config_byte);
    io_wait();
    // Reset port 1 device
    outb(DATA_PORT, 0xFF);
    io_wait();
    if (inb(DATA_PORT) != 0xFA) return false;
    io_wait();
    // Reset port 2 device if present
    if (dual_channel) {
    outb(COMMAND_REG, 0xD4); // next byte goes to port 2
    io_wait();
    outb(DATA_PORT, 0xFF);
    io_wait();
    if (inb(DATA_PORT) != 0xFA) dual_channel = false; // port 2 device broken, ignore
        io_wait();
    }
    return true;
}



static bool last_f0 = false;
static bool shift = false;
char poll_char(void) {
    uint8_t scancode;
    if (key_buffer_length > 0) {
        scancode = key_buffer[key_buffer_read++];
        key_buffer_length--;
    } else {
        return '\0';
    }



    if (scancode == 0xF0) {
        last_f0 = true;
        return '\0';
    }



    if (last_f0) {
        last_f0 = false;
        if (scancode == 0x12) {
            shift = false;
        }
        return '\0';
    }

    if (scancode == 0x12) {
        shift = true;
        return '\0';
    }

    if (scancode == 0x1C) return shift ? 'A' : 'a';
    if (scancode == 0x32) return shift ? 'B' : 'b';
    if (scancode == 0x21) return shift ? 'C' : 'c';
    if (scancode == 0x23) return shift ? 'D' : 'd';
    if (scancode == 0x24) return shift ? 'E' : 'e';
    if (scancode == 0x2B) return shift ? 'F' : 'f';
    if (scancode == 0x34) return shift ? 'G' : 'g';
    if (scancode == 0x33) return shift ? 'H' : 'h';
    if (scancode == 0x43) return shift ? 'I' : 'i';
    if (scancode == 0x3B) return shift ? 'J' : 'j';
    if (scancode == 0x42) return shift ? 'K' : 'k';
    if (scancode == 0x4B) return shift ? 'L' : 'l';
    if (scancode == 0x3A) return shift ? 'M' : 'm';
    if (scancode == 0x31) return shift ? 'N' : 'n';
    if (scancode == 0x44) return shift ? 'O' : 'o';
    if (scancode == 0x4D) return shift ? 'P' : 'p';
    if (scancode == 0x15) return shift ? 'Q' : 'q';
    if (scancode == 0x2D) return shift ? 'R' : 'r';
    if (scancode == 0x1B) return shift ? 'S' : 's';
    if (scancode == 0x2C) return shift ? 'T' : 't';
    if (scancode == 0x3C) return shift ? 'U' : 'u';
    if (scancode == 0x2A) return shift ? 'V' : 'v';
    if (scancode == 0x1D) return shift ? 'W' : 'w';
    if (scancode == 0x22) return shift ? 'X' : 'x';
    if (scancode == 0x35) return shift ? 'Y' : 'y';
    if (scancode == 0x1A) return shift ? 'Z' : 'z';
    if (scancode == 0x29) return ' ';
    if (scancode == 0x66) return '\b';
    if (scancode == 0x5A) return '\n';
    if (scancode == 0x49) return shift ? '>' : '.';
    if (scancode == 0x41) return shift ? '<' : ',';
    if (scancode == 0x0D) return '\t';
    if (scancode == 0x4C) return shift ? ':' : ';';
    if (scancode == 0x16) return shift ? '!' : '1';
    if (scancode == 0x1E) return shift ? '@' : '2';
    if (scancode == 0x26) return shift ? '#' : '3';
    if (scancode == 0x25) return shift ? '$' : '4';
    if (scancode == 0x2E) return shift ? '%' : '5';
    if (scancode == 0x36) return shift ? '^' : '6';
    if (scancode == 0x3D) return shift ? '&' : '7';
    if (scancode == 0x3E) return shift ? '*' : '8';
    if (scancode == 0x46) return shift ? '(' : '9';
    if (scancode == 0x45) return shift ? ')' : '0';
    if (scancode == 0x4E) return shift ? '_' : '-';
    if (scancode == 0x55) return shift ? '+' : '=';
    return '\0';
}

