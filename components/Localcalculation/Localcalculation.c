#include <stdio.h>
#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#include "Http.h"
#include "Mqtt.h"

#include "Localcalculation.h"
#include "Pwm.h"
#include "opt3001.h"
#include "Wallkey.h"
#include "Human.h"
#include "Json_parse.h"
#include "Led.h"

float H = 1.5; //高度
float a;       //系数1
float b;       //系数2
float Y;       //照度
uint64_t Z;    //占空比
uint64_t Z1;
float y; //照度差
//float u;      //???
float lightX; //OPT3001照度

void Localcalculation(float lightX, uint16_t color_temp, int fade_time)
{
    //human_status = 1;
    //H = orientation;
    //printf("lightX=%f\n", lightX);
    //printf("orientation_local = %f\r\n", orientation_local);

    /*if (((hour * 60 + minute) >= (T2_h * 60 + T2_m)) && ((hour * 60 + minute) < (T3_h * 60 + T3_m)))
    {

        human_status = 0;
        Led_UP_W(100, OFF_TIME);
        Led_UP_Y(100, OFF_TIME);
        Led_DOWN_W(100, OFF_TIME);
        Led_DOWN_Y(100, OFF_TIME);

        if (key_read == KEY_SIN)
        {
            temp_hour = -1;
            Up_Light_Status = 1;
            Down_Light_Status = 1;
        }

        //X = 1;
    }*/

    // else
    //{
    //printf("H =%f\r\n", H);
    if (H <= 1.5)
    {
        b = -1.9704 * H + 3.715;
    }
    else if (H > 1.5)
    {
        b = 0.6604 * H - 0.2044;
    }

    a = 3.8178 * H + 2.9131;
    Z1 = Led_Color_CTL(color_temp, fade_time);
    printf("Z1=%lld\r\n", Z1);
    Y = a * (lightX - b * Z1) + ((15.6 * H * H - 63 * H + 76.565) * Z1);
    y = 500 - Y;
    printf("Y=%f\r\n", Y);
    mqtt_json_s.mqtt_Y = Y;

    Z = Z1 + y / (15.6 * H * H - 63 * H + 76.565);
    if ((Z < 0) || (Z > 60))
    {
        Z = 0;
        printf("Z=%lld\r\n", Z);
        strcpy(mqtt_json_s.mqtt_light_char, "0");
        Led_Status = LED_STA_NOSER;
    }
    else if ((Z > 0) && (Z <= 60))
    {
        strcpy(mqtt_json_s.mqtt_light_char, "1");
        //strcpy(mqtt_json_s.mqtt_human_char, "1");
        printf("Z=%lld\r\n", Z);
        //Led_Status = LED_STA_AUTO;
    }
    printf("lightvalue = %f\r\n", lightX);
    printf("Y=%f\r\n", Y);
    printf("Z=%lld\r\n", Z);
}

void Localcalculationlunchtime(int year, int month, int day, int hour, int minute, int T2_h, int T2_m, int T3_h, int T3_m)
{
    if (((hour * 60 + minute) >= (T2_h * 60 + T2_m)) && ((hour * 60 + minute) < (T3_h * 60 + T3_m)))
    {
        Led_UP_W(100, 100);
        Led_UP_Y(100, 100);
        Led_DOWN_W(100, 100);
        Led_DOWN_Y(100, 100);
        work_status = LUNCHTIME;
    }
}
