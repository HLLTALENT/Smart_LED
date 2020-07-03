#ifndef __Json_parse
#define __Json_parse
#include <stdio.h>
#include "esp_err.h"

extern int32_t ret1;

esp_err_t parse_objects_http_active(char *http_json_data);
int32_t parse_objects_bluetooth(char *blu_json_data); //esp_err_t parse_objects_bluetooth(char *blu_json_data);
esp_err_t parse_objects_mqtt(char *json_data);
esp_err_t parse_objects_heart(char *json_data);
esp_err_t parse_Uart0(char *json_data);
esp_err_t parse_objects_http_respond(char *http_json_data);
esp_err_t ParseTcpUartCmd(char *pcCmdBuffer);

esp_err_t creat_object(void);

#define WORK_INIT 0X00       //初始化
#define WORK_AUTO 0x01       //平台自动模式
#define WORK_HAND 0x02       //网页版手动模式
#define WORK_HANDTOAUTO 0x03 //用于自动回复时执行一次自动控制指令
#define WORK_LOCAL 0x04      //本地计算控制模式
#define WORK_WAITLOCAL 0x05  //本地计算等待模式（用于状态机空闲状态）
#define WORK_WALLKEY 0X06    //本地墙壁开关控制模式
#define WORK_PROTECT 0X07    //风速和结霜保护
#define WORK_FIREINIT 0X08   //开机就火灾
#define WORK_FIRE 0x09       //火灾保护状态
#define LUNCHTIME 0X10       //午休

#define PROTECT_ON 0X01  //当前有风速等平台保护状态
#define PROTECT_OFF 0X00 //当前无风速等平台保护状态

#define MAX_AUTO_CTL_TIME (10 * 60)    //平台php不发送高角值，切本地计算时间10min
#define MAX_WALLKEY_TIME (8 * 60 * 60) //8h 墙壁开关控制回自动控制
// #define MAX_WALLKEY_TIME (5 * 60) //

struct
{
    float mqtt_Y;

    //int8_t mqtt_light;
    char mqtt_light_char[8];
    //int8_t mqtt_human;
    char mqtt_human_char[8];

    char mqtt_mode[8];
    char mqtt_mode_char[8];

    char mqtt_control_char[8];

    int8_t mqtt_illumination_adj; //用于手动控制解析改变角度取值-1/+1
    //int8_t mqtt_light_adj;        //用于手动控制解析改变高度取值-1/+1

    int mqtt_last;
    char mqtt_ota_url[128];

    char mqtt_stage[8];
    char mqtt_command_id[32];
    char mqtt_string[256];
    char mqtt_Rssi[8];

} mqtt_json_s;

struct
{
    char wifi_ssid[36];
    char wifi_pwd[36];
} wifi_data;

struct
{
    int http_illumination;
    int http_light;
    int http_mode;
    int http_sun_condition;
    char http_time[24];
    char blu_lng[15];
    char blu_lat[15];
} http_json_c;

typedef struct
{
    char creat_json_b[512];
    int creat_json_c;
} creat_json;

typedef struct
{
    float lon;
    float lat;
    float orientation;
    int T1_h;
    int T1_m;
    int T2_h;
    int T2_m;
    int T3_h;
    int T3_m;
    int T4_h;
    int T4_m;
    uint8_t WallKeyId[4];
    int8_t Switch;
} object_bluetooth_json;

struct
{
    float lon;
    float lat;
    float orientation;
    int T1_h;
    int T1_m;
    int T2_h;
    int T2_m;
    int T3_h;
    int T3_m;
    int T4_h;
    int T4_m;
    uint8_t WallKeyId[4];
    int8_t Switch;
} ob_blu_json;

int read_bluetooth(void);
//creat_json *create_http_json(uint8_t post_status);
void create_http_json(creat_json *pCreat_json);

enum
{
    Automatic_mode = 1,       //自动
    Manual_mode = 0,          //手动
    Storm_mode = 1,           //风速大强制收帘
    Not_Storm_mode = 0,       //阳光强，正常控制
    Frost_protection = 1,     //结霜，强制收帘
    Not_frost_protection = 0, //未结霜
};

uint8_t work_status;    //当前工作状态
uint8_t protect_status; //保护状态，用于火灾和风速混合保护的切换

int auto_ctl_count;
int auto_ctl_count1; //自动控制指令计数，收到平台的自动控制指令后该变量清零，在定时器中每1s+1，加到180S（3min）后，进入本地计算

/************metadata 参数***********/
extern unsigned long fn_dp; //数据发送频率
extern unsigned long fn_th; //温湿度频率
extern uint8_t cg_data_led; //发送数据 LED状态 0关闭，1打开
extern uint8_t net_mode;    //上网模式选择 0：自动模式 1：lan模式 2：wifi模式
/************************************/

#endif