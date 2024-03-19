#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
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
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
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

	return PASS;
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
	
	return PASS;
}

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("div_by_zero", div_by_zero());
	// TEST_OUTPUT("invalid_opcode", invalid_opcode());
}
