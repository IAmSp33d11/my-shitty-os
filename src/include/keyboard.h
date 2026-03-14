#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
bool setup_PS2(void);
char poll_char(void);
extern volatile uint8_t key_buffer_length;
extern uint8_t* key_buffer;
extern volatile uint8_t key_buffer_read;
#endif