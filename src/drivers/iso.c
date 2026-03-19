#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "disc.h"
#include "port.h"
#include "timing.h"
#include "iso.h"
#include "string.h"

void get_pvd(PVD* thingy_i_cant_name) {
    atapi_read(PVD_sector, 1, (uint16_t*) thingy_i_cant_name);
}

bool confirm_pvd(PVD* thingy_i_still_cant_name) {
    if (thingy_i_still_cant_name->type!=0x01 || thingy_i_still_cant_name->version!=1
    || !(string_equals_len(thingy_i_still_cant_name->identifier, "CD001", 5))) {
        return false;
    }
    return true;
}

bool read_file(const char* name, uint8_t* buffer) {
    iso_file_t file = get_lba_file(name);
    if (file.size == 0 && file.lba == 0) {
        return false;
    }
    uint32_t sectors = (file.size + 2047) / 2048; // We do some rounding :3
    atapi_read(file.lba, sectors, (uint16_t*) buffer);
    return true;
}

iso_file_t get_lba_file(const char* name) {
    PVD pvd;
    get_pvd(&pvd);
    char filename[255];
    strncpy(filename, name, 255);
    to_upper_case(filename);
    iso_dirent_t* root = (iso_dirent_t*)pvd.root_dir_record;
    uint32_t root_lba = root->lba.le;

    uint16_t buffer[1024];
    atapi_read(root_lba, 1, buffer);

    uint8_t* buff_ptr = (uint8_t*) buffer; // This pointer is kinda buff
    while (*buff_ptr != 0) {
        iso_dirent_t* entry = (iso_dirent_t*)buff_ptr;

        if (string_equals_len(entry->name,filename, entry->name_length-2)) {
            iso_file_t out;
            out.lba = entry->lba.le;
            out.size = entry->size.le;
            return out;
        }
        buff_ptr += entry->length;
    }
    iso_file_t out;
    out.lba = 0;
    out.size = 0;
    return out;
}