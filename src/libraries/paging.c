#include <stdint.h>
#include "paging.h"

extern uint32_t kernel_end;
mem_region_t free_regions[MAX_MEM_REGIONS];
uint32_t free_region_count = 0;

uint32_t alloc_frame() {
	static uint32_t a = 0;
	if (a == 0) {
		a = (((uint32_t) &kernel_end) + 0xFFF) & ~0xFFF; // Rounded to 4KiB boundary
	}
	for (int i = 0; i < free_region_count; i++) {
		if (a >= free_regions[i].base && a + 0x1000 <= free_regions[i].base + free_regions[i].length) {
			// We can allocate it
			uint32_t frame = a;
			a += 0x1000;
			return frame;
		}
	}
	return 0; // We can't find one D:
}

extern uint32_t page_directory[1024];
void map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
	uint32_t pd_index = virt >> 22;
	uint32_t pt_index = (virt >> 12) & 0x3FF;

	if (!(page_directory[pd_index] & 0x1)) {
		uint32_t pt_addr = alloc_frame();

		uint32_t* pt = (uint32_t*) pt_addr;
        for (int i = 0; i < 1024; i++) pt[i] = 0x2;
        page_directory[pd_index] = pt_addr | 0x3;
	}
	uint32_t* pt = (uint32_t*)(page_directory[pd_index] & ~0xFFF);
    pt[pt_index] = phys | flags;
}