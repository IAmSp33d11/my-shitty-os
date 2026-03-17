#include <stddef.h>
#include <stdint.h>
#include "port.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"
#include "timing.h"
#include "multiboot2.h"
#include "paging.h"

extern uint32_t kernel_end;


char *vendor_id;

void run_command(const char* command) {
	terminal_writestring("\n");
	if (string_equals(command, "info")) {
		terminal_writestring("Your CPU was made by: ");
		terminal_writestring(vendor_id);
	} else if (string_equals(command, "ver")) {
		terminal_writestring("Version 0.0.2 of an unnamed OS lol.\n(maybe if you see this suggest a name for it?)");
	} else if (string_equals(command, "help")) {
		terminal_writestring("Okay so here are the 2 fucking commands we have lol :\nver - outputs the version number lol\ninfo - just shows the vendor-id of your cpu");
	} else if (string_equals(command, "secret")) {
		terminal_writestring("OMG YOU FOUND THE SECRET!\n:3");
	} else {
		terminal_writestring("INVALID COMMAND\nUSE 'help' TO SEE THE DAMN COMMAND LIST!");
	}
	terminal_writestring("\n> ");
}

void kernel_main(void) {

	terminal_initialize();

	
	setup_PS2();

	vendor_id = (char*) 0xCCCC0;
	terminal_writestring("> ");
	
	char input_buffer[256];
	uint16_t input_size = 0;
	while (true) {
		if (key_buffer_length != 0) {
			char b = poll_char();
			char a[] = {b, '\0'};
			if (b == '\0') {
				continue;
			}
			if (b == '\b') {
				delete_last_char();
				input_buffer[input_size] = '\0';
				input_size--;
				continue;
			}
			if (b == '\n') {
				to_lower_case(input_buffer);
				run_command(input_buffer);
				for (int i = 0; i < input_size; i++) {
					input_buffer[i] = '\0';
				}
				input_size = 0;
				continue;
			}
			input_buffer[input_size++] = b;
			
			
			terminal_writestring(a);
		}
	}
}

