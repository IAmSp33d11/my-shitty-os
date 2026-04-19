#ifndef PAGING_H
#define PAGING_H

typedef struct {
	uint64_t base;
	uint64_t length;
} mem_region_t;

#define KERNEL_OFFSET 0xC0000000
#define MAX_MEM_REGIONS 128
void setup_bitmap();
void set_region_status(uint32_t base, uint32_t length, bool status);
void set_page_status(uint32_t page, bool status);
bool check_page_status(uint32_t page);

uintptr_t alloc_frame();
extern uint32_t free_region_count;
extern mem_region_t free_regions[MAX_MEM_REGIONS];
void map_page(uint32_t virt, uint32_t phys, uint32_t flags);
void* kmalloc(uint32_t size);
void* umalloc(uint32_t size);
#endif