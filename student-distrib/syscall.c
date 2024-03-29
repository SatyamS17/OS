#include "syscall.h"

#include "lib.h"

int32_t halt(uint8_t status) {
    printf("halt called");
}

int32_t execute(const uint8_t* command) {
    printf("execute called");
}

int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    printf("read called");
}

int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    printf("write called");
}

int32_t open(const uint8_t* filename) {
    printf("open called");
}

int32_t close(int32_t fd) {
    printf("close called");
}

int32_t getargs(uint8_t* buf, int32_t nbytes) {
    printf("getargs called");
}

int32_t vidmap(uint8_t** screen_start) {
    printf("vidmap called");
}

int32_t set_handler(int32_t signum, void* handler_address) {
    printf("set_handler called");
}

int32_t sigreturn(void) {
    printf("sigreturn called");
}
