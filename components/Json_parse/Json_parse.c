#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cJSON.h>
#include "esp_system.h"
#include "Json_parse.h"
#include "Nvs.h"
#include "ServerTimer.h"
#include "Http.h"

#include "esp_wifi.h"
#include "Smartconfig.h"
#include "E2prom.h"
#include "Http.h"
#include "Human.h"
#include "Pwm.h"

#include "Bluetooth.h"
#include "Led.h"
#include "ota.h"
#include "tcp_bsp.h"
#include "Localcalculation.h"

extern uint8_t human_status;
int32_t ret1;
char buf[512];

//metadata 参数
unsigned long fn_dp = 60; //数据发送频率 默认60s
unsigned long fn_th = 0;  //温湿度频率
uint8_t cg_data_led = 1;  //发送数据 LED状态 0：不闪烁 1：闪烁
uint8_t net_mode = 0;     //上网模式选择 0：自动模式 1：lan模式 2：wifi模式

typedef enum
{
    HTTP_JSON_FORMAT_TINGERROR,
    HTTP_RESULT_UNSUCCESS,
    HTTP_OVER_PARSE,
    HTTP_WRITE_FLASH_OVER,
    CREAT_OBJECT_OVER,
} http_error_info;

extern uint32_t HTTP_STATUS;
char get_num[5];
char *key;
int n = 0, i;
int a, b = 0;

//解析metadata
static short Parse_metadata(char *ptrptr)
{
    // bool fn_flag = 0;
    if (NULL == ptrptr)
    {
        return 0;
    }

    cJSON *pJsonJson = cJSON_Parse(ptrptr);
    if (NULL == pJsonJson)
    {
        cJSON_Delete(pJsonJson); //delete pJson

        return 0;
    }

    cJSON *pSubSubSub = cJSON_GetObjectItem(pJsonJson, "fn_th"); //"fn_th"
    if (NULL != pSubSubSub)
    {

        if ((unsigned long)pSubSubSub->valueint != fn_th)
        {
            fn_th = (unsigned long)pSubSubSub->valueint;
            printf("fn_th = %ld\n", fn_th);
        }
    }

    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "fn_dp"); //"fn_dp"
    if (NULL != pSubSubSub)
    {
        if ((unsigned long)pSubSubSub->valueint != fn_dp)
        {
            // fn_flag = 1;
            fn_dp = (unsigned long)pSubSubSub->valueint;
            printf("fn_dp = %ld\n", fn_dp);
        }
    }

    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "cg_data_led"); //"cg_data_led"
    if (NULL != pSubSubSub)
    {

        if ((uint8_t)pSubSubSub->valueint != cg_data_led)
        {
            cg_data_led = (uint8_t)pSubSubSub->valueint;
            printf("cg_data_led = %d\n", cg_data_led);
            /*if (cg_data_led == 0)
            {
                Turn_Off_LED();
            }
            else
            {
                Turn_ON_LED();
            }*/
        }
    }

    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "net_mode"); //"net_mode"
    if (NULL != pSubSubSub)
    {

        if ((uint8_t)pSubSubSub->valueint != net_mode)
        {
            net_mode = (uint8_t)pSubSubSub->valueint;
            printf("net_mode = %d\n", net_mode);
        }
    }

    cJSON_Delete(pJsonJson);
    return 1;
}

int read_bluetooth(void)
{
    uint8_t bluetooth_sta[512];

    E2prom_BluRead(bluetooth_sta);
    printf("bluetooth_sta=%s\n", bluetooth_sta);
    if (strlen((char *)bluetooth_sta) == 0) //未读到蓝牙配置信息
    {
        return 0;
    }
    ret1 = parse_objects_bluetooth((char *)bluetooth_sta);           //uint8_t ret = parse_objects_bluetooth((char *)bluetooth_sta);
    if ((ret1 == BLU_PWD_REFUSE) || (ret1 == BLU_JSON_FORMAT_ERROR)) //if (ret == BLU_JSON_FORMAT_ERROR)
    {
        return 0;
    }
    else
    {

        return ret1;
    }
    //return 1;
    /*else
    {
        return ret;
    }*/
}

int32_t parse_objects_bluetooth(char *blu_json_data)
{
    cJSON *cjson_blu_data_parse = NULL;
    cJSON *cjson_blu_data_parse_command = NULL;
    // cJSON *cjson_blu_data_parse_wifissid = NULL;
    // cJSON *cjson_blu_data_parse_wifipwd = NULL;
    // cJSON *cjson_blu_data_parse_ob = NULL;
    //cJSON *cjson_blu_data_parse_devicepwd = NULL;

    printf("start_ble_parse_json\r\n");
    if (blu_json_data[0] != '{')
    {
        printf("blu_json_data Json Formatting error\n");
        return 0;
    }

    cjson_blu_data_parse = cJSON_Parse(blu_json_data);
    if (cjson_blu_data_parse == NULL) //如果数据包不为JSON则退出
    {
        printf("Json Formatting error2\n");
        cJSON_Delete(cjson_blu_data_parse);
        return BLU_JSON_FORMAT_ERROR;
    }
    else
    {
        cjson_blu_data_parse_command = cJSON_GetObjectItem(cjson_blu_data_parse, "command");
        printf("command=%s\r\n", cjson_blu_data_parse_command->valuestring);

        ParseTcpUartCmd(cJSON_Print(cjson_blu_data_parse));
    }
    //cJSON_Delete(cjson_blu_data_parse);

    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, 10000 / portTICK_RATE_MS); //10S后仍然未连接上WIFI
    if (wifi_connect_sta == connect_Y)
    {
        //need_reactivate = 1;
        //xEventGroupClearBits(wifi_event_group, ACTIVED_BIT);
        //while (http_activate() < 0) //激活
        //{
        // ESP_LOGE(TAG, "activate fail\n");
        // vTaskDelay(2000 / portTICK_PERIOD_MS);
        //}
        //xEventGroupSetBits(wifi_event_group, ACTIVED_BIT);
        return BLU_RESULT_SUCCESS; //return http_activate();
    }
    else if ((wifi_connect_sta == connect_N) || (a == 0))
    {
        return BLU_COMMAND_CALCULATION;
    }
    else
    {
        return BLU_WIFI_ERR;
    }
    cJSON_Delete(cjson_blu_data_parse);
}

