#include "tests.h"

#include "file_system.h"
#include "keyboard.h"
#include "lib.h"
#include "rtc.h"
#include "terminal.h"
#include "x86_desc.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER                                                                                \
    printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)

#define TEST_OUTPUT(name, result)                                                                  \
    do {                                                                                           \
        int r = result;                                                                            \
        printf("[TEST %s] Result = %s\n", name, r ? "PASS" : "FAIL");                              \
        if (r) {                                                                                   \
            passed++;                                                                              \
        } else {                                                                                   \
            failed++;                                                                              \
        }                                                                                          \
    } while (0)

static inline void assertion_failure() {
    /* Use exception #15 for assertions, otherwise
       reserved by Intel */
    asm volatile("int $15");
}

/* Checkpoint 1 tests */

/* IDT Test
 *
 * Asserts that first 20 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test() {
    TEST_HEADER;

    int i;
    int result = PASS;
    for (i = 0; i < 20; ++i) {
        if (i == 15) {
            continue;
        }

        if ((idt[i].offset_15_00 == NULL && idt[i].offset_31_16 == NULL) || idt[i].present == 0) {
            assertion_failure();
            result = FAIL;
        }
    }

    return result;
}

/* Div by Zero Test
 *
 * Performs a division by zero to test exception handler
 * Inputs: None
 * Outputs: None
 * Side Effects: Should print exception
 * Coverage: Load IDT, IDT definition
 * Files: idt.h/c, exceptions.h/c
 */
int div_by_zero() {
    TEST_HEADER;
    int a = 0;
    int b;
    b = 1 / a;

    return FAIL;
}

/* Invalid Opcode Test
 *
 * Performs an invlid opcode to test exception handler
 * Inputs: None
 * Outputs: None
 * Side Effects: Should print exception
 * Coverage: Load IDT, IDT definition
 * Files: idt.h/c, exceptions.h/c
 */
int invalid_opcode() {
    TEST_HEADER;
    asm("ud2");

    return FAIL;
}

/* Null Pointer Access Test
 *
 * Performs a null pointer access to test page fault exception handler
 * Inputs: None
 * Outputs: None
 * Side Effects: Should print exception
 * Coverage: Load IDT, IDT definition, Paging
 * Files: idt.h/c, exceptions.h/c, paging.h/c
 */
int null_pointer_access() {
    TEST_HEADER;
    int *a = 0;
    int p = *a;
    (void)p;
    return FAIL;
}

/* Kernel Space Memory Access
 *
 * Performs a valid memory access in kernel space
 * Inputs: None
 * Outputs: PASS, blue screen if fail
 * Side Effects: None
 * Coverage: Paging
 * Files: paging.h/c
 */
int kernel_space_memory_access() {
    TEST_HEADER;
    int *a = (int *)0x400000;
    int p = *a;
    (void)p;
    return PASS;
}

/* Video Memory Access
 *
 * Performs a valid memory access in video memory
 * Inputs: None
 * Outputs: PASS, blue screen if fail
 * Side Effects: None
 * Coverage: Paging
 * Files: paging.h/c
 */
int video_memory_access() {
    TEST_HEADER;
    int *a = (int *)0xB8000;
    int p = *a;
    (void)p;
    return PASS;
}

/* Checkpoint 2 tests */

/* Directory Read
 *
 * Prints out all the files in the directory
 * Inputs: None
 * Outputs: PASS, prints out all file in directory
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.h/c
 */
int directory_read() {
    TEST_HEADER;

    int i, j;

    // Prints out each of the files in the directory
    for (i = 0; i < file_system->num_dir_entries; i++) {
        uint8_t temp[FILENAME_SIZE];
        // calls function being tested
        dir_read(0, temp, FILENAME_SIZE);
        printf("File name: ");

        for (j = 0; j < FILENAME_SIZE; j++) {
            printf("%c", temp[j]);
        }

        printf("\n");
    }

    return PASS;
}

/* File Read
 *
 * Prints out contents of the file
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
int file_read_test(uint8_t *test_name) {
    int i;
    dentry_t test_dentry;

    // get file information based on inode so that we can read correct file
    read_dentry_by_name(test_name, &test_dentry);

    // get the size of the file
    uint32_t size =
        ((inodes_t *)((uint8_t *)file_system + ((test_dentry.inode + 1) * (BLOCK_SIZE))))->length;

    // create a large buffer that can hold large files (*20 makes sure the buffer is large for
    // testing)
    uint8_t buf[BLOCK_SIZE * 20] = {0};

    // if its a readable file then read it
    if (test_dentry.file_type == 2) {
        // fill buffer with data
        uint32_t data = file_read(test_dentry.inode, buf, size);

        // print buffer
        for (i = 0; i < data; i++) {
            if (buf[i] != '\0') {
                putc(buf[i]);
            }
        }

        printf("\nFile read: ");
        for (i = 0; i < FILENAME_SIZE; i++) {
            putc(test_dentry.file_name[i]);
        }
        putc('\n');
        return PASS;
    }
    // else print out error that it's not readable
    else {
        printf("\nNot readable file : ");
        for (i = 0; i < FILENAME_SIZE; i++) {
            putc(test_dentry.file_name[i]);
        }
        putc('\n');
        return FAIL;
    }
}

/* File Read Long
 *
 * Prints out contents of the file
 * Inputs: None
 * Outputs: PASS, prints out the content in the file
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.h/c
 */
int file_read_short() {
    TEST_HEADER;
    uint8_t test_name[FILENAME_SIZE] = "frame0.txt";
    return file_read_test(test_name);
}

