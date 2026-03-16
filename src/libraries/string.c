#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "string.h"




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