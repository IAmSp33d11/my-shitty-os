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

	terminal_writestring("Waiting for 1 second\n");
	sleep(1000);
	terminal_writestring("We waited successfully!\n");
	
	/*while (true) {
		if (key_buffer_length != 0) {
			char a[] = {poll_char(), ' ', '\0'};
			terminal_writestring(a);
			terminal_writestring("We got a KEYPRESS OMGOMGOMGOMG\n");
			
		}
	}*/
}