/* File Read Long
 *
 * Prints out contents of the file
 * Inputs: None
 * Outputs: PASS, prints out the content in the file
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.h/c
 */
int file_read_long() {
    TEST_HEADER;
    uint8_t test_name[FILENAME_SIZE] = "verylargetextwithverylongname.tx";
    return file_read_test(test_name);
}

/* File Read Executable
 *
 * Prints out contents of the file
 * Inputs: None
 * Outputs: PASS, prints out the content in the file
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.h/c
 */
int file_read_executable() {
    TEST_HEADER;
    uint8_t test_name[FILENAME_SIZE] = "ls";
    return file_read_test(test_name);
}

/* File Read Not-Readable
 *
 * Prints out contents of the file
 * Inputs: None
 * Outputs: PASS, prints out error
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.h/c
 */
int file_read_not_readable() {
    TEST_HEADER;
    uint8_t test_name[FILENAME_SIZE] = ".";
    return file_read_test(test_name) == FAIL ? PASS : FAIL;
}

/* Read Dentry by Index
 *
 * Prints out inode based on index if its valid else error
 * Inputs: None
 * Outputs: PASS, prints out inode details or error
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.h/c
 */
int read_dentry_index() {
    TEST_HEADER;
    dentry_t test_dentry;
    int i;

    // check both valid and invalid indexs for dentry
    for (i = 0; i < file_system->num_dir_entries + 3; i++) {
        // test read_dentry_by_index
        if (read_dentry_by_index(i, &test_dentry) == 0) {
            printf("File %d name: %s   ", i, test_dentry.file_name);
            printf("type: %d   ", test_dentry.file_type);
            printf("inode: %d \n", test_dentry.inode);
        } else {
            printf("Index %d not valid \n", i);
        }
    }

    return PASS;
}

/* Read Dentry by Name
 *
 * Prints out inode based on file name if its valid else error
 * Inputs: None
 * Outputs: PASS, prints out inode details or error
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.h/c
 */
int read_dentry_name() {
    TEST_HEADER;
    dentry_t test_dentry;
    int i;

    // can change but good selection of test cases
    uint8_t test_names[5][FILENAME_SIZE] = {"ls", "cat", "notreal", "", "frame0.txt"};

    // check both valid and invalid file names for dentry
    for (i = 0; i < 5; i++) {
        // test read_dentry
        if (read_dentry_by_name(test_names[i], &test_dentry) == 0) {
            printf("File %d name: %s  ", i, test_dentry.file_name);
            printf("type: %d   ", test_dentry.file_type);
            printf("inode: %d \n", test_dentry.inode);
        } else {
            printf("Invalid file!!! Name: %s  \n", test_names[i]);
        }
    }
    return PASS;
}

/* Terminal Driver
 *
 * Performs terminal reads/writes to test printing user inputs
 * Inputs: int buf_size - buffer size to test (1-256)
 * Outputs: None, FAIL if terminal doesn't write same number of bytes as read
 * Side Effects: None
 * Coverage: Terminal driver, keyboard
 * Files: keyboard.h/c, terminal.h/c
 */
int terminal_driver(int buf_size) {
    TEST_HEADER;

    char buf[256];
    int bytes_r, bytes_w;

    terminal_open(NULL);
    while (1) {
        bytes_r = terminal_read(0, (void *)buf, buf_size);
        bytes_w = terminal_write(0, (const void *)buf, bytes_r);
        if (bytes_r != bytes_w) {
            printf("Bytes read (%d) != bytes written (%d)\n", bytes_r, bytes_w);
            break;
        }
        memset(buf, 0, buf_size);
    }
    terminal_close(NULL);
    return FAIL;
}

/* RTC Driver Test
 *
 * Changes RTC frequency, printing 1 at each interrupt
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: rtc driver
 * Files: rtc.h/c
 */
int rtc_driver_test() {
    TEST_HEADER;
    int i, j;
    for (i = 2; i <= 1024; i *= 2) {
        rtc_write(NULL, &i, 4);
        for (j = 0; j < i; j++) {
            rtc_read(NULL, NULL, NULL);
            printf("1");
        }
        printf("\n");
    }
    return PASS;
}

/* Checkpoint 3 tests */

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests() {
    int passed = 0, failed = 0;

    /*Checkpoint 1*/

    TEST_OUTPUT("idt_test", idt_test());

    // Enabling these will intentionally cause blue screen
    // TEST_OUTPUT("div_by_zero", div_by_zero());
    // TEST_OUTPUT("invalid_opcode", invalid_opcode());
    // TEST_OUTPUT("null_pointer_access", null_pointer_access());

    // TEST_OUTPUT("kernel_space_memory_access", kernel_space_memory_access());
    // TEST_OUTPUT("video_memory_access", video_memory_access());

    // /*Checkpoint 2*/

    // TEST_OUTPUT("directory_read", directory_read());
    // TEST_OUTPUT("file_read_short", file_read_short());
    // TEST_OUTPUT("file_read_long", file_read_long());
    // TEST_OUTPUT("file_read_executable", file_read_executable());
    // TEST_OUTPUT("file_read_not_readable", file_read_not_readable());
    // TEST_OUTPUT("read_dentry_index", read_dentry_index());
    // TEST_OUTPUT("read_dentry_name", read_dentry_name());

    // Enable to run RTC driver test (takes a few seconds)
    // TEST_OUTPUT("rtc_driver_test", rtc_driver_test());

    // Enable to test terminal read/write (test never exits, run it last)
    // TEST_OUTPUT("terminal_driver", terminal_driver(4));
    printf("Test summary: %d passed, %d failed\n", passed, failed);
}
