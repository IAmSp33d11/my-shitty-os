#include <stdint.h>
#include "port.h"

inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

inline void outw(uint16_t port, uint16_t val)
{
    __asm__ volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

inline uint16_t inw(uint16_t port)
{
    uint16_t ret;
    __asm__ volatile ( "inw %1, %0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

inline void outl(uint16_t port, uint32_t val)
{
    __asm__ volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

inline uint32_t inl(uint16_t port)
{
    uint32_t ret;
    __asm__ volatile ( "inl %1, %0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

inline void io_wait(void)
{
    outb(0x80, 0);
}

inline void insw(uint16_t __port, void *__buf, unsigned long __n) {
	__asm__ __volatile__("cld; rep; insw"
			: "+D"(__buf), "+c"(__n)
			: "d"(__port));
}

inline void outsw(uint16_t __port, const void *__buf, unsigned long __n) {
	__asm__ __volatile__("cld; rep; outsw"
			: "+S"(__buf), "+c"(__n)
			: "d"(__port));
}