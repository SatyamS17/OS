#ifndef _PIT_H
#define _PIT_H 


#include "scheduling.h"

#define PIT_RATE 11932 //1.19318 Mhz 
#define PIT_DATA 0x40 
#define PIT_CMD 0x43 
#define PIT_MODE 0x37 
#define PIT_IRQ 0 

#define PIT_HANDLER_VEC 0x20

extern void pit_init(void); 
extern void pit_handler(void); 

#endif
