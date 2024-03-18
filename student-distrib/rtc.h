#ifndef _RTC_H
#define _RTC_H_

#define RTC_HANDLER_VEC 0x28

extern void rtc_init(void);

extern void rtc_handler(void);
extern void rtc_handler_base(void);

#endif  /* _RTC_H_ */
