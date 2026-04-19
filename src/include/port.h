#ifndef PORT_H
#define PORT_H

#include <stdint.h>
void outb(uint16_t port, uint8_t val);
void outw(uint16_t port, uint16_t val);
void outl(uint16_t port, uint32_t val);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);
void io_wait(void);
void insw(uint16_t __port, void *__buf, unsigned long __n);
void outsw(uint16_t __port, const void *__buf, unsigned long __n);

#endif