#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "file_system.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER \
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result) \
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

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
	for (i = 0; i < 20; ++i){
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
	(void) p;
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
	int *a = (int*) 0x400000;
	int p = *a;
	(void) p;
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
	int *a = (int*) 0xB8000;
	int p = *a;
	(void) p;
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
int d_read_t() {
	TEST_HEADER;
	d_read_test();
	return PASS;
}

/* File Short Read
 * 
 * Prints out contents of the file
 * Inputs: None
 * Outputs: PASS, prints out the content in the file
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.h/c
 */
int f_read_st() {
	TEST_HEADER;
	f_read_short_test();
	return PASS;
}

/* File Long Read
 * 
 * Prints out contents of the file
 * Inputs: None
 * Outputs: PASS, prints out the content in the file
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.h/c
 */
int f_read_lt() {
	TEST_HEADER;
	f_read_long_test();
	return PASS;
}

/* File Executable Read
 * 
 * Prints out contents of the file
 * Inputs: None
 * Outputs: PASS, prints out the content in the file
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.h/c
 */
int f_read_et() {
	TEST_HEADER;
	f_read_exec_test();
	return PASS;
}

/* File Not-Readable Read
 * 
 * Prints out contents of the file
 * Inputs: None
 * Outputs: PASS, prints out error
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.h/c
 */
int f_read_nrt() {
	TEST_HEADER;
	f_read_noread_test();
	return PASS;
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
	test_read_dentry_index();
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
	test_read_dentry_name();
	return PASS;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests()
{
	//TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("div_by_zero", div_by_zero());
	// TEST_OUTPUT("invalid_opcode", invalid_opcode());
	// TEST_OUTPUT("null_pointer_access", null_pointer_access());
	//TEST_OUTPUT("kernel_space_memory_access", kernel_space_memory_access());
	//TEST_OUTPUT("video_memory_access", video_memory_access());
	// TEST_OUTPUT("Directory Read", d_read_t());
	// TEST_OUTPUT("File Short Read", f_read_st());
	// TEST_OUTPUT("File Long Read", f_read_lt());
	// TEST_OUTPUT("File Executable Read", f_read_et());
	// TEST_OUTPUT("File Not-Readable Read", f_read_nrt());
	// TEST_OUTPUT("Read Dentry by Index", read_dentry_index());
	// TEST_OUTPUT("Read Dentry by Name", read_dentry_name());

}
