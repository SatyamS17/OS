#ifndef _PIT_H
#define _PIT_H

#define PIT_HANDLER_VEC 0x20

extern void pit_init(void);

extern void pit_handler(void);

extern void pit_handler_base(void);

#endif
