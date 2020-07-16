#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "Pwm.h"
#include "RtcUsr.h"
#include "driver/ledc.h"
#include "Wallkey.h"
#include "Localcalculation.h"
#include "SD25RTC.h"
#include "Human.h"
#include "Bluetooth.h"
#include "Json_parse.h"
#include <string.h>
#include "Led.h"

static const char *TAG = "Pwm";
extern uint8_t human_status;

#define LEDC_HS_TIMER LEDC_TIMER_0
#define LEDC_HS_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO (19) //上灯条 白灯
#define LEDC_HS_CH0_CHANNEL LEDC_CHANNEL_0
#define LEDC_HS_CH1_GPIO (18) //上灯条 黄灯
#define LEDC_HS_CH1_CHANNEL LEDC_CHANNEL_1
#define LEDC_HS_CH2_GPIO (16) ///下灯条 白灯
#define LEDC_HS_CH2_CHANNEL LEDC_CHANNEL_2
#define LEDC_HS_CH3_GPIO (17) /// 下灯条 黄灯
#define LEDC_HS_CH3_CHANNEL LEDC_CHANNEL_3

#define LEDC_TEST_CH_NUM (4)
//#define LEDC_TEST_DUTY         (8192)
#define LEDC_TEST_FADE_TIME (100)

uint16_t ctl_duty0;
uint16_t ctl_duty3;
uint16_t ctl_duty2;
uint16_t ctl_duty1;
extern uint64_t Z;

ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
    {.channel = LEDC_HS_CH0_CHANNEL,
     .duty = 0,
     .gpio_num = LEDC_HS_CH0_GPIO,
     .speed_mode = LEDC_HS_MODE,
     .hpoint = 0,
     .timer_sel = LEDC_HS_TIMER},
    {.channel = LEDC_HS_CH1_CHANNEL,
     .duty = 0,
     .gpio_num = LEDC_HS_CH1_GPIO,
     .speed_mode = LEDC_HS_MODE,
     .hpoint = 0,
     .timer_sel = LEDC_HS_TIMER},
    {.channel = LEDC_HS_CH2_CHANNEL,
     .duty = 0,
     .gpio_num = LEDC_HS_CH2_GPIO,
     .speed_mode = LEDC_HS_MODE,
     .hpoint = 0,
     .timer_sel = LEDC_HS_TIMER},
    {.channel = LEDC_HS_CH3_CHANNEL,
     .duty = 0,
     .gpio_num = LEDC_HS_CH3_GPIO,
     .speed_mode = LEDC_HS_MODE,
     .hpoint = 0,
     .timer_sel = LEDC_HS_TIMER},

};
void Led_UP_W(uint16_t duty, int fade_time) //上白光控制，duty0-100
{
    esp_err_t ret;
    uint16_t ctl_duty = 8000 - (uint16_t)(80.00 * (float)duty); //将0-100变为8000-0
    ret = ledc_set_fade_time_and_start(ledc_channel[0].speed_mode, ledc_channel[0].channel, ctl_duty, fade_time, ledc_channel[0].speed_mode);
    //printf("ret = %c\r\n", ret);
    if (ret != ESP_OK)
    {
        Pwm_Init();
        printf("ERROR\r\n");
    }
    else
    {
        printf("Led_UP_W=%d\r\n", ctl_duty);
    }
    //ret = ledc_set_fade_with_time(ledc_channel[0].speed_mode, ledc_channel[0].channel, ctl_duty, fade_time);
    //ledc_fade_start(ledc_channel[0].speed_mode, ledc_channel[0].channel, LEDC_FADE_NO_WAIT);
}
void Led_UP_Y(uint16_t duty, int fade_time) //上黄光控制，duty0-100
{
    esp_err_t ret;
    uint16_t ctl_duty = 8000 - (uint16_t)(80.00 * (float)duty); //将0-100变为8000-0
    ret = ledc_set_fade_time_and_start(ledc_channel[1].speed_mode, ledc_channel[1].channel, ctl_duty, fade_time, ledc_channel[1].speed_mode);
    //printf("ret = %c\r\n", ret);
    if (ret != ESP_OK)
    {
        Pwm_Init();
        printf("ERROR\r\n");
    }
    else
    {
        printf("Led_UP_Y=%d\r\n", ctl_duty);
    }
    //ledc_set_fade_with_time(ledc_channel[1].speed_mode, ledc_channel[1].channel, ctl_duty, fade_time);
    //ledc_fade_start(ledc_channel[1].speed_mode, ledc_channel[1].channel, LEDC_FADE_NO_WAIT);
    //printf("Led_UP_Y=%d\r\n", ctl_duty);
}
void Led_DOWN_W(uint16_t duty, int fade_time) //下白光控制，duty0-100
{
    esp_err_t ret;
    uint16_t ctl_duty = 8000 - (uint16_t)(80.00 * (float)duty); //将0-100变为8000-0
    ret = ledc_set_fade_time_and_start(ledc_channel[2].speed_mode, ledc_channel[2].channel, ctl_duty, fade_time, ledc_channel[2].speed_mode);
    //printf("ret = %c\r\n", ret);
    if (ret != ESP_OK)
    {
        Pwm_Init();
        printf("ERROR\r\n");
    }
    else
    {
        printf("Led_DOWN_W=%d\r\n", ctl_duty);
    }
    //ledc_set_fade_with_time(ledc_channel[2].speed_mode, ledc_channel[2].channel, ctl_duty, fade_time);
    //ledc_fade_start(ledc_channel[2].speed_mode, ledc_channel[2].channel, LEDC_FADE_NO_WAIT);
    //printf("Led_DOWN_W=%d\r\n", ctl_duty);
}
void Led_DOWN_Y(uint16_t duty, int fade_time) //下黄光控制，duty0-100
{
    esp_err_t ret;
    uint16_t ctl_duty = 8000 - (uint16_t)(80.00 * (float)duty);
    ret = ledc_set_fade_time_and_start(ledc_channel[3].speed_mode, ledc_channel[3].channel, ctl_duty, fade_time, ledc_channel[3].speed_mode);
    //printf("ret = %c\r\n", ret);
    if (ret != ESP_OK)
    {
        Pwm_Init();
        printf("ERROR\r\n");
    }
    else
    {
        printf("Led_DOWN_Y=%d\r\n", ctl_duty);
    }
    //ledc_set_fade_with_time(ledc_channel[3].speed_mode, ledc_channel[3].channel, ctl_duty, fade_time);
    //ledc_fade_start(ledc_channel[3].speed_mode, ledc_channel[3].channel, LEDC_FADE_NO_WAIT);
    //printf("Led_DOWN_Y=%d\r\n", ctl_duty);
}

