#ifndef __H_P
#define __H_P

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "cJSON.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "Smartconfig.h"
#include "Json_parse.h"

#define FIRMWARE "ILS1-V1.0.0"

#define POST_NORMAL 0X00
#define POST_HEIGHT_ADD 0X01
#define POST_HEIGHT_SUB 0X02
#define POST_ANGLE_ADD 0X03
#define POST_ANGLE_SUB 0X04
#define POST_ALLDOWN 0X05
#define POST_ALLUP 0X06
#define POST_TARGET 0X07    //HTTP直接上传目标值，用于手动切自动的状态上传
#define POST_NOCOMMAND 0X08 //HTTP只上传平台，无commnd id

extern uint8_t need_send;

void initialise_http(void);

void http_send_mes(uint8_t post_status);
int http_activate(void);

#define HTTP_STA_SERIAL_NUMBER 0x00
#define HTTP_KEY_GET 0x01
#define HTTP_KEY_NOT_GET 0x02
/*
typedef enum HTTP_SEND_STA
{
  HTTP_STA_SERIAL_NUMBER = 0x00,
  HTTP_KEY_GET = 0x01,
  HTTP_KEY_NOT_GET = 0x02,
};*/

#endif