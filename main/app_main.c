#include <stdio.h>
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"

#include "Smartconfig.h"
#include "Http.h"
#include "Nvs.h"
#include "Mqtt.h"
#include "Bluetooth.h"
#include "Json_parse.h"

#include "Uart0.h"
//#include "Motorctl.h"
#include "Wallkey.h"
#include "Led.h"
#include "E2prom.h"
#include "Localcalculation.h"
#include "SD25RTC.h"

#include "RtcUsr.h"
#include "Wallkey.h"
#include "SD25RTC.h"
#include "Pwm.h"
#include "Human.h"
#include "Led.h"
#include "opt3001.h"

#include "Json_parse.h"
#include "Uart0.h"

int year, month, day, hour, min, sec;
extern uint8_t human_status;
extern int human_gpio_value;
extern float lightX;
extern uint64_t Z;
extern uint16_t ctl_duty3;
extern uint16_t ctl_duty2;

void timer_periodic_cb(void *arg);

esp_timer_handle_t timer_periodic_handle = 0; //定时器句柄

esp_timer_create_args_t timer_periodic_arg = {
    .callback =
        &timer_periodic_cb,
    .arg = NULL,
    .name = "PeriodicTimer"};

void timer_periodic_cb(void *arg) //200ms中断一次
{
    static uint64_t timer_count = 0;  //人感
    static uint64_t timer_count1 = 0; //照度算法
    static uint64_t timer_count2 = 0; //串口打印
    static uint64_t nohuman_timer_count = 0;

    //uint8_t key = 0;
    timer_count++;
    timer_count1++;
    timer_count2++;
    nohuman_timer_count++;
    //printf("nohuman_timer_count=%lld\n", nohuman_timer_count);

    /////////////////////////////////
    if (timer_count1 >= 5) //1s
    {
        timer_count1 = 0;
        if ((temp_hour == -1) || (human_status == HAVEHUMAN))
        {
            Localcalculation(lightX, color_temp, 1000);
        }

        Localcalculationlunchtime(year, month, day, hour, min, ob_blu_json.T2_h, ob_blu_json.T2_m, ob_blu_json.T3_h, ob_blu_json.T3_m);

        //ESP_LOGI("wifi", "free Heap:%d,%d", esp_get_free_heap_size(), heap_caps_get_free_size(MALLOC_CAP_8BIT));

        /*if (mqtt_json_s.mqtt_last > 0)
        {
            mqtt_json_s.mqtt_last--;
        }
        if ((mqtt_json_s.mqtt_last <= 0) && ((work_status == WORK_HAND) || (work_status == WORK_WALLKEY))) //开机前状态
        {
            if (mqtt_json_s.mqtt_mode == -1) //开机就没收到过自动控制指令，先计算 后执行 自动模式
            {

                work_status = WORK_HANDTOAUTO; //自动回复切自动
                printf("hand to auto by last time1\n");
            }
            else
            {
                work_status = WORK_HANDTOAUTO; //自动回复切自动
                printf("hand to auto by last time2\n");
            }
        }*/
        auto_ctl_count1++; //无线开关指令计时
        //printf("auto_ctl_count=%d\n", auto_ctl_count);
        if (auto_ctl_count1 >= MAX_WALLKEY_TIME) //超时没收到平台自动控制指令,转本地计算 10min
        {
            auto_ctl_count1 = 0;
            Wallkey_status = 0;
            if (human_status == HAVEHUMAN)
            {
                Up_Light_Status = 1;
                Down_Light_Status = 1;
                temp_hour = -1;
                printf("human_status=%d\n", human_status);
            }
            else if (human_status == NOHUMAN)
            {
                Led_DOWN_W(100, 1000);
                Led_DOWN_Y(100, 1000);
                Led_UP_W(100, 1000);
                Led_UP_Y(100, 1000);
                printf("human_status=%d\n", human_status);
            }
        }

        auto_ctl_count++; //平台指令计时
        //printf("auto_ctl_count=%d\n", auto_ctl_count);
        if (auto_ctl_count >= MAX_AUTO_CTL_TIME) //超时没收到平台自动控制指令,转本地计算 10min
        {
            auto_ctl_count = 0;
            if (human_status == HAVEHUMAN)
            {
                Up_Light_Status = 1;
                Down_Light_Status = 1;
                temp_hour = -1;
                printf("human_status=%d\n", human_status);
            }
            else if (human_status == NOHUMAN)
            {
                Led_DOWN_W(100, 1000);
                Led_DOWN_Y(100, 1000);
                Led_UP_W(100, 1000);
                Led_UP_Y(100, 1000);
                printf("human_status=%d\n", human_status);
            }
        }
    }

    /////////////////////////////////

    /*if (timer_count2 >= 25) //5s
    {
        timer_count2 = 0;
        printf("[APP] Free memory: %d bytes\n", esp_get_free_heap_size());
        printf("color_temp1=%d\r\n", color_temp);
        Rtc_Read(&year, &month, &day, &hour, &min, &sec);
        printf("Time:%d-%d-%d %d:%d:%d\r\n", year, month, day, hour, min, sec);
    }*/

    if (human_status == HAVEHUMAN) //有人时，1s内右2个1则转为有人
    {
        if (timer_count >= 10) //2s
        {
            timer_count = 0;
            if (havehuman_count >= 4)
            {
                human_status = HAVEHUMAN;
                printf("human_status1=%d\n", human_status);
                //mqtt_json_s.mqtt_human = 1;
                //printf("human_status=%d\n", human_status);
                //strcpy(mqtt_json_s.mqtt_human_char, "1");
                //temp_hour = -1;
                havehuman_count = 0;
                nohuman_timer_count = 0;
            }
            else
            {
                havehuman_count = 0;
            }
        }
    }

    if (human_status == NOHUMAN) //无人时，2s内右6个1则转为有人
    {

        if (timer_count >= 10) //2s
        {
            timer_count = 0;
            if (havehuman_count >= 6)
            {
                if (human_status == NOHUMAN) //如当前是无人，立即上传有人
                {
                    //strcpy(mqtt_json_s.mqtt_human_char, "1");
                    need_send = 1;
                }
                human_status = HAVEHUMAN;
                printf("human_status2=%d\n", human_status);
                //strcpy(mqtt_json_s.mqtt_human_char, "1");
                //temp_hour = -1;
                //printf("human_status2=%d\n", human_status);
                havehuman_count = 0;
                nohuman_timer_count = 0;
            }
            else
            {
                havehuman_count = 0;
            }
        }
    }

    if (nohuman_timer_count >= 300) //60s 1min
    {
        human_status = NOHUMAN;
        nohuman_timer_count = 0;
        printf("human_status=%d\n", human_status);
        //Led_DOWN_W(100, 1000);
        //Led_DOWN_Y(100, 1000);
        //Led_UP_W(100, 1000);
        //Led_UP_Y(100, 1000);
        //strcpy(mqtt_json_s.mqtt_human_char, "0");
        //strcpy(mqtt_json_s.mqtt_light_char, "0");
        //mqtt_json_s.mqtt_human = 0;
        //printf("human_status=%d\n", human_status);
    }
}

