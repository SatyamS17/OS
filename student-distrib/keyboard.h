#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#define KEYBOARD_HANDLER_VEC 0x21
#define BUFFER_SIZE 128 /* terminal input buffer size (chars) */

/* Struct containing information for the keyboard buffer. */
typedef struct keyboard_buffer {
    char buf[BUFFER_SIZE]; /* keyboard buffer                      */
    int idx;               /* next index to write to in buffer     */
    int data_available;    /* flag for if data is available or not */
} keyboard_buffer_t;

extern void keyboard_set_buffer(keyboard_buffer_t *kb);

extern void keyboard_init(void);

extern void keyboard_handler(void);
extern void keyboard_handler_base(void);

#endif /* _KEYBOARD_H_ */
