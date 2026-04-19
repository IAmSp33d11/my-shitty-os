#ifndef SYSCALLS_H
#define SYSCALLS_H


void write(const char* string) {
    __asm__ volatile(
        "mov $3, %%eax;"
        "int $0x30;"
        :
        : "b"(string)
        : "%eax"
    );
}

void exit(uint32_t status) {
    __asm__ volatile(
        "mov $1, %%eax;"
        "int $0x30;"
        :
        : "b"(status)
        : "%eax"
    );
}




#endif