#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "iso.h"
#include "paging.h"
#include "elf.h"


// Arguments
// Requires a pointer to a buffer of filename's size exactly (via malloc or something)
// Requires the file pointed to by filename to be an actual file on the disc.
bool confirm_elf(const char *filename, uint8_t* buffer) {
    if (!read_file(filename, buffer)) {
        return false;
    }
    elf_header_t* elf_header = (elf_header_t*) buffer;
    if (elf_header->magic[0] != 0x7F || elf_header->magic[1] != 'E' ||
         elf_header->magic[2] != 'L' || elf_header->magic[3] != 'F')
        return false;

    if (elf_header->endian != 1) {
        return false; // Its Big Endian???
    }

    if (elf_header->instruct_set != 0x03) {
        return false; // ITS NOT FOR X86?!?!?!
    }
    return true; // Temporary for now, this just checks magic number
}

// Arguments
// Requires a pointer to a buffer of filename's size exactly (via malloc or something)
// Requires the file pointed to by filename to be an actual file on the disc.
// Returns a pointer to the start of the code in the ELF
uint32_t* load_elf(const char *filename, uint8_t* buffer) {
    if (!confirm_elf(filename, buffer)) {
        return NULL;
    }
    elf_header_t* elf_header = (elf_header_t*) buffer;
    prgm_header_t* prgm_header_array = (prgm_header_t*)(buffer + elf_header->prgm_head_off);
    
    // Do da parsing loop :3
}

// UNIX SYSCALLS HERE
// https://pubs.opengroup.org/onlinepubs/9799919799/