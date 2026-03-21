#include <stddef.h>
#include <stdint.h>
#include "port.h"
#include "timing.h"

volatile uint32_t pit_ticks = 0;
// It can overflow, yes, BUT WHO IS WAITING LIKE A MONTH FOR THAT TO HAPPEN LOL???
// So we should be chill :3

void kernel_sleep(uint32_t ms) {
    uint32_t target = pit_ticks + ms;
    while (pit_ticks < target) {
        __asm__ volatile ("HLT");
    }
}


void kernel_sleep_seconds(uint32_t seconds) {
    uint32_t target = pit_ticks + (seconds * 1000);
    while (pit_ticks < target) {
        __asm__ volatile ("HLT");
    }
}

void sleep(uint32_t ms) {
    uint32_t target = pit_ticks + ms;
    while (pit_ticks < target) {
        // Well we can't hlt cause this is the usermode version and its being called inside an
        // interrupt handler so...
    }
}

void sleep_seconds(uint32_t seconds) {
    uint32_t target = pit_ticks + (seconds * 1000);
    while (pit_ticks < target) {
        // Well we can't hlt cause this is the usermode version and its being called inside an
        // interrupt handler so...
    }
}