#include <stddef.h>
#include <stdint.h>
#include "./../include/port.h"
#include "./../include/vga.h"
#include "./../include/keyboard.h"
#include "./../include/string.h"
#include "./../include/timing.h"



void kernel_main(void) {
	terminal_initialize();

	setup_PS2();
	disable_cursor();
	
	while (true) {
		if (key_buffer_length != 0) {
			char b = poll_char();
			char a[] = {b, '\0'};
			if (b == '\0') {
				continue;
			}
			if (b == '\b') {
				delete_last_char();
				continue;
			}
			terminal_writestring(a);
		}
	}
}