#ifndef _SD25RTC_H
#define _SD25RTC_H
#include "freertos/FreeRTOS.h"

#define RTC_Address 0x64 //RTC器件地址
#define IDcode 0x72      //8字节ID号起始地址

typedef struct
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t week;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} S_Time;

extern void sd25rtc_init(void);
extern uint8_t RtcWriteDate(S_Time *psRTC);
void SD25Rtc_Read(int *year, int *month, int *day, int *hour, int *min, int *sec);

#endif
