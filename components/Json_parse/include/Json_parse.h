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

#define WORK_INIT 0X00       //��ʼ��
#define WORK_AUTO 0x01       //ƽ̨�Զ�ģʽ
#define WORK_HAND 0x02       //��ҳ���ֶ�ģʽ
#define WORK_HANDTOAUTO 0x03 //�����Զ��ظ�ʱִ��һ���Զ�����ָ��
#define WORK_LOCAL 0x04      //���ؼ������ģʽ
#define WORK_WAITLOCAL 0x05  //���ؼ���ȴ�ģʽ������״̬������״̬��
#define WORK_WALLKEY 0X06    //����ǽ�ڿ��ؿ���ģʽ
#define WORK_PROTECT 0X07    //���ٺͽ�˪����
#define WORK_FIREINIT 0X08   //�����ͻ���
#define WORK_FIRE 0x09       //���ֱ���״̬
#define LUNCHTIME 0X10       //����

#define PROTECT_ON 0X01  //��ǰ�з��ٵ�ƽ̨����״̬
#define PROTECT_OFF 0X00 //��ǰ�޷��ٵ�ƽ̨����״̬

#define MAX_AUTO_CTL_TIME (10 * 60)    //ƽ̨php�����͸߽�ֵ���б��ؼ���ʱ��10min
#define MAX_WALLKEY_TIME (8 * 60 * 60) //8h ǽ�ڿ��ؿ��ƻ��Զ�����
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

    int8_t mqtt_illumination_adj; //�����ֶ����ƽ����ı�Ƕ�ȡֵ-1/+1
    //int8_t mqtt_light_adj;        //�����ֶ����ƽ����ı�߶�ȡֵ-1/+1

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
    Automatic_mode = 1,       //�Զ�
    Manual_mode = 0,          //�ֶ�
    Storm_mode = 1,           //���ٴ�ǿ������
    Not_Storm_mode = 0,       //����ǿ����������
    Frost_protection = 1,     //��˪��ǿ������
    Not_frost_protection = 0, //δ��˪
};

uint8_t work_status;    //��ǰ����״̬
uint8_t protect_status; //����״̬�����ڻ��ֺͷ��ٻ�ϱ������л�

int auto_ctl_count;
int auto_ctl_count1; //�Զ�����ָ��������յ�ƽ̨���Զ�����ָ���ñ������㣬�ڶ�ʱ����ÿ1s+1���ӵ�180S��3min���󣬽��뱾�ؼ���

/************metadata ����***********/
extern unsigned long fn_dp; //���ݷ���Ƶ��
extern unsigned long fn_th; //��ʪ��Ƶ��
extern uint8_t cg_data_led; //�������� LED״̬ 0�رգ�1��
extern uint8_t net_mode;    //����ģʽѡ�� 0���Զ�ģʽ 1��lanģʽ 2��wifiģʽ
/************************************/

#endif