#ifndef STRING_H
#define STRING_H
#include <stdbool.h>

size_t strlen(const char* str);
bool string_equals(const char* string1, const char* string2);
void itoa(uint32_t n, char s[]);
void itoa_hex(uint32_t n, char s[]);
void reverse(char s[]);
void to_lower_case(char s[]);
void to_upper_case(char s[]);
bool string_equals_len(const char* string1, const char* string2, size_t len);
char* strncpy(char* dest, const char* src, size_t n);
int strsplit(char* str, char delim, char** parts, uint32_t max_parts);
#endif