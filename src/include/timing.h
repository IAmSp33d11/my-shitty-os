#ifndef TIMING_H
#define TIMING_H

extern volatile uint32_t pit_ticks;
void sleep(uint32_t ms);
void sleep_seconds(uint32_t seconds);

#endif