//解析激活返回数据
esp_err_t parse_objects_http_active(char *http_json_data)
{
    cJSON *json_data_parse = NULL;
    cJSON *json_data_parse_value = NULL;
    cJSON *json_data_parse_time_value = NULL;
    cJSON *json_data_parse_channel_channel_write_key = NULL;
    cJSON *json_data_parse_channel_channel_id_value = NULL;
    cJSON *json_data_parse_channel_metadata = NULL;
    //cJSON *json_data_parse_command_value = NULL;
    cJSON *json_data_parse_channel_value = NULL;
    //char *json_print;

    printf("start_parse_active_http_json\r\n");
    //printf("http_json_data[0]= %d\r\n",http_json_data[0]);

    if (http_json_data[0] != '{')
    {
        printf("http_json_data Json Formatting error\n");

        return 0;
    }

    json_data_parse = cJSON_Parse(http_json_data);
    if (json_data_parse == NULL)
    {

        printf("Json Formatting error3\n");

        cJSON_Delete(json_data_parse);
        return 0;
    }
    else
    {

        json_data_parse_value = cJSON_GetObjectItem(json_data_parse, "result");
        if (!(strcmp(json_data_parse_value->valuestring, "success")))
        {
            printf("active:success\r\n");

            json_data_parse_time_value = cJSON_GetObjectItem(json_data_parse, "server_time");
            Server_Timer_GET(json_data_parse_time_value->valuestring);
        }
        else
        {
            printf("active:error\r\n");
            cJSON_Delete(json_data_parse);
            return 0;
        }

        if (cJSON_GetObjectItem(json_data_parse, "channel") != NULL)
        {
            json_data_parse_channel_value = cJSON_GetObjectItem(json_data_parse, "channel");

            //printf("%s\r\n", cJSON_Print(json_data_parse_channel_value));

            json_data_parse_channel_channel_write_key = cJSON_GetObjectItem(json_data_parse_channel_value, "write_key");
            json_data_parse_channel_channel_id_value = cJSON_GetObjectItem(json_data_parse_channel_value, "channel_id");
            json_data_parse_channel_metadata = cJSON_GetObjectItem(json_data_parse_channel_value, "metadata");

            Parse_metadata(json_data_parse_channel_metadata->valuestring);
            //printf("api key=%s\r\n", json_data_parse_channel_channel_write_key->valuestring);
            //printf("channel_id=%s\r\n", json_data_parse_channel_channel_id_value->valuestring);

            //写入API-KEY
            sprintf(ApiKey, "%s%c", json_data_parse_channel_channel_write_key->valuestring, '\0');
            printf("api key=%s\r\n", ApiKey);
            E2prom_Write(0x00, (uint8_t *)ApiKey, 32);

            //写入channelid
            sprintf(ChannelId, "%s%c", json_data_parse_channel_channel_id_value->valuestring, '\0');
            printf("channel_id=%s\r\n", ChannelId);
            E2prom_Write(0x20, (uint8_t *)ChannelId, strlen(ChannelId));
        }
    }
    cJSON_Delete(json_data_parse);
    return 1;
}

//解析http-post返回数据
esp_err_t parse_objects_http_respond(char *http_json_data)
{
    cJSON *json_data_parse = NULL;
    cJSON *json_data_parse_value = NULL;
    cJSON *json_data_parse_errorcode = NULL;

    if (http_json_data[0] != '{')
    {
        printf("http_respond_json_data Json Formatting error\n");
        return 0;
    }

    json_data_parse = cJSON_Parse(http_json_data);
    if (json_data_parse == NULL)
    {

        printf("Json Formatting error3\n");
        cJSON_Delete(json_data_parse);
        return 0;
    }
    else
    {
        json_data_parse_value = cJSON_GetObjectItem(json_data_parse, "result");
        // printf("result: %s\n", json_data_parse_value->valuestring);
        if (!(strcmp(json_data_parse_value->valuestring, "error")))
        {
            json_data_parse_errorcode = cJSON_GetObjectItem(json_data_parse, "errorCode");
            printf("post send error_code=%s\n", json_data_parse_errorcode->valuestring);
            if (!(strcmp(json_data_parse_errorcode->valuestring, "invalid_channel_id"))) //设备空间ID被删除或API——KEY错误，需要重新激活
            {
                //清空API-KEY存储，激活后获取
                uint8_t data_write2[33] = "\0";
                E2prom_Write(0x00, data_write2, 32);

                //清空channelid，激活后获取
                uint8_t data_write3[16] = "\0";

                E2prom_Write(0x20, data_write3, 16);

                fflush(stdout); //使stdout清空，就会立刻输出所有在缓冲区的内容。
                esp_restart();  //芯片复位 函数位于esp_system.h
            }
        }

        json_data_parse_value = cJSON_GetObjectItem(json_data_parse, "metadata");
        if (json_data_parse_value != NULL)
        {
            // printf("metadata: %s\n", json_data_parse_value->valuestring);
            Parse_metadata(json_data_parse_value->valuestring);
        }
    }
    cJSON_Delete(json_data_parse);
    return 1;
}

