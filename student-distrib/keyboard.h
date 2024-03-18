#ifndef _KEYBOARD_H
#define _KEYBOARD_H_

#define KEYBOARD_HANDLER_VEC 0x21

extern void keyboard_init(void);

extern void keyboard_handler(void);
extern void keyboard_handler_base(void);

#endif  /* _KEYBOARD_H_ */
