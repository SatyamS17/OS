
#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"
#include "x86_desc.h"

#ifndef ASM

/* Number of pages that can be stored in a 4KB directory*/
#define PAGE_NUM 1024

/* Number of bits in a 4KB */
#define FOURKB_BITS (PAGE_NUM * 4)

/* Use to get the base address */
#define ADDRESS_SHIFT 12

/* Starting address given to us in documentation*/
#define KERNEL_ADDRESS 0x400000

/* Video memory address in table */
#define VID_MEM 0xB8000

/* Video memory address in table */
#define VID_MEM_TER1 (VID_MEM + FOURKB_BITS)

/* Video memory address in table */
#define VID_MEM_TER2 (VID_MEM_TER1 + FOURKB_BITS)

/* Video memory address in table */
#define VID_MEM_TER3 (VID_MEM_TER2 + FOURKB_BITS)

/* Video memory address in table */
#define VID_MEM_INDEX (VID_MEM / FOURKB_BITS)

/* Video memory address in table */
#define VID_MEM1_INDEX (VID_MEM_TER1 / FOURKB_BITS)

/* Video memory address in table */
#define VID_MEM2_INDEX (VID_MEM_TER2 / FOURKB_BITS)

/* Video memory address in table */
#define VID_MEM3_INDEX (VID_MEM_TER3 / FOURKB_BITS)

/* Video memory address in table */
#define VIRTUAL_VID_MEM 0x8C00000

/* Video memory address in table */
#define USER_INDEX 32

/* Video memory address in table */
#define USER_VID_INDEX 35

/* Starting address given to us in documentation*/
#define USER_ADDRESS 0x8000000

/* Inits page_dir and page_table and CR0, CR3, CR4 as needed to start paging*/
extern void paging_init();

#endif /* ASM */
#endif /* _PAGING_H */
