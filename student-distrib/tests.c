#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

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

// int dir_open_test() {
// 	int i;
// 	int count = file_system->num_dir_entries;
// 	uint8_t temp[FILENAME_SIZE];
// 	for(i = 0; i < count; i++) {
// 		d_read(i, temp, FILENAME_SIZE);
// 		printf(temp);

// 		memset(temp, 0, FILENAME_SIZE);
// 	}

// 	return PASS;
// }

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
}
