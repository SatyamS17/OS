#include "syscall.h"

#include "lib.h"

void syscall_handler_base(void) {
    printf("system call handler called\n");
}
