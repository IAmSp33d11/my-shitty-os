#ifndef TIMING_H
#define TIMING_H

extern volatile uint32_t pit_ticks;
void kernel_sleep(uint32_t ms);
void kernel_sleep_seconds(uint32_t seconds);
void sleep(uint32_t ms);
void sleep_seconds(uint32_t seconds);

#endif