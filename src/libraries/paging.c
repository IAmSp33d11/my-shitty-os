#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "paging.h"
#include "string.h"
#include "vga.h"


extern uint32_t kernel_end;
mem_region_t free_regions[MAX_MEM_REGIONS];
uint32_t free_region_count = 0;
uint64_t max_addr = 0;
uint32_t total_pages = 0;
extern uint32_t *page_directory;

extern uint32_t kernel_start;


// Buddy Memory Allocation for paging
// Be Like Linux cause we are too stupid so lets just be a fricking copycat

struct page {
	uint8_t order; // Current order of the block
	uint32_t flags;
	struct page *next;
	struct page *prev; 
};

struct page *mem_map;

void init_mem_manage() {}



// Total addressable bytes in 32-bit (2^32)
#define MAX_ADDRESS_SPACE 0x100000000ULL 

// Page size (4 KiB)
#define PAGE_SIZE         4096           

// Total number of pages (1,048,576)
#define TOTAL_PAGES       (MAX_ADDRESS_SPACE / PAGE_SIZE) 

// Total bytes for bitmap (131,072)
#define BITMAP_SIZE       (TOTAL_PAGES / 8)

static uint8_t bitmap[BITMAP_SIZE];

// 4KiB pages, each page is a bit. To get the bit for a page is O(n), to allocate is shit tho
void setup_bitmap() {
	for (uint32_t i = 0; i < BITMAP_SIZE; i++) {
		bitmap[i] = 0xFF; // Set ever 4KiB page to used.
	}

	for (uint32_t i = 0; i < free_region_count; i++) {
		if (free_regions[i].base < MAX_ADDRESS_SPACE) {
			uint64_t length = free_regions[i].length;

			if (free_regions[i].base + length > MAX_ADDRESS_SPACE) {
				length = 0x100000000ULL - free_regions[i].base;
			}

			set_region_status(free_regions[i].base, length, false);
		}
	}
	
	// Kernel must not be allocated
	uintptr_t k_phys_start = virt_to_phys(&kernel_start);
	uintptr_t k_phys_end = virt_to_phys(&kernel_end);
	set_region_status(k_phys_start, k_phys_end - k_phys_start, true);

	// Neither shall the bottom 16KiB. Its too dangerous I think.
	set_region_status(0, 0x4000, true);
}

// True for reserved, false for free.
void set_region_status(uint32_t base, uint32_t length, bool status) {
	uint64_t start_page, end_page;
	
	if (status) {// reserving
		start_page = base / 4096; // Round down
		end_page = (base + length + 4095) / 4096; // Round up
	} else { // freeing
		start_page = (base + 4095) / 4096; // Round up
		end_page = (base + length) / 4096; // Round down
	}

	for (uint32_t i = start_page; i < end_page; i++) {
		set_page_status(i, status);
	}
}

// True for reserved, false for free.
void set_page_status(uint32_t page, bool status) {
	uint32_t byte = page / 8;
	uint8_t bit = page % 8;

	if (status) {
		bitmap[byte] |= (1 << bit);
	} else {
		bitmap[byte] &= ~(1 << bit);
	}
}

bool check_page_status(uint32_t page) {
	uint32_t byte = page / 8;
	uint8_t bit = page % 8;

	return ((bitmap[byte] >> bit) & 0x1);
}


uintptr_t alloc_frame() {
	uint32_t start_byte = (virt_to_phys(&kernel_end) / 4096) / 8;
	for (size_t i = start_byte; i < BITMAP_SIZE; i++) {
		if (bitmap[i] == 0xFF) continue;
		else {
			for (size_t j = 0; j < 8; j++) {
				uint32_t page_idx = j + (i * 8);
				if (!check_page_status(page_idx)) {
					set_page_status(page_idx, true);
					return page_idx * 4096;
				}
			}
		}
	}
	return 0;
}




void map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
	uint32_t pd_index = virt >> 22;
	uint32_t pt_index = (virt >> 12) & 0x3FF;

	if (!(page_directory[pd_index] & 0x1)) {
		uint32_t pt_addr = alloc_frame();

		uint32_t* pt = (uint32_t*)(pt_addr + KERNEL_OFFSET);
        for (int i = 0; i < 1024; i++) pt[i] = 0x0;
        uint32_t pd_flags = (flags & 0x4) ? 0x7 : 0x3;
        page_directory[pd_index] = pt_addr | pd_flags;
	}
	uint32_t* pt = (uint32_t*)((page_directory[pd_index] & ~0xFFF) + KERNEL_OFFSET);
    pt[pt_index] = phys | flags;
	__asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}




void* kmalloc(uint32_t size) {
}