/*esp_err_t parse_Uart0(char *json_data)
{
    cJSON *json_data_parse = NULL;
    cJSON *json_data_parse_ProductID = NULL;
    cJSON *json_data_parse_SeriesNumber = NULL;
    //if(strstr(json_data,"{")==NULL)
    if (json_data[0] != '{')
    {
        printf("uart0 Json Formatting error1\n");
        return 0;
    }
    json_data_parse = cJSON_Parse(json_data);
    if (json_data_parse == NULL) //如果数据包不为JSON则退出
    {
        printf("uart0 Json Formatting error\n");
        cJSON_Delete(json_data_parse);
        return 0;
    }
    else
    {
        json_data_parse_ProductID = cJSON_GetObjectItem(json_data_parse, "ProductID");
        printf("ProductID= %s\n", json_data_parse_ProductID->valuestring);
        json_data_parse_SeriesNumber = cJSON_GetObjectItem(json_data_parse, "SeriesNumber");
        printf("SeriesNumber= %s\n", json_data_parse_SeriesNumber->valuestring);
        sprintf(ProductId, "%s%c", json_data_parse_ProductID->valuestring, '\0');
        E2prom_Write(0x40, (uint8_t *)ProductId, strlen(ProductId));
        sprintf(SerialNum, "%s%c", json_data_parse_SeriesNumber->valuestring, '\0');
        E2prom_Write(0x30, (uint8_t *)SerialNum, strlen(SerialNum));
        //清空API-KEY存储，激活后获取
        uint8_t data_write2[33] = "\0";
        E2prom_Write(0x00, data_write2, 32);
        //清空channelid，激活后获取
        uint8_t data_write3[16] = "\0";
        E2prom_Write(0x20, data_write3, 16);
        uint8_t zerobuf[512] = "\0";
        E2prom_BluWrite(0x00, (uint8_t *)zerobuf, 512); //清空蓝牙
        //E2prom_Read(0x30,(uint8_t *)SerialNum,16);
        //printf("read SerialNum=%s\n", SerialNum);
        //E2prom_Read(0x40,(uint8_t *)ProductId,32);
        //printf("read ProductId=%s\n", ProductId);
        printf("{\"status\":0,\"code\": 0}");
        cJSON_Delete(json_data_parse);
        //fflush(stdout); //使stdout清空，就会立刻输出所有在缓冲区的内容。
        //esp_restart();  //芯片复位 函数位于esp_system.h
        return 1;
    }
}
*/
esp_err_t parse_objects_heart(char *json_data)
{
    cJSON *json_data_parse = NULL;
    cJSON *json_data_parse_value = NULL;
    char *json_print;
    json_data_parse = cJSON_Parse(json_data);

    if (json_data[0] != '{')
    {
        printf("heart Json Formatting error\n");

        return 0;
    }

    if (json_data_parse == NULL) //如果数据包不为JSON则退出
    {

        printf("Json Formatting error4\n");

        cJSON_Delete(json_data_parse);
        return 0;
    }
    else
    {
        json_data_parse_value = cJSON_GetObjectItem(json_data_parse, "server_time");
        Server_Timer_GET(json_data_parse_value->valuestring);
        json_print = cJSON_Print(json_data_parse_value);
        printf("json_data_parse_value %s\n", json_print);
    }
    free(json_print);
    cJSON_Delete(json_data_parse);

    return 1;
}

