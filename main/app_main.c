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
#include "Motorctl.h"
#include "Wind.h"
#include "Wallkey.h"
#include "Led.h"
#include "E2prom.h"
#include "Localcalculation.h"
#include "RtcUsr.h"
#include "Fire.h"
#include "SD25RTC.h"

void timer_periodic_cb(void *arg);

esp_timer_handle_t timer_periodic_handle = 0; //定时器句柄

esp_timer_create_args_t timer_periodic_arg = {
    .callback =
        &timer_periodic_cb,
    .arg = NULL,
    .name = "PeriodicTimer"};

void timer_periodic_cb(void *arg) //1ms中断一次
{
    static int64_t timer_count = 0;
    int year, month, day, hour, min, sec;

    static int M1;
    static int M2;

    timer_count++;
    if (timer_count >= 1000) //1s
    {
        timer_count = 0;

        //ESP_LOGI("wifi", "free Heap:%d,%d", esp_get_free_heap_size(), heap_caps_get_free_size(MALLOC_CAP_8BIT));

        if (mqtt_json_s.mqtt_last > 0)
        {
            mqtt_json_s.mqtt_last--;
        }
        if ((mqtt_json_s.mqtt_last <= 0) && ((work_status == WORK_HAND) || (work_status == WORK_WALLKEY)))
        {
            if (mqtt_json_s.mqtt_height == -1) //开机就没收到过自动控制指令，先计算 后执行
            {
                SD25Rtc_Read(&year, &month, &day, &hour, &min, &sec);
                Localcalculation(year, month, day, hour, min, ob_blu_json.lon, ob_blu_json.lat, ob_blu_json.orientation,
                                 ob_blu_json.T1_h, ob_blu_json.T1_m, ob_blu_json.T2_h, ob_blu_json.T2_m,
                                 ob_blu_json.T3_h, ob_blu_json.T3_m, ob_blu_json.T4_h, ob_blu_json.T4_m,
                                 &M1, &M2);

                mqtt_json_s.mqtt_height = M1;
                mqtt_json_s.mqtt_angle = M2;
                printf("Motor_Height=%d,Motor_Angle=%d\n", mqtt_json_s.mqtt_height, mqtt_json_s.mqtt_angle);
                work_status = WORK_HANDTOAUTO; //自动回复切自动
                printf("hand to auto by last time1\n");
            }
            else
            {
                work_status = WORK_HANDTOAUTO; //自动回复切自动
                printf("hand to auto by last time2\n");
            }
        }

        auto_ctl_count++;
        if (auto_ctl_count >= MAX_AUTO_CTL_TIME) //超时没收到平台自动控制指令,转本地计算
        {
            auto_ctl_count = 0;

            if ((work_status == WORK_AUTO) || (work_status == WORK_WAITLOCAL)) //当前时自动状态时，切本地计算
            {

                SD25Rtc_Read(&year, &month, &day, &hour, &min, &sec);

                //if (((min % 1) == 0) && (sec == 0))
                {
                    printf("Read:%d-%d-%d %d:%d:%d\n", year, month, day, hour, min, sec);

                    Localcalculation(year, month, day, hour, min, ob_blu_json.lon, ob_blu_json.lat, ob_blu_json.orientation,
                                     ob_blu_json.T1_h, ob_blu_json.T1_m, ob_blu_json.T2_h, ob_blu_json.T2_m,
                                     ob_blu_json.T3_h, ob_blu_json.T3_m, ob_blu_json.T4_h, ob_blu_json.T4_m,
                                     &M1, &M2);
                    //printf("T1=%d:%d,T2=%d:%d,T3=%d:%d,T4=%d:%d\n",
                    //ob_blu_json.T1_h, ob_blu_json.T1_m, ob_blu_json.T2_h, ob_blu_json.T2_m,
                    //ob_blu_json.T3_h, ob_blu_json.T3_m, ob_blu_json.T4_h, ob_blu_json.T4_m);
                    mqtt_json_s.mqtt_height = M1;
                    mqtt_json_s.mqtt_angle = M2;
                    printf("Motor_Height=%d,Motor_Angle=%d\n", mqtt_json_s.mqtt_height, mqtt_json_s.mqtt_angle);

                    work_status = WORK_LOCAL;
                    Led_Status = LED_STA_LOCAL;
                    //重新连接一次WIFI
                    //WifiStatus=WIFISTATUS_DISCONNET;
                    //initialise_wifi(wifi_data.wifi_ssid,wifi_data.wifi_pwd);
                }
            }
        }
    }
}

