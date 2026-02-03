#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define GDT_LOCATION   0x50000
#define IDT_MAX_DESCRIPTORS 32

struct GDT_entry {
    uint64_t base;
    uint32_t limit;
    uint8_t access_byte;
    uint8_t flags;
};

void encodeGdtEntry(uint8_t *target, struct GDT_entry source)
{
    // Check the limit to make sure that it can be encoded
    if (source.limit > 0xFFFFF) {return;}
    
    // Encode the limit
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    // target[6] = (source.limit >> 16) & 0x0F;
    
    // Encode the base
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4] = (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF;
    
    // Encode the access byte
    target[5] = source.access_byte;
    
    // Encode the 6th byte weirdly.
    target[6] = ((source.limit >> 16) & 0xF) | (source.flags << 4);
}


extern void setGdt();



void createGdt(void) {
    uint8_t* gdt_pointer = (uint8_t*) GDT_LOCATION;
    struct GDT_entry entry;


    // Null Descriptor
    entry.base = 0x0;
    entry.limit = 0x00000;
    entry.access_byte = 0x00;
    entry.flags = 0x0;
    encodeGdtEntry(gdt_pointer, entry);

    // Kernel Mode Code Segment
    entry.base = 0x0;
    entry.limit = 0xFFFFF;
    entry.access_byte = 0x9A;
    entry.flags = 0xC;
    encodeGdtEntry(gdt_pointer + 0x08, entry);

    // Kernel Mode Data Segment
    entry.base = 0x0;
    entry.limit = 0xFFFFF;
    entry.access_byte = 0x93;
    entry.flags = 0xC;
    encodeGdtEntry(gdt_pointer + 0x10, entry);



    setGdt();
    return;
}



// Start setting up everything
void boot_main(void) {
    createGdt();
    return;
}