#include <stddef.h>
#include <stdint.h>
#include "port.h"
#include "vga.h"
#include "string.h"



static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return 
fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}



size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;

void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void scroll(void) {
	// Do not worry about the black magic happening in the conditonal.
	// It is stupid cause I was bored and had nothing better to do but make
	// my code worse for everyone (including myself) :3
	for (int i = 0; i < (uint8_t) (((uint8_t) VGA_HEIGHT) - 257); i++) {
		for (int j = 0; j < VGA_WIDTH; j++) {
			terminal_buffer[i * VGA_WIDTH + j] = terminal_buffer[(i + 1) * VGA_WIDTH + j];
		}
	}

	for (int i = 0; i < VGA_WIDTH; i++) {
		terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + i] = (uint16_t)(' ' | (0x07 << 8));
	}
}

void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
	update_cursor(x, y);
}

void terminal_putchar(char c) 
{
	if (c == '\n') {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT) {
			terminal_row--;
			scroll();
		}
		return;
	}
	if (c == '\t') {
		if (terminal_column + 4 > VGA_WIDTH) {
			terminal_column = VGA_WIDTH;
		} else {
			terminal_column += 4;
		}
		return;
	}


	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT) {
			terminal_row--;
			scroll();
		}
	}
}

void terminal_write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++) {
		terminal_putchar(data[i]);
	}
}

void terminal_writestring(const char* data) 
{
	terminal_write(data, strlen(data));
}

void update_cursor(int x, int y)
{
	uint16_t pos = y * VGA_WIDTH + x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) { 
// Okay so the name is misleading
// It changes the cursor shape??
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void disable_cursor()
{
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

void delete_last_char() {
    if (terminal_column == 0) {
        if (terminal_row > 0) {
            terminal_row--;
            terminal_column = VGA_WIDTH - 1;
            for (int i = terminal_column; i >= 0; i--) {
                if ((terminal_buffer[terminal_row * VGA_WIDTH + i] & 0xFF) != ' ') {
                    terminal_column = i + 1;
                    break;
                }
            }
        }
    } else {
        terminal_column--;
    }
    terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
}

// Okay So everything above this is text mode. (Mode 3)