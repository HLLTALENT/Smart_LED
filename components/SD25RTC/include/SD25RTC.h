#ifndef _SD25RTC_H
#define _SD25RTC_H
#include "freertos/FreeRTOS.h"

#define RTC_Address 0x64 //RTC器件地址
#define IDcode 0x72      //8字节ID号起始地址

#define I2C_MASTER_SCL2_IO 26        /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA2_IO 25        /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM1 I2C_NUM_1    /*!< I2C port number for master dev */
#define I2C_MASTER_TX2_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_MASTER_RX2_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ 100000    /*!< I2C master clock frequency */

#define ACK_CHECK_EN 0x1  /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0 /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0       /*!< I2C ack value */
#define NACK_VAL 0x1      /*!< I2C nack value */

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
extern int year, month, day, hour, min, sec;

extern void sd25rtc_init(void);
extern uint8_t RtcWriteDate(S_Time *psRTC);
extern void SD25RTC_IIC_Init(void);
extern void SD25Rtc_Read(int *year, int *month, int *day, int *hour, int *min, int *sec);

#endif
