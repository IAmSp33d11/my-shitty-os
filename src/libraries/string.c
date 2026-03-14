#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "./../include/string.h"


size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}




bool stringEquals(const char* string1, const char* string2) {
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