static void Motor_Task(void *arg)
{
    while (1)
    {
        if (work_status == WORK_INIT)
        {

            if (Motor_SetAllDown() == MOTOROK)
            {
                printf("set down over init\n");
            }

            //工作状态和LED控制
            if (start_read_blue_ret == BLU_COMMAND_SWITCH) //只开关版本
            {
                work_status = WORK_AUTO;
                Led_Status = LED_STA_HAND;
            }
            else if (start_read_blue_ret == BLU_COMMAND_CALCULATION)
            {
                work_status = WORK_LOCAL;
                Led_Status = LED_STA_LOCAL;
            }
            else if (start_read_blue_ret == BLU_RESULT_SUCCESS)
            {
                work_status = WORK_AUTO;
                Led_Status = LED_STA_LOCAL;
            }
        }
        else if (work_status == WORK_FIREINIT)
        {
            Motor_SetAllUp();
            work_status = WORK_FIRE;
            Led_Status = LED_STA_FIRE;
        }
        if ((work_status != WORK_FIRE) && (work_status != WORK_PROTECT)) //不是火灾和平台保护状态下可执行
        {
            Motor_Ctl_App(); //执行墙壁开关控制
        }
        if (work_status == WORK_HANDTOAUTO) //last计时到时间，手动自动回复
        {

            Led_Status = LED_STA_LOCAL;
            strcpy(mqtt_json_s.mqtt_mode, "1");
            printf("hand to auto,auto_height=%d,auto_angle=%d\n", mqtt_json_s.mqtt_height, mqtt_json_s.mqtt_angle);
            if ((mqtt_json_s.mqtt_height == 0) && (mqtt_json_s.mqtt_angle == 0)) //目标是0，0 直接执行全收
            {
                Motor_AutoCtl((int)mqtt_json_s.mqtt_height, (int)mqtt_json_s.mqtt_angle); //转自动控制
                if (WifiStatus == WIFISTATUS_CONNET)
                {
                    http_send_mes(POST_NORMAL);
                }
            }
            else
            {

                Motor_SetAllDown();                                                       //先清零
                Motor_AutoCtl((int)mqtt_json_s.mqtt_height, (int)mqtt_json_s.mqtt_angle); //转自动控制
                if (WifiStatus == WIFISTATUS_CONNET)
                {
                    http_send_mes(POST_NORMAL);
                }
            }

            work_status = WORK_AUTO;
        }
        else if (work_status == WORK_LOCAL)
        {
            printf("Motor task work local\n");
            work_status = WORK_WAITLOCAL;
            Motor_AutoCtl((int)mqtt_json_s.mqtt_height, (int)mqtt_json_s.mqtt_angle);
        }

        if ((WallKeyCtl_Status != WallKeyUpStart) && (WallKeyCtl_Status != WallKeyDownStart)) //避免墙壁开关上升下降有停顿
        {
            vTaskDelay(10 / portTICK_RATE_MS);
        } //喂狗
    }
}