uint64_t Led_Color_CTL(uint16_t color_temp, int fade_time)
{

    if (color_temp == 3000) //仅黄灯
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty2;
            duty2 = 100 - Z;
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(100, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty2;
            duty2 = 100 - Z;
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(100, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 3100) //  白灯/黄灯 = 5/95 eg：Z = 60；白灯为60*0.05=3 黄灯为60*0.95=57
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 5 / 95;
            duty1 = 100 - Z * 0.05;
            duty2 = 100 - Z * 0.95;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 5 / 95;
            duty1 = 100 - Z * 0.05;
            duty2 = 100 - Z * 0.95;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 3200)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 10 / 90;
            duty1 = 100 - Z * 0.1;
            duty2 = 100 - Z * 0.9;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 10 / 90;
            duty1 = 100 - Z * 0.1;
            duty2 = 100 - Z * 0.9;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 3300)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 15 / 85;
            duty1 = 100 - Z * 0.15;
            duty2 = 100 - Z * 0.85;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 15 / 85;
            duty1 = 100 - Z * 0.15;
            duty2 = 100 - Z * 0.85;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 3400)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 20 / 80;
            duty1 = 100 - Z * 0.2;
            duty2 = 100 - Z * 0.8;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 20 / 80;
            duty1 = 100 - Z * 0.2;
            duty2 = 100 - Z * 0.8;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 3500)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 23 / 77;
            duty1 = 100 - Z * 0.23;
            duty2 = 100 - Z * 0.77;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 23 / 77;
            duty1 = 100 - Z * 0.23;
            duty2 = 100 - Z * 0.77;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 3510)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 34 / 66;
            duty1 = 100 - Z * 0.34;
            duty2 = 100 - Z * 0.66;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 34 / 66;
            duty1 = 100 - Z * 0.34;
            duty2 = 100 - Z * 0.66;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 3520)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 35 / 65;
            duty1 = 100 - Z * 0.35;
            duty2 = 100 - Z * 0.65;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 35 / 65;
            duty1 = 100 - Z * 0.35;
            duty2 = 100 - Z * 0.65;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 3530)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 40 / 60;
            duty1 = 100 - Z * 0.40;
            duty2 = 100 - Z * 0.60;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 40 / 60;
            duty1 = 100 - Z * 0.40;
            duty2 = 100 - Z * 0.60;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 3540)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 42 / 58;
            duty1 = 100 - Z * 0.42;
            duty2 = 100 - Z * 0.58;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 42 / 58;
            duty1 = 100 - Z * 0.42;
            duty2 = 100 - Z * 0.58;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }

    else if (color_temp == 4000)
    {
        //printf("灯自动运行2\r\n");
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 45 / 55;
            duty1 = 100 - Z * 0.45;
            duty2 = 100 - Z * 0.55;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time); //45 55
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 45 / 55;
            duty1 = 100 - Z * 0.45;
            duty2 = 100 - Z * 0.55;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 4100)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 50 / 50;
            duty1 = 100 - Z * 0.50;
            duty2 = 100 - Z * 0.50;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time); //45 55
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 50 / 50;
            duty1 = 100 - Z * 0.50;
            duty2 = 100 - Z * 0.50;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 4200)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 55 / 45;
            duty1 = 100 - Z * 0.55;
            duty2 = 100 - Z * 0.45;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time); //45 55
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 55 / 45;
            duty1 = 100 - Z * 0.55;
            duty2 = 100 - Z * 0.45;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 4300)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 60 / 40;
            duty1 = 100 - Z * 0.60;
            duty2 = 100 - Z * 0.40;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time); //45 55
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 60 / 40;
            duty1 = 100 - Z * 0.60;
            duty2 = 100 - Z * 0.40;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 4400)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 65 / 35;
            duty1 = 100 - Z * 0.65;
            duty2 = 100 - Z * 0.35;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time); //45 55
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 65 / 35;
            duty1 = 100 - Z * 0.65;
            duty2 = 100 - Z * 0.35;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }

    else if (color_temp == 4500)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 72 / 28;
            duty1 = 100 - Z * 0.72;
            duty2 = 100 - Z * 0.28;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {

            uint16_t duty1;
            uint16_t duty2;
            u = 72 / 28;
            duty1 = 100 - Z * 0.72;
            duty2 = 100 - Z * 0.28;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 4600)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 79 / 21;
            duty1 = 100 - Z * 0.79;
            duty2 = 100 - Z * 0.21;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {

            uint16_t duty1;
            uint16_t duty2;
            u = 79 / 21;
            duty1 = 100 - Z * 0.79;
            duty2 = 100 - Z * 0.21;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 4700)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 86 / 14;
            duty1 = 100 - Z * 0.86;
            duty2 = 100 - Z * 0.14;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {

            uint16_t duty1;
            uint16_t duty2;
            u = 86 / 14;
            duty1 = 100 - Z * 0.86;
            duty2 = 100 - Z * 0.14;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 4800)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 93 / 7;
            duty1 = 100 - Z * 0.93;
            duty2 = 100 - Z * 0.07;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {

            uint16_t duty1;
            uint16_t duty2;
            u = 93 / 7;
            duty1 = 100 - Z * 0.93;
            duty2 = 100 - Z * 0.07;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 4900)
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            uint16_t duty2;
            u = 97 / 3;
            duty1 = 100 - Z * 0.97;
            duty2 = 100 - Z * 0.03;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(duty2, fade_time);
        }
        if (Down_Light_Status == 1)
        {

            uint16_t duty1;
            uint16_t duty2;
            u = 97 / 3;
            duty1 = 100 - Z * 0.97;
            duty2 = 100 - Z * 0.03;
            printf("duty1=%d\r\n", duty1);
            printf("duty2=%d\r\n", duty2);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(duty2, fade_time);
        }
    }
    else if (color_temp == 5000) //仅白灯
    {
        if (Up_Light_Status == 1)
        {
            uint16_t duty1;
            duty1 = 100 - Z;
            //printf("duty1=%d\r\n", duty1);
            printf("duty1=%d\r\n", duty1);
            Led_UP_W(duty1, fade_time);
            Led_UP_Y(100, fade_time);
        }
        if (Down_Light_Status == 1)
        {

            uint16_t duty1;
            duty1 = 100 - Z;
            //printf("duty1=%d\r\n", duty1);
            printf("duty1=%d\r\n", duty1);
            Led_DOWN_W(duty1, fade_time);
            Led_DOWN_Y(100, fade_time);
        }
    }

    return Z;
}

