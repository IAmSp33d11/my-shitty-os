#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "disc.h"
#include "port.h"
#include "timing.h"
#include "iso.h"
#include "string.h"
#include "vga.h"

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

int list_items(const char* dir, char** items) {
    PVD pvd;
    get_pvd(&pvd);
    char filename[255];
    char* path[16];
    strncpy(filename, dir, 255);
    to_upper_case(filename);
    int a = strsplit(filename, '/', path, 16);
    
    iso_dirent_t* root = (iso_dirent_t*)pvd.root_dir_record;
    uint32_t root_lba = root->lba.le;

    uint16_t buffer[1024];
    atapi_read(root_lba, 1, buffer);
    uint8_t* buff_ptr = (uint8_t*) buffer; // This pointer is kinda buff
    uint32_t current_lba = root_lba;
    for (int i = 0; i < a; i++) {
        atapi_read(current_lba, 1, buffer);
        buff_ptr = (uint8_t*) buffer;
        
        bool found = false;
        while (*buff_ptr != 0) {
            iso_dirent_t* entry = (iso_dirent_t*)buff_ptr;
            uint8_t cmp_len = entry->name_length;
            if (cmp_len >= 2 && entry->name[cmp_len - 2] == ';') {
                cmp_len -= 2;
            }           
            if ((entry->flags & 0b10) && string_equals_len(entry->name, path[i], cmp_len)) {
                current_lba = entry->lba.le;
                found = true;
                break;
            }
            buff_ptr += entry->length;
        }
        if (!found) return -1;
    }


    atapi_read(current_lba, 1, buffer);
    buff_ptr = (uint8_t*)buffer;  
    int i = 0;
    while (*buff_ptr != 0) {
        
        iso_dirent_t* entry = (iso_dirent_t*)buff_ptr;

        if (entry->name_length == 1 && 
            (entry->name[0] == 0x00 || entry->name[0] == 0x01)) {
            buff_ptr += entry->length;
           continue;
        }
        uint8_t cmp_len = entry->name_length;
        if (cmp_len >= 2 && entry->name[cmp_len - 2] == ';') {
            cmp_len -= 2;
        }           
        strncpy(items[i], entry->name, cmp_len);
        items[i][cmp_len] = '\0';
        buff_ptr += entry->length;
        i++;
    }
    return i;
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
    char* path[16];
    strncpy(filename, name, 255);
    to_upper_case(filename);
int a = strsplit(filename, '/', path, 16);
char last[255];
strncpy(last, path[a-1], 255); // save the filename
a--;
    
    iso_dirent_t* root = (iso_dirent_t*)pvd.root_dir_record;
    uint32_t root_lba = root->lba.le;

    uint16_t buffer[1024];
    atapi_read(root_lba, 1, buffer);
    uint8_t* buff_ptr = (uint8_t*) buffer; // This pointer is kinda buff
    uint32_t current_lba = root_lba;
    for (int i = 0; i < a; i++) {
        atapi_read(current_lba, 1, buffer);
        buff_ptr = (uint8_t*) buffer;
        

        while (*buff_ptr != 0) {
            iso_dirent_t* entry = (iso_dirent_t*)buff_ptr;
            uint8_t cmp_len = entry->name_length;
            if (cmp_len >= 2 && entry->name[cmp_len - 2] == ';') {
                cmp_len -= 2;
            }           
            if ((entry->flags & 0b10) && string_equals_len(entry->name, path[i], cmp_len)) {
                current_lba = entry->lba.le;
                break;
            }
            buff_ptr += entry->length;
        }
    }

    atapi_read(current_lba, 1, buffer);
    buff_ptr = (uint8_t*)buffer;   
    while (*buff_ptr != 0) {
        iso_dirent_t* entry = (iso_dirent_t*)buff_ptr;
        uint8_t cmp_len = entry->name_length;
        if (cmp_len >= 2 && entry->name[cmp_len - 2] == ';') {
            cmp_len -= 2;
        }    
        if (!(entry->flags & 0b10) && string_equals_len(entry->name, last, cmp_len)) {
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