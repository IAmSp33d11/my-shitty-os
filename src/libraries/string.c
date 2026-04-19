#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "string.h"

// Trims a string to the length provided in len and returns 0
// Otherwise returns 1 if string is already at that length or
// Is below it
int trim_to_len(char* str, uint32_t len) {
    // This is horribly inefficient
    if (strlen(str) <= len) {
        return 1;
    } else {
        for (int i = strlen(str); ; i--) {
            str[i] = '\0';
            if (strlen(str) <= len) break;
        }
    }
    return 0;
}

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

bool string_equals(const char* string1, const char* string2) {
    if (strlen(string1) != strlen(string2)) {
        return false;
    }
    for (size_t i = 0; i < strlen(string1); i++) {
        if (string1[i] != string2[i]) {
            return false;
        }
    }
    return true;
}


bool string_equals_len(const char* string1, const char* string2, size_t len) {
    if (strlen(string2) != len) return false;
    for (size_t i = 0; i < len; i++) {
        if (string1[i] != string2[i]) {
            return false;
        }
    }
    return true;
}

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    return dest;
}

char* strcpy(char* dest, const char* src) {
    size_t i;
    for (i = 0; src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    return dest;
}

void* memcpy(void* dest, const void* src, size_t n) {
    size_t i;
    uint8_t* d = (uint8_t*) dest;
    const uint8_t* s = (const uint8_t*) src;
    for (i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return d;
}

char* concat(char* dest, const char* src) {
    size_t i;
    size_t len = strlen(dest) + 1;
    for (i = 0; src[i] != '\0'; i++) {
        dest[i + len] = src[i];
    }
    dest[i + 1] = '\0';
    return dest;
}

void itoa(uint32_t n, char s[])
{
    uint32_t i;

    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    s[i] = '\0';
    reverse(s);
}

void itoa_hex(uint32_t n, char s[]) {
    uint32_t i;
    i = 0;
    do {
        int digit = n % 16;
        if (digit < 10)
            s[i++] = digit + '0';
        else
            s[i++] = digit - 10 + 'A';
    } while ((n /= 16) > 0);
    s[i] = '\0';
    reverse(s);
}

void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void to_upper_case(char s[]) {
    for (int i = 0; i < strlen(s); i++) {
        if (s[i] >= 'a' && s[i] <= 'z') {
            s[i] -= 32;
        }
    }
}

void to_lower_case(char s[]) {
    for (int i = 0; i < strlen(s);i++) {
        if (s[i] >= 'A' && s[i] <= 'Z') {
            s[i] += 32;
        }
    }
}

int strsplit(char* str, char delim, char** parts, uint32_t max_parts) {
    uint32_t count = 0;
    int i = 0;
    
    // skip leading delimiter (handles leading "/")
    if (str[0] == delim) i++;
    
    if (str[i] != '\0') parts[count++] = str + i;
    
    for (; str[i] != '\0' && count < max_parts; i++) {
        if (str[i] == delim) {
            str[i] = '\0';
            if (str[i+1] != '\0' && str[i+1] != delim)
                parts[count++] = str + i + 1;
        }
    }
    return count;
}

// okay this isn't really string only stuff anymore but eh.
// What you gonna do about it?
uintptr_t virt_to_phys(uintptr_t virt) {
    return (virt - 0xC0000000);
}

uintptr_t phys_to_virt(uintptr_t phys) {
    return (phys + 0xC0000000);
}