/*static void Wallkey_Read_Task(void *arg)
{
    while (1)
    {
        Wallkey_App(ob_blu_json.WallKeyId, ob_blu_json.Switch);
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}*/

static void opt3001_task(void *arg)
{
    float lightvalue;

    while (1)
    {

        osi_OPT3001_value(&lightvalue);
        if (lightvalue != 65535)
        {
            lightX = lightvalue;
        }

        //printf("lightvalue = %f\r\n", lightvalue);
        ///vTaskDelay(2000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

static void Uart0_Task(void *arg)
{
    while (1)
    {
        Uart0_read();
        vTaskDelay(10 / portTICK_RATE_MS);
        /*if (Ble_need_restart == 1)
        {
            vTaskDelay(3000 / portTICK_RATE_MS);
            fflush(stdout); //使stdout清空，就会立刻输出所有在缓冲区的内容。
            esp_restart();  //芯片复位 函数位于esp_system.h
        }*/
    }
}

void app_main(void)
{

    nvs_flash_init();

    strcpy(mqtt_json_s.mqtt_mode, "1");
    Ble_need_restart = 0;

    ESP_LOGI("MAIN", "[APP] IDF version: %s", esp_get_idf_version());
    Wallkey_Init();
    E2prom_Init();
    SD25RTC_IIC_Init();
    sd25rtc_init();
    OPT3001_Init();
    Pwm_Init();
    Human_Init();
    Led_Init();

    Uart0_Init();

    xTaskCreate(Uart0_Task, "Uart0_Task", 4096, NULL, 10, NULL);

    /*step1 判断是否有序列号和product id****/
    E2prom_Read(0x30, (uint8_t *)SerialNum, 16);
    printf("SerialNum=%s\n", SerialNum);

    E2prom_Read(0x40, (uint8_t *)ProductId, 32);
    printf("ProductId=%s\n", ProductId);

    if ((SerialNum[0] == 0xff) && (SerialNum[1] == 0xff)) //新的eeprom，先清零
    {
        printf("new eeprom\n");
        char zero_data[512];
        bzero(zero_data, sizeof(zero_data));
        E2prom_Write(0x00, (uint8_t *)zero_data, 256);
        E2prom_BluWrite(0x00, (uint8_t *)zero_data, 512); //清空蓝牙

        E2prom_Read(0x30, (uint8_t *)SerialNum, 16);
        printf("SerialNum=%s\n", SerialNum);

        E2prom_Read(0x40, (uint8_t *)ProductId, 32);
        printf("ProductId=%s\n", ProductId);
    }

    if ((strlen(SerialNum) == 0) || (strlen(ProductId) == 0)) //未获取到序列号或productid，未烧写序列号
    {
        printf("no SerialNum or product id!\n");
        while (1)
        {
            //故障灯
            Led_Status = LED_STA_NOSER;
            vTaskDelay(500 / portTICK_RATE_MS);
        }
    }

    ble_app_start();
    init_wifi();
    start_read_blue_ret = read_bluetooth();
    if (start_read_blue_ret == 0) //未获取到蓝牙配置信息
    {
        printf("no Ble message!waiting for ble message\n");
        Ble_mes_status = BLEERR;
        while (1)
        {
            //故障灯闪烁
            Led_Status = LED_STA_TOUCH;
            vTaskDelay(500 / portTICK_RATE_MS);
            //待蓝牙配置正常后，退出
            if (Ble_mes_status == BLEOK)
            {
                break;
            }
        }
    }
    if (start_read_blue_ret == BLU_RESULT_SUCCESS)
    {
        /*while (http_activate() < 0) //激活
        {
            printf("activate fail\n");
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }*/
        /*E2prom_Read(0x00, (uint8_t *)ApiKey, 32);
        printf("readApiKey=%s\n", ApiKey);
        E2prom_Read(0x20, (uint8_t *)ChannelId, 16);
        printf("readChannelId=%s\n", ChannelId);
        if ((strlen(SerialNum) == 0) || (strlen(ChannelId) == 0)) //未获取到API-KEY，和channelid进行激活流程
        {
            printf("no ApiKey or channelId!\n");

            while (http_activate() < 0) //激活失败
            {
                vTaskDelay(10000 / portTICK_RATE_MS);
            }

            //激活成功
            E2prom_Read(0x00, (uint8_t *)ApiKey, 32);
            printf("ApiKey=%s\n", ApiKey);
            E2prom_Read(0x20, (uint8_t *)ChannelId, 16);
            printf("ChannelId=%s\n", ChannelId);
        }*/

        /*******************************timer 1s init**********************************************/
        esp_err_t err = esp_timer_create(&timer_periodic_arg, &timer_periodic_handle);
        err = esp_timer_start_periodic(timer_periodic_handle, 200000); //创建定时器，单位us，定时200ms
        if (err != ESP_OK)
        {
            printf("timer periodic create err code:%d\n", err);
        }

        xTaskCreate(Human_Task, "Human_Task", 8192, NULL, 10, NULL);
        xTaskCreate(&opt3001_task, "opt3001_task", 4096, NULL, 10, NULL);

        xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                            false, true, portMAX_DELAY); //等待网络连接、
        initialise_http();
        initialise_mqtt();
    }
}
