#ifndef STRING_H
#define STRING_H
#include <stdbool.h>
#include <stddef.h>

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
int trim_to_len(char* str, uint32_t len);
char* concat(char* dest, const char* src);
void* memcpy(void* dest, const void* src, size_t n);
uintptr_t virt_to_phys(uintptr_t virt);
uintptr_t phys_to_virt(uintptr_t phys);

#endif