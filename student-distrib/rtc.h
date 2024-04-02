#ifndef _RTC_H
#define _RTC_H_

#include "types.h" 

#define RTC_HANDLER_VEC 0x28

extern void rtc_init(void); 
extern void rtc_handler(void);
extern void rtc_handler_base(void); 

extern int32_t rtc_set_frequency(int32_t freq); 
extern int32_t rtc_get_log2(int32_t freq); 

extern int32_t rtc_open(const uint8_t* filename);  
extern int32_t rtc_close(int32_t fd);  
extern int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes); 
extern int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes); 

extern func_pt_t make_rtc_fops(void);

#endif  /* _RTC_H_ */
