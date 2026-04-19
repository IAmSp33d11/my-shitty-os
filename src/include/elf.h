#ifndef ELF_H
#define ELF_H

typedef struct {
    uint8_t magic[4];
    uint8_t bit_size; // 1 for 32 bit
    uint8_t endian; // 1 for little endian
    uint8_t header_ver; // ELF header version
    uint8_t OS_ABI; // OS ABI???
    uint64_t useless1; // Padding/Useless
    uint16_t type;
    uint16_t instruct_set; // ISA
    uint32_t elf_ver; // ELF version
    uint32_t prgm_entry_off; // Program entry offset
    uint32_t prgm_head_off; // Program Header table offset
    uint32_t sect_head_off; // Section Header table offset
    uint8_t flags[4]; // Undefined for x86-32
    uint16_t head_size; // ELF header size
    uint16_t prgm_entry_size; // Size of entry in program header table
    uint16_t num_entries_prgm; // Number of entries in program header table
    uint16_t sect_entry_size; // Size of entry in section header table
    uint16_t num_entries_sect; // Number of entries in section header table
    uint16_t sect_index_string; // Section index to the section header string table;
} __attribute__((packed)) elf_header_t;

typedef struct {
    uint32_t type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t flags;
    uint32_t alignment;
} __attribute__((packed)) prgm_header_t;

bool confirm_elf(const char *filename, uint8_t* buffer);
uint32_t* load_elf(const char *filename, uint8_t* buffer);

#endif