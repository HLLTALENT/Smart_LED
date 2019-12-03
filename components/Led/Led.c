#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "Led.h"

#define GPIO_LED_G 22
#define GPIO_LED_R 23
#define GPIO_LED_B 21

void Led_R_On(void);
void Led_G_On(void);
void Led_B_On(void);
void Led_Y_On(void);
void Led_C_On(void);
void Led_Off(void);

static void Led_Task(void *arg)
{
    while (1)
    {
        switch (Led_Status)
        {
        case LED_STA_INIT:
            Led_Y_On();
            vTaskDelay(300 / portTICK_RATE_MS);
            Led_Off();
            vTaskDelay(300 / portTICK_RATE_MS);
            break;

        case LED_STA_TOUCH:
            Led_R_On();
            vTaskDelay(300 / portTICK_RATE_MS);
            Led_G_On();
            vTaskDelay(300 / portTICK_RATE_MS);
            break;

        case LED_STA_NOSER:
            Led_R_On();
            vTaskDelay(10 / portTICK_RATE_MS);
            break;

        case LED_STA_WIFIERR:
            Led_B_On();
            vTaskDelay(300 / portTICK_RATE_MS);
            Led_Off();
            vTaskDelay(300 / portTICK_RATE_MS);
            break;

        case LED_STA_HAND:
            Led_Y_On();
            vTaskDelay(10 / portTICK_RATE_MS);
            break;

        case LED_STA_LOCAL:
            Led_C_On();
            vTaskDelay(10 / portTICK_RATE_MS);
            break;

        case LED_STA_AUTO:
            Led_G_On();
            vTaskDelay(10 / portTICK_RATE_MS);
            break;

        case LED_STA_PROTECT:
            Led_G_On();
            vTaskDelay(300 / portTICK_RATE_MS);
            Led_Off();
            vTaskDelay(300 / portTICK_RATE_MS);
            break;
            /*case LED_KEY_ID:
            Led_R_On();
            vTaskDelay(300 / portTICK_RATE_MS);
            Led_Off();
            vTaskDelay(300 / portTICK_RATE_MS);
            break;*/

            //case LED_STA_FIRE:
            // Led_R_On();
            // vTaskDelay(300 / portTICK_RATE_MS);
            //// Led_Off();
            //vTaskDelay(300 / portTICK_RATE_MS);
            // break;
        }
    }
}

void Led_Init(void)
{
    gpio_config_t io_conf;

    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO16
    io_conf.pin_bit_mask = (1 << GPIO_LED_G);
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1 << GPIO_LED_R);
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1 << GPIO_LED_B);
    gpio_config(&io_conf);

    //Led_Status = LED_STA_INIT;

    xTaskCreate(Led_Task, "Led_Task", 4096, NULL, 5, NULL);
}

void Led_R_On(void)
{
    gpio_set_level(GPIO_LED_R, 0);
    gpio_set_level(GPIO_LED_G, 1);
    gpio_set_level(GPIO_LED_B, 1);
}

void Led_G_On(void)
{
    gpio_set_level(GPIO_LED_R, 1);
    gpio_set_level(GPIO_LED_G, 0);
    gpio_set_level(GPIO_LED_B, 1);
}

void Led_B_On(void)
{
    gpio_set_level(GPIO_LED_R, 1);
    gpio_set_level(GPIO_LED_G, 1);
    gpio_set_level(GPIO_LED_B, 0);
}

void Led_Y_On(void)
{
    gpio_set_level(GPIO_LED_R, 0);
    gpio_set_level(GPIO_LED_G, 0);
    gpio_set_level(GPIO_LED_B, 1);
}

void Led_C_On(void) //青色
{
    gpio_set_level(GPIO_LED_R, 1);
    gpio_set_level(GPIO_LED_G, 0);
    gpio_set_level(GPIO_LED_B, 0);
}

void Led_Off(void)
{
    gpio_set_level(GPIO_LED_R, 1);
    gpio_set_level(GPIO_LED_G, 1);
    gpio_set_level(GPIO_LED_B, 1);
}
