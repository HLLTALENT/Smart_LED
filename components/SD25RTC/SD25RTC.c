#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "E2prom.h"
#include "RtcUsr.h"
#include "SD25RTC.h"

uint8_t WriteOn(void);                                           //写允许
uint8_t WriteOff(void);                                          //写禁止
uint8_t RtcReadMulByte(uint8_t addr, uint8_t len, uint8_t *dat); //rtc读取多个字节
uint8_t RtcWriteOneByte(uint8_t addr, uint8_t dat);
uint8_t RtcWriteMulByte(uint8_t addr, uint8_t len, uint8_t *dat);
uint8_t RtcReadDate(uint8_t *time_dat);
void RtcDisplay(void);

S_Time DSRTC = {0x00};

uint8_t RtcWriteOneByte(uint8_t addr, uint8_t dat) //rtc写一个字节
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, RTC_Address, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, addr, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, dat, ACK_CHECK_EN);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

uint8_t RtcReadMulByte(uint8_t addr, uint8_t len, uint8_t *dat) //rtc读取多个字节
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, RTC_Address, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, addr, ACK_CHECK_EN);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, RTC_Address + 1, ACK_CHECK_EN);

    for (int i = 0; i < len; i++)
    {
        if (i != len - 1)
        {
            i2c_master_read_byte(cmd, dat, ACK_VAL);
            dat++;
        }
        else
        {
            i2c_master_read_byte(cmd, dat, NACK_VAL);
        }
    }

    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

uint8_t RtcWriteMulByte(uint8_t addr, uint8_t len, uint8_t *dat) //RTC写入多个字节
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, RTC_Address, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, addr, ACK_CHECK_EN);

    for (int i = 0; i < len; i++)
    {
        i2c_master_write_byte(cmd, *dat, ACK_CHECK_EN);
        dat++;
    }

    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

uint8_t WriteOn(void)
{

    if (!RtcWriteOneByte(0x10, 0x80))
    {
        return false;
    }

    RtcWriteOneByte(0x0f, 0x84);
    return true;
}

uint8_t WriteOff(void)
{
    if (!RtcWriteOneByte(0x0f, 0))
        return false;

    RtcWriteOneByte(0x10, 0);
    return true;
}

uint8_t RtcReadDate(uint8_t *time_dat)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, RTC_Address + 1, ACK_CHECK_EN);

    for (int i = 0; i < 7; i++)
    {
        if (i != 6)
        {
            i2c_master_read_byte(cmd, time_dat, ACK_VAL);
            time_dat++;
        }
        else
        {
            i2c_master_read_byte(cmd, time_dat, NACK_VAL);
        }
    }

    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    /*psRTC->second=time_dat[0];
	psRTC->minute=time_dat[1];
	psRTC->hour=time_dat[3];
	psRTC->hour=psRTC->hour& 0x3f;
	psRTC->week=time_dat[4];
	psRTC->day=time_dat[5];
	psRTC->month=time_dat[6];
	psRTC->year=time_dat[7];*/

    if (ret == ESP_OK)
    {
        printf("rtc read ok\n");
        return true;
    }
    else
    {
        printf("rtc read err\n");
        return false;
    }
}

