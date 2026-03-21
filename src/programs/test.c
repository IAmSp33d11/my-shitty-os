#include <stdint.h>

void main();

__attribute__((section(".text._start"))) void _start() {
    main();
    __asm__ volatile("mov $4, %%eax; int $0x30;" ::: "%eax"); // exit
}

void write(const char* string) {
    __asm__ volatile(
        "mov $1, %%eax;"
        "int $0x30;"
        :
        : "b"(string)
        : "%eax"
    );
}

void sleep(uint32_t a) {
    __asm__ volatile(
        "mov $2, %%eax;"
        "mov %0, %%ebx;"
        "int $0x30;"
        :
        : "r"(a)
        : "%eax", "ebx"
    );
}

void main() {
    write("Hello from Userspace!!");
    sleep(1000);
}



