#include <stddef.h>
#include "./../include/stdint.h"
#include "./../include/port.h"
#include "./../include/vga.h"



void kernel_main(void) 
{
	terminal_initialize();

	update_cursor(6, 9);

	terminal_writestring("Hello, World!\nHello");
	terminal_putentryat('A', VGA_COLOR_WHITE, 6, 9);


}