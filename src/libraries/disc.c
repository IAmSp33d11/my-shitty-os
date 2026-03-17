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