esp_err_t parse_objects_mqtt(char *mqtt_json_data)
{
    cJSON *json_data_parse = NULL;

    cJSON *json_data_mode_parse = NULL;

    cJSON *json_data_string_parse = NULL;
    cJSON *json_data_command_id_parse = NULL;
    //cJSON *json_data_control_parse = NULL;

    cJSON *json_data_stage_parse = NULL;

    cJSON *json_data_action = NULL;
    cJSON *json_data_url = NULL;
    cJSON *json_data_vesion = NULL;
    //cJSON *json_data_set_state = NULL;

    json_data_parse = cJSON_Parse(mqtt_json_data);
    //printf("%s", cJSON_Print(json_data_parse));

    if (mqtt_json_data[0] != '{')
    {
        printf("mqtt_json_data Json Formatting error\n");

        return 0;
    }

    if (json_data_parse == NULL) //如果数据包不为JSON则退出
    {

        printf("Json Formatting error5\n");

        cJSON_Delete(json_data_parse);
        return 0;
    }
    /* if ((human_status == 1) && (strcmp(json_data_action->valuestring, "command") == 0))//有人则退出
    {
            printf("Json Formatting error6\n");
            cJSON_Delete(json_data_parse);
            return 0;
    }*/

    json_data_string_parse = cJSON_GetObjectItem(json_data_parse, "command_string");
    if (json_data_string_parse != NULL)
    {
        json_data_command_id_parse = cJSON_GetObjectItem(json_data_parse, "command_id");
        strncpy(mqtt_json_s.mqtt_command_id, json_data_command_id_parse->valuestring, strlen(json_data_command_id_parse->valuestring));
        strncpy(mqtt_json_s.mqtt_string, json_data_string_parse->valuestring, strlen(json_data_string_parse->valuestring));
        //need_send = 1;
        post_status = POST_NORMAL;
        //printf("%s\r\n", cJSON_Print(json_data_string_parse));
        json_data_string_parse = cJSON_Parse(json_data_string_parse->valuestring);
        if (json_data_string_parse != NULL)
        {
            //printf("%s\r\n", cJSON_Print(json_data_string_parse));
            if ((json_data_action = cJSON_GetObjectItem(json_data_string_parse, "action")) != NULL)
            {
                //如果命令是OTA
                if (strcmp(json_data_action->valuestring, "ota") == 0)
                {
                    printf("OTA命令进入\r\n");
                    if ((json_data_vesion = cJSON_GetObjectItem(json_data_string_parse, "version")) != NULL &&
                        (json_data_url = cJSON_GetObjectItem(json_data_string_parse, "url")) != NULL)
                    {
                        if (strcmp(json_data_vesion->valuestring, FIRMWARE) != 0) //与当前 版本号 对比
                        {
                            strcpy(mqtt_json_s.mqtt_ota_url, json_data_url->valuestring);
                            // E2prom_page_Write(ota_url_add, (uint8_t *)mqtt_json_s.mqtt_ota_url, 128);
                            printf("OTA_URL=%s\r\n OTA_VERSION=%s\r\n", mqtt_json_s.mqtt_ota_url, json_data_vesion->valuestring);
                            ota_start(); //启动OTA
                        }
                        else
                        {
                            printf("当前版本无需升级 \r\n");
                        }
                    }
                }
                else if (strcmp(json_data_action->valuestring, "command") == 0)
                {
                    if ((human_status == 1) || (work_status == LUNCHTIME)) //有人则退出
                    {

                        printf("Json Formatting error6\n");

                        cJSON_Delete(json_data_parse);
                        return 0;
                    }
                    else
                    {
                        //human_status = 0;
                        //auto_ctl_count = 0;
                        strcpy(mqtt_json_s.mqtt_control_char, "command");
                        work_status = WORK_HAND;
                        strcpy(mqtt_json_s.mqtt_mode, "0");

                        if ((json_data_mode_parse = cJSON_GetObjectItem(json_data_string_parse, "s_port")) != NULL)
                        {
                            if (strcmp(json_data_mode_parse->valuestring, "port1") == 0) //if (strncmp("port1", json_data_mode_parse->valuestring, strlen("port1")) == 0) //上灯条
                            {
                                strcpy(mqtt_json_s.mqtt_mode_char, "port1");
                                printf("port1\r\n");
                                if ((json_data_stage_parse = cJSON_GetObjectItem(json_data_string_parse, "set_state1")) != NULL)
                                {
                                    if (strncmp("100", json_data_stage_parse->valuestring, strlen("100")) == 0)
                                    {

                                        strcpy(mqtt_json_s.mqtt_stage, "100");
                                        Up_Light_Status = 0;
                                        Led_UP_W(100, 10);
                                        Led_UP_Y(100, 10);
                                        printf("100\r\n");
                                    }
                                    if (strncmp("0", json_data_stage_parse->valuestring, strlen("0")) == 0)
                                    {

                                        strcpy(mqtt_json_s.mqtt_stage, "0");
                                        Up_Light_Status = 1;
                                        Down_Light_Status = 0;
                                        temp_hour = -1;
                                        printf("0\r\n");
                                    }
                                }
                            }
                            if (strcmp(json_data_mode_parse->valuestring, "port0") == 0) //(strncmp("port2", json_data_mode_parse->valuestring, strlen("port2")) == 0) //下灯条
                            {
                                strcpy(mqtt_json_s.mqtt_mode_char, "port0");
                                printf("port0\r\n");
                                if ((json_data_stage_parse = cJSON_GetObjectItem(json_data_string_parse, "set_state0")) != NULL)
                                {
                                    if (strncmp("100", json_data_stage_parse->valuestring, strlen("100")) == 0)
                                    {

                                        strcpy(mqtt_json_s.mqtt_stage, "100");
                                        Down_Light_Status = 0;
                                        Led_DOWN_W(100, 10);
                                        Led_DOWN_Y(100, 10);
                                        printf("100\r\n");
                                    }
                                    if (strncmp("0", json_data_stage_parse->valuestring, strlen("0")) == 0)
                                    {

                                        strcpy(mqtt_json_s.mqtt_stage, "0");
                                        Down_Light_Status = 1;
                                        Up_Light_Status = 0;
                                        temp_hour = -1;
                                        printf("0\r\n");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        printf("Json Formatting error6\n");
        cJSON_Delete(json_data_parse);
        cJSON_Delete(json_data_string_parse);
        return 0;
    }
    cJSON_Delete(json_data_parse);
    cJSON_Delete(json_data_string_parse);

    return 1;
}
void create_http_json(creat_json *pCreat_json)
{
    printf("INTO CREATE_HTTP_JSON\r\n");
    //creat_json *pCreat_json = malloc(sizeof(creat_json));
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateObject();
    cJSON *next = cJSON_CreateObject();
    cJSON *fe_body = cJSON_CreateArray();
    //char status_creat_json_c[256];
    char status_creat_json[512];
    char status_cj[4];
    /////////float Wind_Read_t = -1;
    ///////////char wind_Read_c[16];

    //////////Wind_Read_t = Wind_Read();
    //////////sprintf(wind_Read_c, "%f", Wind_Read_t);
    //printf("Server_Timer_SEND() %s", (char *)Server_Timer_SEND());
    //strncpy(http_json_c.http_time[20], Server_Timer_SEND(), 20);
    // printf("this http_json_c.http_time[20]  %s\r\n", http_json_c.http_time);
    strncpy(http_json_c.http_time, Server_Timer_SEND(), 24);
    wifi_ap_record_t wifidata;
    if (esp_wifi_sta_get_ap_info(&wifidata) == 0)
    {
        itoa(wifidata.rssi, mqtt_json_s.mqtt_Rssi, 10);
    }

    //itoa(mqtt_json_s.mqtt_fire_alarm, mqtt_json_s.mqtt_fire_alarm_char, 10);

    // sprintf(status_creat_json, "%s%s%s%s%s%s%s%s%s%s%s", "{\"control\":", mqtt_height1 + ",", "\"angle\":", mqtt_angle1 + ",", "\"auto\":0,", "\"wind_protection\":", ",", "\"sun_condition\":", ",", "\"frost_protection\":", "}");
    sprintf(status_cj, "%s", "\"");

    sprintf(status_creat_json, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", "{",
            status_cj, "action", status_cj, ":", mqtt_json_s.mqtt_control_char, ",",
            status_cj, "s_port", status_cj, ":", mqtt_json_s.mqtt_mode_char, ",",
            status_cj, "set_state", status_cj, ":", mqtt_json_s.mqtt_stage, "}");

    sprintf(http_json_c.blu_lng, "%f", ob_blu_json.lon);
    sprintf(http_json_c.blu_lat, "%f", ob_blu_json.lat);

    {
        //printf("status_creat_json %s\r\n", status_creat_json);
        cJSON_AddItemToObject(root, "feeds", fe_body);
        cJSON_AddItemToArray(fe_body, item);
        cJSON_AddItemToObject(item, "created_at", cJSON_CreateString(http_json_c.http_time));
        cJSON_AddItemToObject(item, "status", cJSON_CreateString(status_creat_json));
        cJSON_AddItemToArray(fe_body, next);
        cJSON_AddItemToObject(next, "created_at", cJSON_CreateString(http_json_c.http_time));
        cJSON_AddItemToObject(next, "field1", cJSON_CreateString(mqtt_json_s.mqtt_light_char)); //  灯实际亮灭
        cJSON_AddItemToObject(next, "field2", cJSON_CreateNumber(mqtt_json_s.mqtt_Y));          //   灯照度
        if (human_status == NOHUMAN)
        {
            cJSON_AddItemToObject(next, "field3", cJSON_CreateString("0"));
            strcpy(mqtt_json_s.mqtt_light_char, "0");
            mqtt_json_s.mqtt_Y = Y1;
        }
        else if (human_status == HAVEHUMAN)
        {
            cJSON_AddItemToObject(next, "field3", cJSON_CreateString("1"));
            mqtt_json_s.mqtt_Y = Y;
        }                                                                                 //  人感值
        cJSON_AddItemToObject(next, "field4", cJSON_CreateString(mqtt_json_s.mqtt_mode)); //模式   自动/手动
    }

    char *cjson_printunformat;
    cjson_printunformat = cJSON_PrintUnformatted(root);
    //pCreat_json->creat_json_b = cJSON_PrintUnformatted(root);
    //pCreat_json->creat_json_c = strlen(cJSON_PrintUnformatted(root));

    pCreat_json->creat_json_c = strlen(cjson_printunformat);
    //pCreat_json->creat_json_b=cjson_printunformat;
    //pCreat_json->creat_json_b=malloc(pCreat_json->creat_json_c);
    bzero(pCreat_json->creat_json_b, sizeof(pCreat_json->creat_json_b));
    memcpy(pCreat_json->creat_json_b, cjson_printunformat, pCreat_json->creat_json_c);
    //printf("http_json=%s\n",pCreat_json->creat_json_b);
    free(cjson_printunformat);
    cJSON_Delete(root);
    //return pCreat_json;
}
esp_err_t ParseTcpUartCmd(char *pcCmdBuffer)
{
    cJSON *cjson_blu_data_parse_wifissid = NULL;
    cJSON *cjson_blu_data_parse_wifipwd = NULL;
    esp_err_t blu_ret = BLU_PWD_REFUSE;
    char send_buf[128] = {0};
    sprintf(send_buf, "{\"status\":0,\"code\": 0}");
    if (NULL == pcCmdBuffer) //null
    {
        return ESP_FAIL;
    }

    cJSON *pJson = cJSON_Parse(pcCmdBuffer); //parse json data
    if (NULL == pJson)
    {
        cJSON_Delete(pJson); //delete pJson

        return ESP_FAIL;
    }

    cJSON *pSub = cJSON_GetObjectItem(pJson, "Command"); //"Command"
    if (NULL != pSub)
    {
        if (!strcmp((char const *)pSub->valuestring, "SetupProduct")) //Command:SetupProduct
        {
            pSub = cJSON_GetObjectItem(pJson, "Password"); //"Password"
            if (NULL != pSub)
            {

                if (!strcmp((char const *)pSub->valuestring, "CloudForce"))
                {
                    //       E2prom_Write(PRODUCTURI_FLAG_ADDR, PRODUCT_URI, strlen(PRODUCT_URI), 1); //save product-uri flag

                    pSub = cJSON_GetObjectItem(pJson, "ProductID"); //"ProductID"
                    if (NULL != pSub)
                    {
                        printf("ProductID= %s\n", pSub->valuestring);
                        E2prom_Write(0x40, (uint8_t *)pSub->valuestring, strlen(pSub->valuestring)); //save ProductID
                    }

                    pSub = cJSON_GetObjectItem(pJson, "SeriesNumber"); //"SeriesNumber"
                    if (NULL != pSub)
                    {
                        printf("SeriesNumber= %s\n", pSub->valuestring);
                        E2prom_Write(0x30, (uint8_t *)pSub->valuestring, strlen(pSub->valuestring)); //save SeriesNumber
                    }

                    pSub = cJSON_GetObjectItem(pJson, "Host"); //"Host"
                    if (NULL != pSub)
                    {

                        //E2prom_Write(HOST_ADDR, (uint8_t *)pSub->valuestring, strlen(pSub->valuestring), 1); //save host in at24c08
                    }

                    pSub = cJSON_GetObjectItem(pJson, "apn"); //"apn"
                    if (NULL != pSub)
                    {

                        //E2prom_Write(APN_ADDR, (uint8_t *)pSub->valuestring, strlen(pSub->valuestring), 1); //save apn
                    }

                    pSub = cJSON_GetObjectItem(pJson, "user"); //"user"
                    if (NULL != pSub)
                    {

                        //E2prom_Write(BEARER_USER_ADDR, (uint8_t *)pSub->valuestring, strlen(pSub->valuestring), 1); //save user
                    }

                    pSub = cJSON_GetObjectItem(pJson, "pwd"); //"pwd"
                    if (NULL != pSub)
                    {

                        // E2prom_Write(BEARER_PWD_ADDR, (uint8_t *)pSub->valuestring, strlen(pSub->valuestring), 1); //save pwd
                    }

                    //清空API-KEY存储，激活后获取
                    uint8_t data_write2[33] = "\0";
                    E2prom_Write(0x00, data_write2, 32);

                    //清空channelid，激活后获取
                    uint8_t data_write3[16] = "\0";
                    E2prom_Write(0x20, data_write3, 16);

                    uint8_t zerobuf[256] = "\0";
                    E2prom_BluWrite(0x00, (uint8_t *)zerobuf, 256); //清空蓝牙
                    printf("SetupProduct Successed !");
                    printf("{\"status\":0,\"code\": 0}");

                    if (start_AP == 1)
                    {
                        printf("%s\n", send_buf);
                        tcp_send_buff(send_buf, sizeof(send_buf));
                    }
                    vTaskDelay(3000 / portTICK_RATE_MS);
                    cJSON_Delete(pJson);
                    fflush(stdout); //使stdout清空，就会立刻输出所有在缓冲区的内容。
                    esp_restart();  //芯片复位 函数位于esp_system.h

                    return ESP_OK;
                }
            }
        }

        else
        {
            cjson_blu_data_parse_wifissid = cJSON_GetObjectItem(pJson, "wifiSSID");
            if (NULL == cjson_blu_data_parse_wifissid)
            {
                return BLU_NO_WIFI_SSID;
            }
            else if (NULL != cjson_blu_data_parse_wifissid)
            {
                if (cjson_blu_data_parse_wifissid->valuestring == (void *)0)
                {
                    //return BLU_NO_WIFI_SSID;
                    printf("BLU_NO_WIFI_SSID2\r\n");
                    a = 0;
                }
                else
                {
                    bzero(wifi_data.wifi_ssid, sizeof(wifi_data.wifi_ssid));
                    strcpy(wifi_data.wifi_ssid, cjson_blu_data_parse_wifissid->valuestring);
                    printf("wifi ssid=%s\r\n", cjson_blu_data_parse_wifissid->valuestring);
                    a = 1;
                    //}
                }
            }
            cJSON *cjson_blu_data_parse_wifipwd = cJSON_GetObjectItem(pJson, "wifiPwd");
            if (NULL == cjson_blu_data_parse_wifipwd)
            {
                return BLU_NO_WIFI_PWD;
            }
            else if (NULL != cjson_blu_data_parse_wifipwd)
            {
                if (cjson_blu_data_parse_wifipwd->valuestring == (void *)0)
                {
                    printf("BLU_NO_WIFI_PWD2\r\n");
                }
                else
                {
                    bzero(wifi_data.wifi_pwd, sizeof(wifi_data.wifi_pwd));
                    strcpy(wifi_data.wifi_pwd, cjson_blu_data_parse_wifipwd->valuestring);
                    printf("wifi pwd=%s\r\n", cjson_blu_data_parse_wifipwd->valuestring);
                    b = 1;
                    //}
                }
            }
            pSub = cJSON_GetObjectItem(pJson, "s2");
            if (NULL == pSub)
            {
                return BLU_NO_S2;
            }
            else if (NULL != pSub)
            {
                if (pSub->valuestring == (void *)0)
                {
                    printf("BLU_NO_S2\r\n");
                }
                else
                {
                    printf("s2 %s\r\n", pSub->valuestring);
                    sscanf(pSub->valuestring, "%2s", get_num);

                    ob_blu_json.T2_h = atoi(get_num);
                    ob_blu_json.T2_m = atoi((pSub->valuestring) + 3);
                    printf("T2 %d %d \r\n", ob_blu_json.T2_h, ob_blu_json.T2_m);
                }
            }
            pSub = cJSON_GetObjectItem(pJson, "e2");
            if (NULL == pSub)
            {
                return BLU_NO_E2;
            }
            else if (NULL != pSub)
            {
                if (pSub->valuestring == (void *)0)
                {
                    printf("BLU_NO_E2\r\n");
                }
                else
                {
                    printf("e2 %s\r\n", pSub->valuestring);
                    sscanf(pSub->valuestring, "%2s", get_num);

                    ob_blu_json.T3_h = atoi(get_num);
                    ob_blu_json.T3_m = atoi((pSub->valuestring) + 3);
                    printf("T3 %d %d \r\n", ob_blu_json.T3_h, ob_blu_json.T3_m);
                }
            }

            //else if (!strcmp((char const *)pSub->valuestring, "SetupConfig")) //if (strcmp(cjson_blu_data_parse_command->valuestring, "SetupConfig") == 0) //收到全部信息指令
            //{
            pSub = cJSON_GetObjectItem(pJson, "serial");
            if (NULL != pSub)
            {
                printf("serial %s\r\n", pSub->valuestring);
                n = strlen(pSub->valuestring) / 2;
                key = (char *)malloc(sizeof(char) * n);
                for (i = 0; i < n; ++i)
                {
                    sscanf(pSub->valuestring + 2 * i, "%2hhX", key + i);
                }
                for (i = 0; i < n; ++i)
                {
                    ob_blu_json.WallKeyId[i] = key[i];
                    printf("0x%02X ", key[i]);
                }
                free(key);
                printf("\r\n");
            }
            pSub = cJSON_GetObjectItem(pJson, "switch");
            if (NULL != pSub)
            {
                printf("switch %d\r\n", pSub->valueint);
                ob_blu_json.Switch = pSub->valueint;
            }
            //initialise_wifi(cjson_blu_data_parse_wifissid->valuestring, cjson_blu_data_parse_wifipwd->valuestring); //重新初始化WIFI
            //}
            if ((a == 1) && (b == 1)) //有网络
            {
                initialise_wifi(cjson_blu_data_parse_wifissid->valuestring, cjson_blu_data_parse_wifipwd->valuestring); //initialise_wifi(); //重新初始化WIFI
                blu_ret = BLU_RESULT_SUCCESS;
                start_read_blue_mode = BLU_RESULT_SUCCESS;
                printf("BLU_RESULT_SUCCESS1\r\n");
            }
            else if (a == 0)
            {
                blu_ret = BLU_RESULT_SUCCESS;
                start_read_blue_mode = BLU_COMMAND_CALCULATION;
                printf("BLU_COMMAND_CALCULATION\r\n");
            }

            // cJSON_Delete(pJson);
            // return blu_ret;
        }
    }
    cJSON_Delete(pJson); //delete pJson

    return blu_ret;
}
/*int32_t parse_objects_bluetooth(char *blu_json_data) //esp_err_t parse_objects_bluetooth(char *blu_json_data)
{
    cJSON *cjson_blu_data_parse = NULL;
    cJSON *cjson_blu_data_parse_command = NULL;
    cJSON *cjson_blu_data_parse_wifissid = NULL;
    cJSON *cjson_blu_data_parse_wifipwd = NULL;
    cJSON *cjson_blu_data_parse_ob = NULL;
    cJSON *cjson_blu_data_parse_devicepwd = NULL;
    esp_err_t blu_ret = BLU_PWD_REFUSE;
    printf("start_ble_parse_json\r\n");
    if (blu_json_data[0] != '{')
    {
        printf("blu_json_data Json Formatting error\n");
        return 0;
    }
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, 10000 / portTICK_RATE_MS); //10S后仍然未连接上WIFI
    if (wifi_connect_sta == connect_Y)
    {
        need_reactivate = 1;
        return http_activate();
    }
    else
    {
        return BLU_WIFI_ERR;
    }
    cjson_blu_data_parse = cJSON_Parse(blu_json_data);
    if (cjson_blu_data_parse == NULL) //如果数据包不为JSON则退出
    {
        printf("Json Formatting error2\n");
        cJSON_Delete(cjson_blu_data_parse);
        return BLU_JSON_FORMAT_ERROR;
    }
    else
    {
        cjson_blu_data_parse_devicepwd = cJSON_GetObjectItem(cjson_blu_data_parse, "devicePassword");
        //if(cjson_blu_data_parse_ob->valuedouble==0)
        if (cjson_blu_data_parse_devicepwd == NULL)
        {
            printf("ble devpwd err1=%s\r\n", cjson_blu_data_parse_devicepwd->valuestring);
            cJSON_Delete(cjson_blu_data_parse);
            return BLU_PWD_REFUSE;
        }
        else if (strcmp(cjson_blu_data_parse_devicepwd->valuestring, ble_dev_pwd) != 0) //校验密码,错误，返回
        {
            printf("ble devpwd err2=%s\r\n", cjson_blu_data_parse_devicepwd->valuestring);
            cJSON_Delete(cjson_blu_data_parse);
            return BLU_PWD_REFUSE;
        }
        else
        {
            printf("ble devpwd ok=%s\r\n", cjson_blu_data_parse_devicepwd->valuestring);
        }
        cjson_blu_data_parse_wifissid = cJSON_GetObjectItem(cjson_blu_data_parse, "wifiSSID");
        if (cjson_blu_data_parse_wifissid == NULL)
        {
            return BLU_NO_WIFI_SSID;
        }
        else
        {
            if (cjson_blu_data_parse_wifissid->valuestring == (void *)0)
            {
                //return BLU_NO_WIFI_SSID;
                printf("BLU_NO_WIFI_SSID2\r\n");
                a = 0;
            }
            else
            {
                bzero(wifi_data.wifi_ssid, sizeof(wifi_data.wifi_ssid));
                strcpy(wifi_data.wifi_ssid, cjson_blu_data_parse_wifissid->valuestring);
                printf("wifi ssid=%s\r\n", cjson_blu_data_parse_wifissid->valuestring);
                a = 1;
            }
        }
        cjson_blu_data_parse_wifipwd = cJSON_GetObjectItem(cjson_blu_data_parse, "wifiPwd");
        if (cjson_blu_data_parse_wifipwd == NULL)
        {
            return BLU_NO_WIFI_PWD;
        }
        else
        {
            if (cjson_blu_data_parse_wifipwd->valuestring == (void *)0)
            {
                printf("BLU_NO_WIFI_PWD2\r\n");
            }
            else
            {
                bzero(wifi_data.wifi_pwd, sizeof(wifi_data.wifi_pwd));
                strcpy(wifi_data.wifi_pwd, cjson_blu_data_parse_wifipwd->valuestring);
                printf("wifi pwd=%s\r\n", cjson_blu_data_parse_wifipwd->valuestring);
                b = 1;
            }
        }
        cjson_blu_data_parse_ob = cJSON_GetObjectItem(cjson_blu_data_parse, "s2");
        if (cjson_blu_data_parse_ob == NULL)
        {
            return BLU_NO_S2;
        }
        else
        {
            if (cjson_blu_data_parse_ob->valuestring == (void *)0)
            {
                printf("BLU_NO_S2\r\n");
            }
            else
            {
                printf("s2 %s\r\n", cjson_blu_data_parse_ob->valuestring);
                sscanf(cjson_blu_data_parse_ob->valuestring, "%2s", get_num);
                ob_blu_json.T2_h = atoi(get_num);
                ob_blu_json.T2_m = atoi((cjson_blu_data_parse_ob->valuestring) + 3);
                printf("T2 %d %d \r\n", ob_blu_json.T2_h, ob_blu_json.T2_m);
            }
        }
        cjson_blu_data_parse_ob = cJSON_GetObjectItem(cjson_blu_data_parse, "e2");
        if (cjson_blu_data_parse_ob == NULL)
        {
            return BLU_NO_E2;
        }
        else
        {
            if (cjson_blu_data_parse_ob->valuestring == (void *)0)
            {
                printf("BLU_NO_E2\r\n");
            }
            else
            {
                printf("e2 %s\r\n", cjson_blu_data_parse_ob->valuestring);
                sscanf(cjson_blu_data_parse_ob->valuestring, "%2s", get_num);
                ob_blu_json.T3_h = atoi(get_num);
                ob_blu_json.T3_m = atoi((cjson_blu_data_parse_ob->valuestring) + 3);
                printf("T3 %d %d \r\n", ob_blu_json.T3_h, ob_blu_json.T3_m);
            }
        }
        cjson_blu_data_parse_command = cJSON_GetObjectItem(cjson_blu_data_parse, "command");
        printf("command=%s\r\n", cjson_blu_data_parse_command->valuestring);
        if (strcmp(cjson_blu_data_parse_command->valuestring, "SetupConfig") == 0) //收到全部信息指令
        {
            if ((cjson_blu_data_parse_ob = cJSON_GetObjectItem(cjson_blu_data_parse, "serial")) != NULL)
            {
                printf("serial %s\r\n", cjson_blu_data_parse_ob->valuestring);
                n = strlen(cjson_blu_data_parse_ob->valuestring) / 2;
                key = (char *)malloc(sizeof(char) * n);
                for (i = 0; i < n; ++i)
                {
                    sscanf(cjson_blu_data_parse_ob->valuestring + 2 * i, "%2hhX", key + i);
                }
                for (i = 0; i < n; ++i)
                {
                    ob_blu_json.WallKeyId[i] = key[i];
                    printf("0x%02X ", key[i]);
                }
                free(key);
                printf("\r\n");
            }
            if ((cjson_blu_data_parse_ob = cJSON_GetObjectItem(cjson_blu_data_parse, "switch")) != NULL)
            {
                printf("switch %d\r\n", cjson_blu_data_parse_ob->valueint);
                ob_blu_json.Switch = cjson_blu_data_parse_ob->valueint;
            }
            //initialise_wifi(cjson_blu_data_parse_wifissid->valuestring, cjson_blu_data_parse_wifipwd->valuestring); //重新初始化WIFI
        }
    }
    if ((a == 1) && (b == 1)) //有网络
    {
        initialise_wifi(); //重新初始化WIFI
        blu_ret = BLU_RESULT_SUCCESS;
        start_read_blue_mode = BLU_RESULT_SUCCESS;
        printf("BLU_RESULT_SUCCESS\r\n");
    }
    else if (a == 0)
    {
        blu_ret = BLU_RESULT_SUCCESS;
        start_read_blue_mode = BLU_COMMAND_CALCULATION;
        printf("BLU_COMMAND_CALCULATION\r\n");
    }
    cJSON_Delete(cjson_blu_data_parse);
    return blu_ret;
}
*/