/*void Led_Time_Ctl_Task(void *arg)
{
    while (1)
    {
        if ((Up_Light_Status == 1) || (Down_Light_Status == 1) || (start_read_blue_ret == BLU_COMMAND_SWITCH) || BLU_COMMAND_CALCULATION)
        {
            Led_Time_Ctl();
            //printf("灯自动运行中\r\n");
        }
        vTaskDelay(100 / portTICK_RATE_MS);
    }
    //vTaskDelete(NULL);
}*/

void Led_Time_Ctl(void)
{

    int year, month, day, hour, min, sec;
    Rtc_Read(&year, &month, &day, &hour, &min, &sec);

    if (temp_hour != hour)
    {

        if ((hour == 0) && (min < 5))
        {
            if (min == 1)
            {
                color_temp = 3400;
            }
            else if (min == 2)
            {
                color_temp = 3300;
            }
            else if (min == 3)
            {
                color_temp = 3200;
            }
            else if (min == 4)
            {
                color_temp = 3100;
            }
        }

        else if (((hour >= 1) && (hour <= 7)) || ((hour == 0) && (min >= 5)))
        {
            color_temp = 3000;
        }
        else if ((hour == 8) && (min < 5))
        {
            if (min == 1)
            {
                color_temp = 3100;
            }
            else if (min == 2)
            {
                color_temp = 3200;
            }
            else if (min == 3)
            {
                color_temp = 3300;
            }
            else if (min == 4)
            {
                color_temp = 3400;
            }
        }
        else if ((hour == 8) && (min >= 5))
        {
            color_temp = 3500;
        }

        else if ((hour == 9) && (min < 5))
        {
            if (min == 1)
            {
                color_temp = 3510;
            }
            else if (min == 2)
            {
                color_temp = 3520;
            }
            else if (min == 3)
            {
                color_temp = 3530;
            }
            else if (min == 4)
            {
                color_temp = 3540;
            }
        }
        else if ((hour == 9) && (min >= 5))
        {
            color_temp = 4000;
        }

        else if ((hour == 10) && (min < 5))
        {
            if (min == 1)
            {
                color_temp = 4100;
            }
            else if (min == 2)
            {
                color_temp = 4200;
            }
            else if (min == 3)
            {
                color_temp = 4300;
            }
            else if (min == 4)
            {
                color_temp = 4400;
            }
        }
        else if ((hour == 10) && (min >= 5))
        {
            color_temp = 4500;
        }

        else if ((hour == 11) && (min < 5))
        {
            if (min == 1)
            {
                color_temp = 4600;
            }
            else if (min == 2)
            {
                color_temp = 4700;
            }
            else if (min == 3)
            {
                color_temp = 4800;
            }
            else if (min == 4)
            {
                color_temp = 4900;
            }
            /*vTaskDelay(60000 / portTICK_RATE_MS);
            color_temp = 4600;
            vTaskDelay(60000 / portTICK_RATE_MS);
            color_temp = 4700;
            vTaskDelay(60000 / portTICK_RATE_MS);
            color_temp = 4800;
            vTaskDelay(60000 / portTICK_RATE_MS);
            color_temp = 4900;
            vTaskDelay(60000 / portTICK_RATE_MS);*/
        }
        else if (((hour >= 12) && (hour <= 13)) || ((hour == 11) && (min >= 5)))
        {
            color_temp = 5000;
        }

        else if ((hour == 14) && (min < 5))
        {
            if (min == 1)
            {
                color_temp = 4900;
            }
            else if (min == 2)
            {
                color_temp = 4800;
            }
            else if (min == 3)
            {
                color_temp = 4700;
            }
            else if (min == 4)
            {
                color_temp = 4600;
            }
        }

        else if (((hour > 14) && (hour <= 15)) || ((hour == 14) && (min >= 5)))
        {

            color_temp = 4500;
        }
        else if ((hour == 16) && (min < 5))
        {
            if (min == 1)
            {
                color_temp = 4400;
            }
            else if (min == 2)
            {
                color_temp = 4300;
            }
            else if (min == 3)
            {
                color_temp = 4200;
            }
            else if (min == 4)
            {
                color_temp = 4100;
            }
        }
        else if (((hour >= 17) && (hour <= 21)) || ((hour == 16) && (min >= 5)))
        {
            color_temp = 4000;
            //printf("灯自动运行2\r\n");
        }
        else if ((hour == 22) && (min < 5))
        {
            if (min == 1)
            {
                color_temp = 3540;
            }
            else if (min == 2)
            {
                color_temp = 3530;
            }
            else if (min == 3)
            {
                color_temp = 3520;
            }
            else if (min == 4)
            {
                color_temp = 3510;
            }
        }

        else if (((hour > 22) && (hour <= 23)) || ((hour == 22) && (min >= 5)))
        {
            color_temp = 3500;
        }

        if ((temp_hour == -1) || (human_status == HAVEHUMAN)) //开机或者开关控制，1s到达指定亮度
        {
            temp_hour = hour;
            temp_min = min;
            Led_Color_CTL(color_temp, ON_TIME);
            Led_Status = LED_STA_AUTO;
            printf("灯自动运行1\r\n");
            //Led_Color_CTL(color_temp, ON_TIME);
        }
        else if ((human_status == NOHUMAN) && (work_status != WORK_HAND))
        {

            Led_DOWN_W(100, 800);
            Led_DOWN_Y(100, 800);
            Led_UP_W(100, 800);
            Led_UP_Y(100, 800);
            //Led_Status = LED_STA_NOSER;
            printf("无人\r\n");
        }
        else
        {
            temp_hour = hour;
            temp_min = min;
            //Led_Color_CTL(color_temp, ON_TIME);
            Led_Color_CTL(color_temp, COLOR_CHANGE_TIME);
            strcpy(mqtt_json_s.mqtt_mode, "1");

            printf("灯自动运行2\r\n");
        }
        //printf("color_temp2=%d\r\n", color_temp);
    }
}
void Led_Time_Ctl_Task(void *arg)
{
    while (1)
    {
        if ((Up_Light_Status == 1) || (Down_Light_Status == 1)) //|| (start_read_blue_ret == BLU_COMMAND_SWITCH) || BLU_COMMAND_CALCULATION)
        {

            Led_Time_Ctl();

            //printf("灯自动运行中\r\n");
        }
        vTaskDelay(100 / portTICK_RATE_MS);
    }
    //vTaskDelete(NULL);
}

void Pwm_Init(void)
{
    int ch;

    Z1 = 0;

    //vTaskDelay(5000 / portTICK_RATE_MS);
    temp_hour = -1;
    Up_Light_Status = 1;
    Down_Light_Status = 1;

    //color_temp = 0;
    //color_temp = 0;

    //PWM频率 5KHZ  占空比个数 0-8000
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 3500,                      // frequency of PWM signal
        .speed_mode = LEDC_HS_MODE,           // timer mode
        .timer_num = LEDC_HS_TIMER            // timer index
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    // Set LED Controller with previously prepared configuration
    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++)
    {
        ledc_channel_config(&ledc_channel[ch]);
    }

    // Initialize fade service.
    ledc_fade_func_install(0);

    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++)
    {
        ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, 0);
        ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
    }

    //temp_hour = -1;

    printf("led start on\r\n");
    xTaskCreate(Led_Time_Ctl_Task, "Led_Time_Ctl_Task", 2048, NULL, 9, NULL);
}