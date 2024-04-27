#include "rtc.h"

#include "i8259.h"
#include "lib.h"
#include "scheduling.h"
#include "syscall.h"
#include "terminal.h"

#define RTC_IRQ 8
#define RTC_IO_PORT 0x70
#define CMOS_IO_PORT 0x71
#define RTC_REG_A 0x8A
#define RTC_REG_B 0x8B
#define RTC_REG_C 0x8C 

#define MIN_FREQ 2
#define MAX_FREQ 1024   
#define INIT_FREQ (MAX_FREQ / MIN_FREQ)

#define MIN_RATE 3 
#define MAX_RATE 15 

// volatile uint32_t rtc_interrupt_counter = 0;           //counter for number of interrupts in a time interval
volatile uint32_t rtc_ticks_per_interrupt = INIT_FREQ; //used to track the ticks for the emulated frequency

/* void rtc_init(void)
 * Inputs: void
 * Return Value: N/A
 * Function: initializes IRQ number for RTC (IRQ8)
 *  -initializes to highest possible frequency
 */
void rtc_init(void) {
    unsigned char prev;

    outb(RTC_REG_B, RTC_IO_PORT);
    prev = inb(CMOS_IO_PORT);
    outb(RTC_REG_B, RTC_IO_PORT);
    outb(prev | 0x40, CMOS_IO_PORT);

    rtc_set_frequency(MAX_FREQ); //set to highest possible frequency

    enable_irq(RTC_IRQ);    
}

/* void rtc_handler(void)
 * Inputs: void
 * Return Value: N/A
 * Function: executes interrupts from RTC
 */
void rtc_handler_base(void) { 
    cli();
    unsigned char temp; 

    outb(RTC_REG_C, RTC_IO_PORT);
    temp = inb(CMOS_IO_PORT); 

    // loop through each process using rtc to increment counter
    int i;
    for(i = 0; i < NUM_TERMINALS; i++) {
        terminal_get_state(i)->rtc_interrupt_counter++;

        if (terminal_get_state(i)->rtc_interrupt_counter >= rtc_ticks_per_interrupt){ 
            terminal_get_state(i)->rtc_interrupt_flag = 1; 
            terminal_get_state(i)->rtc_interrupt_counter = 0;
        }
    }

    send_eoi(RTC_IRQ); 
    sti();
} 

/* 
* int32_t rtc_open()
* Inputs: None
* Output: Returns 0
* Function: Initializes RTC frequency to 2 Hz
*/ 

int32_t rtc_open(const uint8_t* filename){  
    rtc_ticks_per_interrupt = INIT_FREQ;
    return 0;
} 

/* 
* int32_t rtc_close()
* Inputs: None
* Output: Returns 0
* Function: Closes the RTC
*/ 

int32_t rtc_close(int32_t fd){
    return 0;
} 

/* 
* int32_t rtc_read()
* Inputs: None
* Output: Returns 0
* Function: Waits for an interrupt to be raised
*/ 

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    uint8_t idx = scheduler_terminal_idx;
    terminal_get_state(idx)->rtc_interrupt_flag = 0;
    while (!terminal_get_state(idx)->rtc_interrupt_flag) { }

    return 0;
} 

int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){ 

    if (buf == NULL){ //check for valid input buffer
        return -1;
    } 
    if (nbytes != 4){ //check that interrupt rate is correct size
        return -1;
    } 

    uint32_t freq = *(uint32_t*) buf;

    //check that freq is in bounds and is a power of 2
    if ((rtc_get_log2(freq) == -1) || (freq < MIN_FREQ || freq > MAX_FREQ)){ 
        return -1;
    }  

    rtc_ticks_per_interrupt = MAX_FREQ / freq; //change the virtualized rate
    return 0;
}

/* 
* int32_t rtc_set_frequency(int32_t freq)
* Inputs: freq - frequency to be converted to rate
* Output: Success or Fail
* Function: Change the frequency of the RTC
*/

int32_t rtc_set_frequency(int32_t freq){  
    unsigned char prev;  

    if (freq < MIN_FREQ || freq > MAX_FREQ){ //check frequency bounds
        return -1; 
    }

    int32_t rate = (16 - rtc_get_log2(freq)) & 0x0F;  //convert frequency to rate

    if (rate < MIN_RATE || rate > MAX_RATE){ //check rate bounds
        return -1;
    }  

    outb(RTC_REG_A, RTC_IO_PORT); 
    prev = inb(CMOS_IO_PORT); 
    outb(RTC_REG_A, RTC_IO_PORT); 
    outb((prev & 0xF0) | rate, CMOS_IO_PORT);

    return 0;
} 

/* 
* int32_t rtc_get_log2(int32_t freq)
* Inputs: freq - frequency 
* Output: Returns the converted frequency
* Function: Convert frequency to log base 2
*/  

int32_t rtc_get_log2(int32_t freq) {
    int32_t bits = 0;   

    if ((freq & (freq - 1)) != 0){  //check for power of 2
        return -1;
    } 

    while(freq >>= 1) { //right shift (divide by 2) until it becomes 0
        bits++;
    } 
    return bits;
}  

func_pt_t make_rtc_fops(void) {
    func_pt_t f;
    f.open = rtc_open;
    f.close = rtc_close;
    f.read = rtc_read;
    f.write = rtc_write;
    return f;
}
