# Memory Management Header (mm.h)

## Overview

This header file defines the core memory management structures and functions for a Linux-based operating system kernel. It establishes the foundation for page-based memory management, swap operations, and memory allocation capabilities that are fundamental to the kernel's operation.

## Memory Management Architecture

The file implements a paging-based memory management system that divides physical memory into fixed-size pages (4KB each). This approach allows the OS to efficiently manage memory allocation, virtual memory mapping, and swap operations when physical memory is exhausted.

## Core Definitions and Constants


#define PAGE_SIZE 4096


Defines the standard page size as 4KB (4096 bytes), which is the fundamental unit of memory allocation in this system. This size is chosen as it balances efficiency with granularity, and matches the x86 architecture's standard page size.


#define LOW_MEM 0x100000
extern unsigned long HIGH_MEMORY;
#define PAGING_MEMORY (15*1024*1024)
#define PAGING_PAGES (PAGING_MEMORY>>12)


These define the memory layout:
- `LOW_MEM`: Marks the first 1MB (0x100000) as reserved for kernel use
- `HIGH_MEMORY`: Upper bound of available memory (defined elsewhere)
- `PAGING_MEMORY`: Sets a 15MB limit for pageable memory
- `PAGING_PAGES`: Calculates the number of pages in paging memory (dividing by 4096 via right shift by 12)

## Memory Mapping


#define MAP_NR(addr) (((addr)-LOW_MEM)>>12)


Converts a physical memory address to its corresponding index in the memory map by:
1. Subtracting `LOW_MEM` to skip reserved memory
2. Right-shifting by 12 to divide by 4096 (PAGE_SIZE), effectively identifying which page contains the address


extern unsigned char mem_map[PAGING_PAGES];


This array tracks the usage status of each page in the system. Each byte represents one page:
- A value of 0 indicates a free page
- Values > 0 indicate the number of references to the page
- `USED` (100) marks pages that are permanently allocated

## Page Table Entry Flags


#define PAGE_DIRTY     0x40
#define PAGE_ACCESSED  0x20
#define PAGE_USER      0x04
#define PAGE_RW        0x02
#define PAGE_PRESENT   0x01


These bit flags define attributes for page table entries:
- `PAGE_DIRTY`: Page has been written to and needs to be saved to disk on eviction
- `PAGE_ACCESSED`: Page has been recently accessed (used for page replacement algorithms)
- `PAGE_USER`: Page is accessible to user-mode code (not just kernel)
- `PAGE_RW`: Page is writable (if 0, page is read-only)
- `PAGE_PRESENT`: Page is currently loaded in physical memory

## Swap Management


extern int SWAP_DEV;

#define read_swap_page(nr,buffer) ll_rw_page(READ,SWAP_DEV,(nr),(buffer));
#define write_swap_page(nr,buffer) ll_rw_page(WRITE,SWAP_DEV,(nr),(buffer));


These macros provide an interface to read and write pages to the swap device:
- `SWAP_DEV`: Identifies which block device is used for swap space
- The macros wrap the lower-level `ll_rw_page` function to perform the actual I/O operations

## Memory Management Functions


extern unsigned long get_free_page(void);


Allocates a free physical memory page and returns its address. If no free pages are available, it may trigger page swapping or memory reclamation.


extern unsigned long put_dirty_page(unsigned long page, unsigned long address);


Maps a physical page to a virtual address and marks it as dirty. This function is typically used when loading data from disk or swap space.


extern void free_page(unsigned long addr);


Releases a previously allocated page, making it available for reuse. It decrements the reference count in the memory map.


void swap_free(int page_nr);


Frees a page in the swap area, marking it as available for future use.


void swap_in(unsigned long *table_ptr);


Loads a page from swap space into memory and updates the page table to point to it.

## Special Operations


static inline void oom(void)
{
    printk("out of memory\n\r");
    do_exit(SIGSEGV);
}


This function handles "out of memory" conditions by:
1. Printing an error message to the kernel log
2. Terminating the current process with a segmentation fault (SIGSEGV) signal
3. Using `inline` to reduce function call overhead in critical memory situations


#define invalidate() \
__asm__("movl %%eax,%%cr3"::"a" (0))


Invalidates the Translation Lookaside Buffer (TLB) by writing to the CR3 register, forcing the CPU to reload page tables. This is necessary after modifying page tables to ensure the CPU uses the updated mappings.

## Implementation Notes

This memory management system follows key design principles:
- **Paging-based architecture**: Memory is managed in fixed-size blocks
- **On-demand paging**: Pages are loaded only when needed and can be swapped out
- **Reference counting**: Pages can be shared between processes with proper accounting
- **Virtual memory**: Physical pages are mapped to virtual addresses for process isolation

The memory management header provides a comprehensive foundation for a Unix-like operating system's memory subsystem, balancing efficiency with flexibility to support multitasking and memory protection.

