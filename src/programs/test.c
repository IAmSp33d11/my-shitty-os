#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "syscalls.h"
#include "string.h"

void main();

__attribute__((section(".text._start"))) void _start() {
    main();
    exit(0);
}




void main() {  
    write("Hello, User-space!\n");
}
