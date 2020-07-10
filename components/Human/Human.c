#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "Human.h"
#include "Pwm.h"
#include "Led.h"
#include "Json_parse.h"
#include "Http.h"
#include "Wallkey.h"

#define TAG "HUNAN"

int human_gpio_value;
uint8_t human_status;

void Human_Init(void)
{
    //配置继电器输出管脚
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.pin_bit_mask = 1 << GPIO_HUMAN;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 1;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    //human_status = HAVEHUMAN;
}

void Humanapp(void)
{

    human_gpio_value = gpio_get_level(GPIO_HUMAN); //读取人感电平
    ESP_LOGD(TAG, "human_gpio_value=%d\n", human_gpio_value);

    if (Wallkey_status == 0)
    {
        if (human_gpio_value == 1) //传感器报有人
        {
            havehuman_count++;
            ESP_LOGD(TAG, "havehuman_count=%d\n", havehuman_count);
            //human_status = HAVEHUMAN;
            //printf("human_status=%d\n", human_status);
        }
        if (human_gpio_value == 0) //传感器报无人
        {
            vTaskDelay(1 / portTICK_RATE_MS);
        }
    }
    else if (Wallkey_status == 1)
    {
        //human_status = HAVEHUMAN;
        //vTaskDelay(60000 / portTICK_RATE_MS);
        //human_status = 1;
    }
    /*if (human_gpio_value == 0) //传感器报无人
    {
        human_status = NOHUMAN;
        printf("human_status=%d\n", human_status);
    }*/
}

void Human_Task(void *arg)
{
    while (1)
    {
        Humanapp();

        vTaskDelay(200 / portTICK_RATE_MS);
    }
}
