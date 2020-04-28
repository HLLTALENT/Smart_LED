#ifndef _PWM_H_
#define _PWM_H_
#include "freertos/FreeRTOS.h"
#include "driver/ledc.h"

#define LEDC_HS_TIMER LEDC_TIMER_0
#define LEDC_HS_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO (19)
#define LEDC_HS_CH0_CHANNEL LEDC_CHANNEL_0
#define LEDC_HS_CH1_GPIO (18)
#define LEDC_HS_CH1_CHANNEL LEDC_CHANNEL_1
#define LEDC_HS_CH2_GPIO (16)
#define LEDC_HS_CH2_CHANNEL LEDC_CHANNEL_2
#define LEDC_HS_CH3_GPIO (17)
#define LEDC_HS_CH3_CHANNEL LEDC_CHANNEL_3

#define LEDC_TEST_CH_NUM (4)
//#define LEDC_TEST_DUTY         (8192)
#define LEDC_TEST_FADE_TIME (100)

extern void Pwm_Init(void);
extern void Led_UP_W(uint16_t duty, int fade_time);
extern void Led_UP_Y(uint16_t duty, int fade_time);
extern void Led_DOWN_W(uint16_t duty, int fade_time);
extern void Led_DOWN_Y(uint16_t duty, int fade_time);
extern uint64_t Led_Color_CTL(uint16_t color_temp, int fade_time);
extern void Led_Time_Ctl(void);
extern void Led_Time_Ctl_Task(void *arg);

int temp_hour;
int temp_min;
uint8_t Up_Light_Status, Down_Light_Status;
uint16_t color_temp;

uint16_t ctl_duty0;
uint16_t ctl_duty3;
uint16_t ctl_duty2;
uint16_t ctl_duty1;

//#define COLOR_CHANGE_TIME   600000
#define COLOR_CHANGE_TIME 350000
#define ON_TIME 1000
#define OFF_TIME 2000

#endif
