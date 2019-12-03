#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"

#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
/*  user include */
#include "Smartconfig.h"
#include "esp_log.h"
#include "Led.h"
#include "Bluetooth.h"
#include "Json_parse.h"

wifi_config_t s_staconf;
uint8_t wifi_status;

enum wifi_connect_sta
{
    connect_Y = 1,
    connect_N = 2,
};
uint8_t wifi_con_sta = 0;
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        //esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        /*if (work_status != WORK_INIT)
        {
            if ((work_status == WORK_WALLKEY) || (work_status == WORK_HAND))
            {
                Led_Status = LED_STA_HAND;
            }
            else
            {
                Led_Status = LED_STA_LOCAL;
            }
        }*/

        //Led_Status=LED_STA_INIT;
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        /*if (start_read_blue_ret == BLU_RESULT_SUCCESS) //在全功能版本时闪烁故障灯
        {
            Led_Status = LED_STA_WIFIERR;
        }*/
        break;
    default:
        break;
    }
    return ESP_OK;
}

void initialise_wifi(char *wifi_ssid, char *wifi_password)
{
    printf("WIFI Reconnect,SSID=%s,PWD=%s\r\n", wifi_ssid, wifi_password);

    ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &s_staconf));
    if (s_staconf.sta.ssid[0] == '\0')
    {
        //ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &s_staconf));
        strcpy((char *)s_staconf.sta.ssid, wifi_ssid);
        strcpy((char *)s_staconf.sta.password, wifi_password);

        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &s_staconf));
        ESP_ERROR_CHECK(esp_wifi_start());
        esp_wifi_connect();
    }
    else
    {
        ESP_ERROR_CHECK(esp_wifi_stop());
        memset(&s_staconf.sta, 0, sizeof(s_staconf));
        //printf("WIFI CHANGE\r\n");
        strcpy((char *)s_staconf.sta.ssid, wifi_ssid);
        strcpy((char *)s_staconf.sta.password, wifi_password);
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &s_staconf));
        ESP_ERROR_CHECK(esp_wifi_start());
        esp_wifi_connect();
    }
    /*else if (strcmp(wifi_ssid, s_staconf.sta.ssid) == 0 && strcmp(wifi_password, s_staconf.sta.password) == 0)
    {

        if (wifi_con_sta == connect_Y)
        {
            printf("ALREADY CONNECT \r\n");
        }
        else
        {
            printf("WIFI NO CHANGE\r\n");
            ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &s_staconf));
            ESP_ERROR_CHECK(esp_wifi_start());
            esp_wifi_connect();
        }
    }
    else if (strcmp(wifi_ssid, s_staconf.sta.ssid) != 0 || strcmp(wifi_password, s_staconf.sta.password) != 0)
    {
        ESP_ERROR_CHECK(esp_wifi_stop());
        memset(&s_staconf.sta, 0, sizeof(s_staconf));
        printf("WIFI CHANGE\r\n");
        strcpy(s_staconf.sta.ssid, wifi_ssid);
        strcpy(s_staconf.sta.password, wifi_password);
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &s_staconf));
        ESP_ERROR_CHECK(esp_wifi_start());
        esp_wifi_connect();
    }*/
}

void reconnect_wifi_usr(void)
{
    printf("WIFI Reconnect\r\n");
    ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &s_staconf));

    ESP_ERROR_CHECK(esp_wifi_stop());
    memset(&s_staconf.sta, 0, sizeof(s_staconf));

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &s_staconf));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_connect();
}

void init_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    memset(&s_staconf.sta, 0, sizeof(s_staconf));
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &s_staconf));
    if (s_staconf.sta.ssid[0] != '\0')
    {
        printf("wifi_init_sta finished.");
        printf("connect to ap SSID:%s password:%s\r\n",
               s_staconf.sta.ssid, s_staconf.sta.password);
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &s_staconf));
        ESP_ERROR_CHECK(esp_wifi_start());
        esp_wifi_connect();
        wifi_status = 1;

        //Led_G_On();
    }
    else
    {
        //Led_B_On();
        printf("Waiting for ble connect info ....\r\n");
    }
}