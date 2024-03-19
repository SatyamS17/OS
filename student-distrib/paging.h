
#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"
#include "x86_desc.h"

#ifndef ASM
/* Number of pages that can be stored in a 4KB directory*/
#define PAGE_NUM            1024

/* Number of bits in a 4KB */
#define FOURKB_BITS         (PAGE_NUM * 4)

/* Starting address given to us in documentation*/
#define KERNEL_ADDRESS      0x400000

/* Video memory address in table */
#define VID_MEM_INDEX       (0xB8000 / FOURKB_BITS)

/* Inits page_dir and page_table and CR0, CR3, CR4 as needed to start paging*/
extern void paging_init();

#endif /* ASM */
#endif /* _PAGING_H */
