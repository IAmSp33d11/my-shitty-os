#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "disc.h"
#include "port.h"
#include "timing.h"
#include "vga.h"


#define primary_io_base 0x1F0 
#define primary_ctrl_base 0x3F6
#define secondary_io_base 0x170
#define secondary_ctrl_base 0x376
/*  Okay so here is a fucking link to a graph since I know future me is gonna think present me
    was crazy - https://osdev.wiki/wiki/ATA_PIO_Mode#Registers
*/

uint8_t get_status(uint16_t ctrl_base) {
    for (int i = 0; i <= 14; i++) {
        inb(ctrl_base+0); // Do the 400ns wait
    }
    return inb(ctrl_base+0); // Return the status register
}

void wait_400ns() {
    for (int i = 0; i <= 14; i++) {
        inb(secondary_ctrl_base+0); // Do the 400ns wait
    }
}

bool detect_discs() {
    outb(secondary_io_base+6, 0xA0); // select the 'master' drive (kinda racist if you think about it)
    uint8_t status = get_status(secondary_ctrl_base);
    if (status == 0xFF) {
        return false;
    }
    outb(secondary_ctrl_base+0, 0x02); // STOP THE FUCKING INTERRUPTS
    
    do {
        status = inb(secondary_ctrl_base);
    } while (status & 0x88); // Wait for BSY and DRQ to clear.

    // We had to use this because QEMU is annoying. DON'T DO THIS. ITS HORRIBLE. WHY ARE WE
    // INTENTIONALLY CAUSING AN ERROR?!?!
    outb(secondary_io_base+7, 0xEC); // If it responds with an error, its an optical drive (ATAPI)

    sleep(10);
    status = get_status(secondary_ctrl_base); // Give the drive ~10ms to respond
    bool BSY = status >> 7;
    if (BSY) return false; // Drive didn't respond D:
    if (inb(secondary_io_base+4) == 0x14 && inb(secondary_io_base+5) == 0xEB) {
        return true;
    } else {
        return false;
    }
}

int atapi_read(uint32_t lba, uint32_t sectors, uint16_t* buffer) {
    volatile uint8_t read_cmd[12] = {0xA8, 0,
	                            (lba >> 0x18) & 0xFF, (lba >> 0x10) & 0xFF, (lba >> 0x08) & 0xFF,
	                            (lba >> 0x00) & 0xFF,
	                            (sectors >> 0x18) & 0xFF, (sectors >> 0x10) & 0xFF, (sectors >> 0x08) & 0xFF,
	                            (sectors >> 0x00) & 0xFF,
	                            0, 0};
    
    outb(secondary_io_base+6, 0xA0); // select the 'master' drive (kinda racist if you think about it)
    wait_400ns();
    outb(secondary_ctrl_base+0, 0x02); // STOP THE FUCKING INTERRUPTS
    wait_400ns();
    outb(secondary_io_base+1, 0x00); // Write 0 to Error Register
    outb(secondary_io_base+4, 2048 & 0xFF);
    outb(secondary_io_base+5, 2048 >> 8);
    outb(secondary_io_base+7, 0xA0); // Send the packet command
    wait_400ns();

    // Wait for the Status...
    while (true) {
        uint8_t status = get_status(secondary_ctrl_base);
        // Hey did you know Chuck Norris' keyboard doesn't have a ctrl button?

        if ((status & 0x01) == 1) {
            return 1; // Why we are doing this? I dunno, I'm just looking at the code example.
            // I believe its an error code since it returns 0 at the end of it so.... 
        }
        if (!(status & 0x80) && (status & 0x08)) {
            break; // Well we got the status so...
        }
    }

    outsw(secondary_io_base+0, (uint16_t*) read_cmd, 6); // send the command

    // Read the output?
    for (int i = 0; i < sectors; i++) {
        // Wait until its ready.
        while (true) {
            uint8_t status = get_status(secondary_ctrl_base);
            // It's cause he is always in control.

            if ((status & 0x01) == 1) {
                return 1; // Why we are doing this? I dunno, I'm just looking at the code example.
                // I believe its an error code since it returns 0 at the end of it so.... 
            }
            if (!(status & 0x80) && (status & 0x08)) {
                break; // Well we got the status so...
            }
        }

        int size = inb(secondary_io_base+5) << 8 | inb(secondary_io_base+4); // get the size of the transfer
        insw(secondary_io_base + 0, (uint16_t*) ((uint16_t*) buffer + i * 0x800), size / 2); // Read it
    }
    return 0;
}