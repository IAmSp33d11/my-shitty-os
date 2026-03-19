#include <stddef.h>
#include <stdint.h>
#include "port.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"
#include "timing.h"
#include "multiboot2.h"
#include "paging.h"
#include "disc.h"
#include "iso.h"

extern uint32_t kernel_end;

bool disc_connected;
char *vendor_id;

void run_command(char* parts[], int parts_length) {
	terminal_writestring("\n");
	if (string_equals(parts[0], "info")) {
		terminal_writestring("Your CPU was made by: ");
		terminal_writestring(vendor_id);
	} else if (string_equals(parts[0], "ver")) {
		terminal_writestring("Version 0.0.4 of an unnamed OS lol.\n(maybe if you see this suggest a name for it?)");
	} else if (string_equals(parts[0], "help")) {
		terminal_writestring("Okay so here are the we have lol :\nver - outputs the version number lol\ninfo - just shows the vendor-id of your cpu\ndiscinfo - shows if a disc is inserted or not.\nfindfile <filename> - tells you where on the disc a file is located and its sizereadfile <filename> - reads the contents of a file to the terminal\n(Only works with files containing text)");
	} else if (string_equals(parts[0], "secret")) {
		terminal_writestring("OMG YOU FOUND THE SECRET!\n:3");
	} else if (string_equals(parts[0], "discinfo")) {
		terminal_writestring("A disc is ");
		if (!disc_connected) {
			terminal_writestring("not ");
		}
		terminal_writestring("inserted.\n");
		if (disc_connected) {
			PVD pvd;
			get_pvd(&pvd);
			terminal_writestring("The disc's PVD is ");
			if (!confirm_pvd(&pvd)) {
				terminal_writestring("not ");
			}
			terminal_writestring("valid");
		}
	} else if (string_equals(parts[0], ":3")) {
		for (int i = 0; i < 5; i++) {
			terminal_writestring(":3\nUwU\n");
		} // if you see this, I was bored lol. UwU
	} else if (string_equals(parts[0], "findfile")) {
		if (parts_length == 1) {
			terminal_writestring("missing operand!");
		} else {
			uint32_t lba = get_lba_file(parts[1]).lba;
			uint32_t size = get_lba_file(parts[1]).size;
			if (lba == 0) {
				terminal_writestring("That file does not exist in the location specified.");
			} else {
				terminal_writestring("The file exists at LBA: 0x");
				char buf[32];
				itoa_hex(lba, buf);
				terminal_writestring(buf);
				terminal_writestring("\nThe file is ");
				itoa(size, buf);
				terminal_writestring(buf);
				terminal_writestring(" bytes large!");
			}
		}
	} else if (string_equals(parts[0], "cat")) {
		terminal_writestring("Meow :3\n\nWhat you expected this to be like the Linux 'cat'?\nNah its just a cat command :3");
	} else if (string_equals(parts[0], "readfile")) {
		if (parts_length == 1) {
			terminal_writestring("missing operand!");
		} else {
			uint8_t buf[4096];
			if (!read_file(parts[1], buf)) {
				terminal_writestring("File does not exist!");
			} else {
				terminal_writestring("\n");
				terminal_writestring((char*)buf);
				terminal_writestring("\n");
			}
		}
	} else {
		terminal_writestring("INVALID COMMAND\nUSE 'help' TO SEE THE DAMN COMMAND LIST!");
	}
	terminal_writestring("\n> ");
}

void kernel_main(void) {
	terminal_initialize();

	disc_connected = detect_discs();



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
				char* parts[16];
				int parts_length = strsplit(input_buffer, ' ', parts, 16);
				run_command(parts, parts_length);
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

