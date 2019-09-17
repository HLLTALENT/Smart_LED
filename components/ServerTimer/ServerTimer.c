#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "RtcUsr.h"
#include "SD25RTC.h"
#include "ServerTimer.h"

static const char *TAG = "SeverTime";

time_t timer_timstamp;
time_t base_timstamp;
time_t now_timstamp;
struct tm *now_time;
char time_down[24];

uint8_t dev_start=0;//用于开机第一次得到网络时间设置SD25rtc时钟

esp_err_t Server_Timer_GET(char *Server_timer_data)
{
    struct tm *tmp_time = (struct tm *)malloc(sizeof(struct tm));
    strptime(Server_timer_data, "%Y-%m-%dT%H:%M:%SZ", tmp_time);
    base_timstamp = mktime(tmp_time);
    printf("this is time tamp%ld\r\n", base_timstamp);

    //time_t t = mktime(tmp_time);
    ESP_LOGI(TAG, "Setting time: %s", asctime(tmp_time));
    struct timeval now = { .tv_sec = (base_timstamp+28800) };
    settimeofday(&now, NULL);

    free(tmp_time);

    if(dev_start==0)
    {
        //开机第一次进入该函数时，设置SD25RTC芯片时钟
        int year, month, day, hour, min, sec;
        int ret;
        Rtc_Read(&year, &month, &day, &hour, &min, &sec);
        ESP_LOGE("net time Read:","%d-%d-%d %d:%d:%d\n", year, month, day, hour, min, sec);

        S_Time DS25_Set={0};
        DS25_Set.year=year%100/10;
        DS25_Set.year=DS25_Set.year<<4;
        year=year%10;
        DS25_Set.year=DS25_Set.year|year;
        printf("year=0x%02x\n",DS25_Set.year);

        DS25_Set.month=month/10;
        DS25_Set.month=DS25_Set.month<<4;
        month=month%10;
        DS25_Set.month=DS25_Set.month|month;
        printf("month=0x%02x\n",DS25_Set.month);

        DS25_Set.day=day/10;
        DS25_Set.day=DS25_Set.day<<4;
        day=day%10;
        DS25_Set.day=DS25_Set.day|day;
        printf("day=0x%02x\n",DS25_Set.day);

        DS25_Set.hour=hour/10;
        DS25_Set.hour=DS25_Set.hour<<4;
        hour=hour%10;
        DS25_Set.hour=DS25_Set.hour|hour;
        printf("hour=0x%02x\n",DS25_Set.hour);

        DS25_Set.minute=min/10;
        DS25_Set.minute=DS25_Set.minute<<4;
        min=min%10;
        DS25_Set.minute=DS25_Set.minute|min;
        printf("minute=0x%02x\n",DS25_Set.minute);

        DS25_Set.second=sec/10;
        DS25_Set.second=DS25_Set.second<<4;
        sec=sec%10;
        DS25_Set.second=DS25_Set.second|sec;
        printf("second=0x%02x\n",DS25_Set.second);   

        ret=RtcWriteDate(&DS25_Set);//设置时间
        /*设置时间格式例子：16进制格式
        S_Time DSRTC={0x00,0x38,0x17,0x02,0x11,0x09,0x18};
                        00秒 38分 17时 周二 11日 9月 18年
        */
        printf("rtc write=%d\n",ret);
        if(ret==1)
        {
            dev_start=1;//设置成功，下次获取网络时间不再设置
        }
    }

    return base_timstamp;
}
char* Server_Timer_SEND(void)
{
    time (&now_timstamp);
    now_timstamp-=28800;
    now_time = gmtime(&now_timstamp);
    strftime(time_down, sizeof(time_down), "%Y-%m-%dT%H:%M:%SZ", now_time);
    printf("this is time %s\r\n", time_down);
    return time_down;
}