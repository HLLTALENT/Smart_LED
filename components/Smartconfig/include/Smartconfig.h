#ifndef __S_C
#define __S_C

#include "freertos/event_groups.h"

void smartconfig_example_task(void *parm);
void initialise_wifi(char *wifi_ssid, char *wifi_password);
void init_wifi(void);
void reconnect_wifi_usr(void);
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;

EventGroupHandle_t wifi_event_group;

#define WIFISTATUS_CONNET 0X01
#define WIFISTATUS_DISCONNET 0X00
uint8_t WifiStatus;
uint8_t wifi_status;
extern uint8_t Wifi_ErrCode; //7：密码错误8：未找到指定wifi

#define connect_Y 1
#define connect_N 2
#define turn_on 1
#define turn_off 2

extern uint8_t bl_flag;          //蓝牙配网模式
extern uint8_t wifi_connect_sta; //wifi连接状态

#endif