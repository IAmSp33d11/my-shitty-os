#include <stddef.h>
#include <stdint.h>
#include "port.h"
#include "timing.h"

volatile uint32_t pit_ticks = 0;
// It can overflow, yes, BUT WHO IS WAITING LIKE A MONTH FOR THAT TO HAPPEN LOL???
// So we should be chill :3

void sleep(uint32_t ms) {
    uint32_t target = pit_ticks + ms;
    while (pit_ticks < target) {
        __asm__ volatile ("HLT");
    }
}


void sleep_seconds(uint32_t seconds) {
    uint32_t target = pit_ticks + (seconds * 1000);
    while (pit_ticks < target) {
        __asm__ volatile ("HLT");
    }
}