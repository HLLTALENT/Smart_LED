#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_task_wdt.h"
#include "Wallkey.h"
#include "Pwm.h"
#include "Led.h"
#include "Human.h"
#include "Json_parse.h"

#define UART1_TXD (UART_PIN_NO_CHANGE)
#define UART1_RXD (GPIO_NUM_5)
#define UART1_RTS (UART_PIN_NO_CHANGE)
#define UART1_CTS (UART_PIN_NO_CHANGE)

#define BUF_SIZE 100
static const char *TAG = "WALLKEY";

int8_t number = 0;

//uint8_t Key_Id[4] = {0x99, 0x17, 0x06, 0x00}; //{0x86, 0x6d, 0x02, 0x00};

int8_t Wallkey_Read(uint8_t *Key_Id, int8_t Switch)
{
    uint8_t data_u1[100];

    int len1 = uart_read_bytes(UART_NUM_1, data_u1, BUF_SIZE, 20 / portTICK_RATE_MS);
    if (len1 != 0) //读取到按键数据
    {
        len1 = 0;
        if ((data_u1[0] == 0x7e) && (data_u1[8] == 0xef)) //校验数据头和第9个字节固定为0XEF
        {
            if ((data_u1[3] == Key_Id[0]) && (data_u1[4] == Key_Id[1]) && (data_u1[5] == Key_Id[2]) && (data_u1[6] == Key_Id[3])) //校验KEY ID是否满足
            {
                //if (Switch == 0) //左边
                //{
                if (data_u1[9] == KEY_SIN)
                {
                    ESP_LOGI(TAG, "KEY_SIN");
                    //vTaskDelay(2000 / portTICK_RATE_MS);
                }

                else if (data_u1[9] == KEY_SIN_RELEASE)
                {
                    ESP_LOGI(TAG, "KEY_RELEASE");
                    vTaskDelay(2000 / portTICK_RATE_MS);
                }
                //}
                /*if (data_u1[9] == KEY_SIN)
                {
                    ESP_LOGI(TAG, "KEY_SIN");
                    Led_R_On();
                    vTaskDelay(1000 / portTICK_RATE_MS);
                    Led_Off();
                    vTaskDelay(1000 / portTICK_RATE_MS);
                }

                else if (data_u1[9] == KEY_SIN_RELEASE)
                {
                    ESP_LOGI(TAG, "KEY_RELEASE");
                }*/
                return data_u1[9];
            }
            else
            {
                ESP_LOGE(TAG, "Key ID Refuse,ID=%02x-%02x-%02x-%02x", data_u1[3], data_u1[4], data_u1[5], data_u1[6]);
                ESP_LOGE(TAG, "Key value=%02x", data_u1[9]);
                return -1;
            }
        }
    }
    return -1;
}

static void Wallkey_Read_Task(void *arg) //void Wallkey_App(uint8_t *Key_Id, int8_t Switch)
{

    int8_t key_read = 0;
    while (1)
    {
        key_read = Wallkey_Read(ob_blu_json.WallKeyId, ob_blu_json.Switch);
        if ((key_read == KEY_SIN) && (Up_Light_Status == 1) && (Down_Light_Status == 1))
        {
            auto_ctl_count1 = 0;
            Led_Status = LED_STA_NOSER;
            Led_UP_W(100, 100);
            Led_UP_Y(100, 100);
            Led_DOWN_W(100, 100);
            Led_DOWN_Y(100, 100);

            printf("全关\r\n");
            Down_Light_Status = 0;
            Up_Light_Status = 0;
        }
        else if ((key_read == KEY_SIN) && (Up_Light_Status == 0) && (Down_Light_Status == 0))
        {
            auto_ctl_count1 = 0;
            Led_Status = LED_STA_AUTO; //绿灯亮
            Down_Light_Status = 1;
            Up_Light_Status = 0;
            temp_hour = -1;
            printf("下亮\r\n");
        }
        else if ((key_read == KEY_SIN) && (Down_Light_Status == 1) && (Up_Light_Status == 0))
        {
            auto_ctl_count1 = 0;
            Led_Status = LED_STA_AUTO; //绿灯亮
            Down_Light_Status = 0;
            Up_Light_Status = 1;
            temp_hour = -1;
            printf("全亮\r\n");
        }
        else if ((key_read == KEY_SIN) && (Down_Light_Status == 0) && (Up_Light_Status == 1))
        {
            auto_ctl_count1 = 0;
            Led_Status = LED_STA_AUTO; //绿灯亮
            temp_hour = -1;
            Led_DOWN_W(100, 100);
            Led_DOWN_Y(100, 100);
            printf("全开\r\n");

            Down_Light_Status = 1;
            Up_Light_Status = 1;
        }
        //vTaskDelay(10 / portTICK_RATE_MS);
    }

    /* if ((key_read == KEY_SIN) && (number == 0))
    {
        auto_ctl_count1 = 0;
        //work_status = WORK_WALLKEY;
        strcpy(mqtt_json_s.mqtt_mode, "0");
        strcpy(mqtt_json_s.mqtt_human_char, "1");
        Led_Status = LED_STA_AUTO; //绿灯亮

        Up_Light_Status = 0;
        Down_Light_Status = 0;

        Led_UP_W(100, 500);
        Led_UP_Y(100, 500);
        Led_DOWN_W(100, 500);
        Led_DOWN_Y(100, 500);
        //temp_hour = -1;
        //Up_Light_Status = 0;
        //Down_Light_Status = 0;
        number = 1;

        printf("全关\r\n");
    }
    else if ((key_read == KEY_SIN) && (number == 1))
    {
        auto_ctl_count1 = 0;

        //work_status = WORK_WALLKEY;
        strcpy(mqtt_json_s.mqtt_mode, "0");

        strcpy(mqtt_json_s.mqtt_human_char, "1");
        Led_Status = LED_STA_AUTO;
        Up_Light_Status = 1;
        Down_Light_Status = 0;

        temp_hour = -1;

        number = 0;

        printf("全开\r\n");
    }
    //printf("number=%d\r\n", number);*/

    /*else if ((key_read == KEY_SIN) && (number == 2))
    {
        auto_ctl_count1 = 0;

        //work_status = WORK_WALLKEY;
        strcpy(mqtt_json_s.mqtt_mode, "0");

        strcpy(mqtt_json_s.mqtt_human_char, "1");
        Led_Status = LED_STA_AUTO;

        Down_Light_Status = 0;
        Up_Light_Status = 1;

        Led_DOWN_W(100, 500);
        Led_DOWN_Y(100, 500);
        //temp_hour = -1;
        number = 3;
        printf("上开");
    }
    else if ((key_read == KEY_SIN) && (number == 3))
    {
        auto_ctl_count1 = 0;

        //work_status = WORK_WALLKEY;
        strcpy(mqtt_json_s.mqtt_mode, "0");

        Up_Light_Status = 0;
        //Down_Light_Status = 0;
        Down_Light_Status = 1;
        //temp_hour = -1;
        Led_UP_W(100, 500);
        Led_UP_Y(100, 500);
        strcpy(mqtt_json_s.mqtt_mode, "1");
        Led_Status = LED_STA_NOSER;
        printf("下开");
        number = 0;
    }*/
}
void Wallkey_Init(void)
{
    //配置GPIO
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = 1 << UART1_RXD;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, UART1_TXD, UART1_RXD, UART1_RTS, UART1_CTS);
    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);

    xTaskCreate(Wallkey_Read_Task, "Wallkey_Read_Task", 2048, NULL, 12, NULL);
}