uint8_t RtcWriteDate(S_Time *psRTC) //设置时间应全部写入，不能单独写一个字节
{
    WriteOn();

    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, RTC_Address, ACK_CHECK_EN);

    i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);

    i2c_master_write_byte(cmd, psRTC->second, ACK_CHECK_EN);

    i2c_master_write_byte(cmd, psRTC->minute, ACK_CHECK_EN);

    i2c_master_write_byte(cmd, psRTC->hour | 0x80, ACK_CHECK_EN); //hour ,同时设置小时寄存器最高位，0=12小时制，1=24小时制

    i2c_master_write_byte(cmd, psRTC->week, ACK_CHECK_EN);

    i2c_master_write_byte(cmd, psRTC->day, ACK_CHECK_EN);

    i2c_master_write_byte(cmd, psRTC->month, ACK_CHECK_EN);

    i2c_master_write_byte(cmd, psRTC->year, ACK_CHECK_EN);

    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    WriteOff();

    if (ret == ESP_OK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void RtcDisplay(void)
{
    printf("%c", (DSRTC.hour >> 4) + '0');
    printf("%c", (DSRTC.hour & 0x0f) + '0');
    printf("时");
    printf("%c", (DSRTC.minute >> 4) + '0');
    printf("%c", (DSRTC.minute & 0x0f) + '0');
    printf("分");
    printf("%c", (DSRTC.second >> 4) + '0');
    printf("%c", (DSRTC.second & 0x0f) + '0');
    printf("秒 ");

    printf("%c", (DSRTC.year >> 4) + '0');
    printf("%c", (DSRTC.year & 0x0f) + '0');
    printf("年");
    printf("%c", (DSRTC.month >> 4) + '0');
    printf("%c", (DSRTC.month & 0x0f) + '0');
    printf("月");
    printf("%c", (DSRTC.day >> 4) + '0');
    printf("%c", (DSRTC.day & 0x0f) + '0');
    printf("日\n");
}

void sd25rtc_init(void)
{

    uint8_t read_dat[10];
    //uint8_t	Sram[12]={0};	//通用数据缓存器
    WriteOn();
    //RtcReadMulByte(IDcode,8,Sram);//读取内部8位ID
    //printf("Sram=%x %x %x %x %x %x %x %x\r\n",Sram[0],Sram[1],Sram[2],Sram[3],Sram[4],Sram[5],Sram[6],Sram[7]);
    //RtcWriteMulByte(0x30,8,Sram);//将8位ID写入0x30开始的用户存储区
    RtcWriteOneByte(0x18, 0x82); //打开充电
    WriteOff();
    //printf("rtc write=%d\n",RtcWriteDate(&DSRTC));//设置时间
    RtcReadDate(read_dat); //读取时间

    DSRTC.second = read_dat[0];
    DSRTC.minute = read_dat[1];
    DSRTC.hour = read_dat[2];
    DSRTC.hour = DSRTC.hour & 0x3f;
    DSRTC.week = read_dat[3];
    DSRTC.day = read_dat[4];
    DSRTC.month = read_dat[5];
    DSRTC.year = read_dat[6];

    /*printf("time_dat0=%x\n",read_dat[0]);
	printf("time_dat1=%x\n",read_dat[1]);
	printf("time_dat2=%x\n",read_dat[2]);
	printf("time_dat3=%x\n",read_dat[3]);
	printf("time_dat4=%x\n",read_dat[4]);
	printf("time_dat5=%x\n",read_dat[5]);
	printf("time_dat6=%x\n",read_dat[6]);*/

    RtcDisplay(); //打印时间

    //开机将SD25时间设置为系统时间
    int year, month, day, hour, min, sec;
    uint8_t temp;

    temp = DSRTC.year >> 4;
    temp = temp & 0x0f;
    DSRTC.year = DSRTC.year & 0x0f;
    year = 2000 + temp * 10 + DSRTC.year;
    printf("rtc year=%d\n", year);

    temp = DSRTC.month >> 4;
    temp = temp & 0x0f;
    DSRTC.month = DSRTC.month & 0x0f;
    month = temp * 10 + DSRTC.month;
    printf("rtc month=%d\n", month);

    temp = DSRTC.day >> 4;
    temp = temp & 0x0f;
    DSRTC.day = DSRTC.day & 0x0f;
    day = temp * 10 + DSRTC.day;
    printf("rtc day=%d\n", day);

    temp = DSRTC.hour >> 4;
    temp = temp & 0x0f;
    DSRTC.hour = DSRTC.hour & 0x0f;
    hour = temp * 10 + DSRTC.hour;
    printf("rtc hour=%d\n", hour);

    temp = DSRTC.minute >> 4;
    temp = temp & 0x0f;
    DSRTC.minute = DSRTC.minute & 0x0f;
    min = temp * 10 + DSRTC.minute;
    printf("rtc min=%d\n", min);

    temp = DSRTC.second >> 4;
    temp = temp & 0x0f;
    DSRTC.second = DSRTC.second & 0x0f;
    sec = temp * 10 + DSRTC.second;
    printf("rtc sec=%d\n", sec);

    Rtc_Set(year, month, day, hour, min, sec);
}

void SD25Rtc_Read(int *year, int *month, int *day, int *hour, int *min, int *sec)
{
    uint8_t temp;
    uint8_t read_dat[10];
    RtcReadDate(read_dat); //读取时间

    DSRTC.second = read_dat[0];
    DSRTC.minute = read_dat[1];
    DSRTC.hour = read_dat[2];
    DSRTC.hour = DSRTC.hour & 0x3f;
    DSRTC.week = read_dat[3];
    DSRTC.day = read_dat[4];
    DSRTC.month = read_dat[5];
    DSRTC.year = read_dat[6];

    temp = DSRTC.year >> 4;
    temp = temp & 0x0f;
    DSRTC.year = DSRTC.year & 0x0f;
    *year = 2000 + temp * 10 + DSRTC.year;
    printf("rtc year=%d\n", (int)year);

    temp = DSRTC.month >> 4;
    temp = temp & 0x0f;
    DSRTC.month = DSRTC.month & 0x0f;
    *month = temp * 10 + DSRTC.month;
    printf("rtc month=%d\n", (int)month);

    temp = DSRTC.day >> 4;
    temp = temp & 0x0f;
    DSRTC.day = DSRTC.day & 0x0f;
    *day = temp * 10 + DSRTC.day;
    printf("rtc day=%d\n", (int)day);

    temp = DSRTC.hour >> 4;
    temp = temp & 0x0f;
    DSRTC.hour = DSRTC.hour & 0x0f;
    *hour = temp * 10 + DSRTC.hour;
    printf("rtc hour=%d\n", (int)hour);

    temp = DSRTC.minute >> 4;
    temp = temp & 0x0f;
    DSRTC.minute = DSRTC.minute & 0x0f;
    *min = temp * 10 + DSRTC.minute;
    printf("rtc min=%d\n", (int)min);

    temp = DSRTC.second >> 4;
    temp = temp & 0x0f;
    DSRTC.second = DSRTC.second & 0x0f;
    *sec = temp * 10 + DSRTC.second;
    printf("rtc sec=%d\n", (int)sec);

    // *year = (1900 + p->tm_year);
    // *month = (1 + p->tm_mon);
    // *day = p->tm_mday;
    // *hour = p->tm_hour;
    // *min = p->tm_min;
    // *sec = p->tm_sec;
    //ESP_LOGI(TAG, "Read:%d-%d-%d %d:%d:%d No.%d",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec,(1+p->tm_yday));
}
