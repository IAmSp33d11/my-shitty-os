#ifndef PAGING_H
#define PAGING_H

typedef struct {
	uint64_t base;
	uint64_t length;
} mem_region_t;

#define MAX_MEM_REGIONS 32
uint32_t alloc_frame();
extern uint32_t free_region_count;
extern mem_region_t free_regions[MAX_MEM_REGIONS];
void map_page(uint32_t virt, uint32_t phys, uint32_t flags);

#endif