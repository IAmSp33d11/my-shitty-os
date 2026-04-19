#ifndef ISO_H
#define ISO_H

#define PVD_sector 0x10 // Primary Volume Descriptor Sector

typedef struct {
    uint32_t le;
    uint32_t be;
} __attribute__((packed)) both_endian_32;

typedef struct {
    uint16_t le;
    uint16_t be;
} __attribute__((packed)) both_endian_16;

typedef struct {
    char year[4];
    char month[2];
    char day[2];
    char hour[2];
    char minute[2];
    char second[2];
    char centisecond[2]; // A hundrendth of a second
    uint8_t offset;
} __attribute__((packed)) dec_datetime;

typedef struct {
    uint8_t type; // Always 0x01
    char identifier[5]; // Always 'CD001'
    uint8_t version; // Always 1
    uint8_t unused0; // Always 0
    char sys_identify[32]; // System identifier
    char vol_identify[32]; // Volume identifier
    uint64_t unused1; // Always 0
    both_endian_32 vol_space; // Volume Space Field
    uint8_t unused2[32]; // All 0s.
    both_endian_16 vol_set_size; // Volume Set Size
    both_endian_16 vol_seq_num; // Volume Sequence Number
    both_endian_16 log_block_size; // Logical Block Size
    both_endian_32 path_table_size; // Path Table Size
    uint32_t location_path_table; // Location of Path Table
    uint32_t option_path_table; // Location of Optional Path Table
    uint64_t unused3; // Technically not unused but we don't care about it as its MSB not LSB and its copies of above
    uint8_t root_dir_record[34]; // Root Directory Entry
    uint8_t vol_set_identify[128]; // Volume Set Identifier
    uint8_t pub_identify[128]; // Publisher Identifier
    uint8_t data_prepare_identify[128]; // Data Preparer Identifier
    uint8_t app_identify[128]; // Aplication Identifier
    uint8_t copy_identify[37]; // Copyright File Identifier
    uint8_t abs_identify[37]; // Abstract File Identifier
    uint8_t bib_identify[37]; // Bibliographic File Identifier???
    dec_datetime creation_date; // Creation Date. DUH
    dec_datetime modify_date; // Modified Date.
    dec_datetime obsolete_date; // Wtf does data being considered obsolete mean?!?
    dec_datetime effective_date; // Okay why do we have this?
    uint8_t file_struct_ver; // File Structure Version
    uint8_t unused4;
    uint8_t app_used[512]; // So... We just giving it 512 bytes for some damn reason lol?
    uint8_t reserved[653];
} __attribute__((packed)) PVD;

typedef struct {
    uint8_t length;
    uint8_t ext_length;
    both_endian_32 lba;
    both_endian_32 size;
    uint8_t date[7];
    uint8_t flags;
    uint8_t file_unit_size;
    uint8_t interleave_gap;
    both_endian_16 vol_seq_num;
    uint8_t name_length;
    char name[];
} __attribute__((packed)) iso_dirent_t;

typedef struct {
    uint32_t lba;
    uint32_t size;
} iso_file_t;

// Functions
bool confirm_pvd(PVD* thingy_i_still_cant_name);
void get_pvd(PVD* thingy_i_cant_name);
iso_file_t get_lba_file(const char* name);
bool read_file(const char* name, uint8_t* buffer);
int list_items(const char* dir, char** items);
uint32_t get_file_size(const char* name);

#endif