static void Wallkey_Read_Task(void *arg)
{
    while (1)
    {
        Wallkey_App(ob_blu_json.WallKeyId, ob_blu_json.Switch);
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

static void Uart0_Task(void *arg)
{
    while (1)
    {
        Uart0_read();
        vTaskDelay(10 / portTICK_RATE_MS);
        if (Ble_need_restart == 1)
        {
            vTaskDelay(3000 / portTICK_RATE_MS);
            fflush(stdout); //使stdout清空，就会立刻输出所有在缓冲区的内容。
            esp_restart();  //芯片复位 函数位于esp_system.h
        }
    }
}

/*
  EEPROM PAGE0 
    0x00 APIkey(32byte)
    0x20 chnnel_id(4byte)
    0x30 Serial_No(16byte)
    0x40 Protuct_id(32byte)
  EEPROM PAGE1
    0X00 bluesave  (256byte)
  */

void app_main(void)
{
    nvs_flash_init(); //初始化flash

    WifiStatus = WIFISTATUS_DISCONNET;
    work_status = WORK_INIT;
    Ble_need_restart = 0;
    mqtt_json_s.mqtt_height = -1;
    mqtt_json_s.mqtt_angle = -1;

    Led_Init();
    Motor_Init();
    Wind_Init();
    Wallkey_Init();
    E2prom_Init();
    Fire_Init();
    sd25rtc_init();
    Uart0_Init();

    xTaskCreate(Uart0_Task, "Uart0_Task", 4096, NULL, 10, NULL);

    //uint8_t data_write[256] = "\0";
    //E2prom_Write(0x00, data_write, 256);

    //模拟清空序列号，串口烧写
    //uint8_t data_write[16] = "\0";
    //E2prom_Write(0x30, data_write, 16);

    //模拟清空Productid，串口烧写
    //uint8_t data_write1[32] = "\0";
    //E2prom_Write(0x40, data_write1, 32);

    //模拟清空API-KEY存储，激活后获取
    //uint8_t data_write2[33]="\0";
    //E2prom_Write(0x00, data_write2, 32);

    //模拟清空channelid，激活后获取
    //uint8_t data_write3[16]="\0";
    //E2prom_Write(0x20, data_write3, 16);

    /*
    printf("new eeprom\n");
    char zero_data[512];
    bzero(zero_data,sizeof(zero_data));
    E2prom_BluWrite(0x00, (uint8_t *)zero_data, 512); //清空蓝牙
*/

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

    if (gpio_get_level(GPIO_XF) == 0) //火灾
    {
        printf("on fire!\n");
        mqtt_json_s.mqtt_fire_alarm = 1;
        work_status = WORK_FIREINIT;
    }

    strncpy(ble_dev_pwd, SerialNum + 3, 4);
    printf("ble_dev_pwd=%s\n", ble_dev_pwd);

    ble_app_start();
    init_wifi();

    //清空蓝牙配置信息
    //uint8_t zerobuf[256]="\0";
    //E2prom_BluWrite(0x00, (uint8_t *)zerobuf, 256);
    /*step2 判断是否有蓝牙配置信息****/

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

    if (start_read_blue_ret == BLU_RESULT_SUCCESS) //全功能版本
    {

        /*step3 判断是否有API-KEY和channel-id****/
        E2prom_Read(0x00, (uint8_t *)ApiKey, 32);
        printf("readApiKey=%s\n", ApiKey);
        E2prom_Read(0x20, (uint8_t *)ChannelId, 16);
        printf("readChannelId=%s\n", ChannelId);
        if ((strlen(SerialNum) == 0) || (strlen(ChannelId) == 0)) //未获取到API-KEY，和channelid进行激活流程
        {
            printf("no ApiKey or channelId!\n");

            while (http_activate() == 0) //激活失败
            {
                vTaskDelay(10000 / portTICK_RATE_MS);
            }

            //激活成功
            E2prom_Read(0x00, (uint8_t *)ApiKey, 32);
            printf("ApiKey=%s\n", ApiKey);
            E2prom_Read(0x20, (uint8_t *)ChannelId, 16);
            printf("ChannelId=%s\n", ChannelId);
        }

        /*******************************timer 1s init**********************************************/
        esp_err_t err = esp_timer_create(&timer_periodic_arg, &timer_periodic_handle);
        err = esp_timer_start_periodic(timer_periodic_handle, 1000); //创建定时器，单位us，定时1ms
        if (err != ESP_OK)
        {
            printf("timer periodic create err code:%d\n", err);
        }

        xTaskCreate(Motor_Task, "Motor_Task", 7168, NULL, 5, NULL);
        xTaskCreate(Wallkey_Read_Task, "Wallkey_Read_Task", 2048, NULL, 10, NULL);

        initialise_http();
        initialise_mqtt();
    }

    else if (start_read_blue_ret == BLU_COMMAND_SWITCH) //只开关版本
    {
        esp_wifi_stop();
        xTaskCreate(Motor_Task, "Motor_Task", 7168, NULL, 5, NULL);
        xTaskCreate(Wallkey_Read_Task, "Wallkey_Read_Task", 2048, NULL, 10, NULL);
    }

    else if (start_read_blue_ret == BLU_COMMAND_CALCULATION) //本地计算版本
    {
        esp_wifi_stop();
        /*******************************timer 1s init**********************************************/
        esp_err_t err = esp_timer_create(&timer_periodic_arg, &timer_periodic_handle);
        err = esp_timer_start_periodic(timer_periodic_handle, 1000); //创建定时器，单位us，定时1ms
        if (err != ESP_OK)
        {
            printf("timer periodic create err code:%d\n", err);
        }

        xTaskCreate(Motor_Task, "Motor_Task", 7168, NULL, 5, NULL);
        xTaskCreate(Wallkey_Read_Task, "Wallkey_Read_Task", 2048, NULL, 10, NULL);
    }
}
