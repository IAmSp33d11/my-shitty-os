#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "./../include/port.h"

#define DATA_PORT   0x60 // Read/Write
#define STATUS_REG  0x64 // Read
#define COMMAND_REG 0x64 // Write

#define KEY_BUFFER_VAL 0xBBBBB
uint8_t* key_buffer = (uint8_t*) KEY_BUFFER_VAL;

volatile uint16_t key_buffer_length = 0;


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

char poll_char(void) {
    uint8_t scancode;
    if (key_buffer_length > 0) {
        scancode = key_buffer[--key_buffer_length];
    } else {
        return 0;
    }


    if (scancode == 0x1C)
        return 'A';
    return '?';
}

