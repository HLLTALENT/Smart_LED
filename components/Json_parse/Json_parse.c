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

extern uint8_t human_status;

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

int read_bluetooth(void)
{
    uint8_t bluetooth_sta[512];

    E2prom_BluRead(bluetooth_sta);
    printf("bluetooth_sta=%s\n", bluetooth_sta);
    if (strlen((char *)bluetooth_sta) == 0) //未读到蓝牙配置信息
    {
        return 0;
    }
    uint8_t ret = parse_objects_bluetooth((char *)bluetooth_sta);
    if ((ret == BLU_PWD_REFUSE) || (ret == BLU_JSON_FORMAT_ERROR))
    {
        return 0;
    }
    else
    {
        return ret;
    }
}

esp_err_t parse_objects_bluetooth(char *blu_json_data)
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

    //数据包包含NULL则直接返回error
    /*if (strstr(blu_json_data, "null") != NULL) //需要解析的字符串内含有null
        {
                printf("there is null in blu data\r\n");
                
                return BLU_PWD_REFUSE;
        }*/

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
            printf("BLU_NO_WIFI_SSID1\r\n");
        }
        else
        {
            if (cjson_blu_data_parse_wifissid->valuestring == (void *)0)
            {
                //return BLU_NO_WIFI_SSID;
                printf("BLU_NO_WIFI_SSID2\r\n");
            }
            else
            {
                bzero(wifi_data.wifi_ssid, sizeof(wifi_data.wifi_ssid));
                strcpy(wifi_data.wifi_ssid, cjson_blu_data_parse_wifissid->valuestring);
                printf("wifi ssid=%s\r\n", cjson_blu_data_parse_wifissid->valuestring);
            }
        }

        cjson_blu_data_parse_wifipwd = cJSON_GetObjectItem(cjson_blu_data_parse, "wifiPwd");
        if (cjson_blu_data_parse_wifipwd == NULL)
        {
            return BLU_NO_WIFI_PWD;
            printf("BLU_NO_WIFI_PWD1\r\n");
        }
        else
        {
            if (cjson_blu_data_parse_wifipwd->valuestring == (void *)0)
            {
                //return BLU_NO_WIFI_PWD;
                printf("BLU_NO_WIFI_PWD2\r\n");
            }
            else
            {
                bzero(wifi_data.wifi_pwd, sizeof(wifi_data.wifi_pwd));
                strcpy(wifi_data.wifi_pwd, cjson_blu_data_parse_wifipwd->valuestring);
                printf("wifi pwd=%s\r\n", cjson_blu_data_parse_wifipwd->valuestring);
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
                //return BLU_NO_S2;
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
                //return BLU_NO_E2;
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
            blu_ret = BLU_COMMAND_SWITCH;
        }
    }

    cJSON_Delete(cjson_blu_data_parse);
    return blu_ret;
}

//解析激活返回数据
esp_err_t parse_objects_http_active(char *http_json_data)
{
    cJSON *json_data_parse = NULL;
    cJSON *json_data_parse_value = NULL;
    cJSON *json_data_parse_time_value = NULL;
    cJSON *json_data_parse_channel_channel_write_key = NULL;
    cJSON *json_data_parse_channel_channel_id_value = NULL;
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

            //printf("api key=%s\r\n", json_data_parse_channel_channel_write_key->valuestring);
            //printf("channel_id=%s\r\n", json_data_parse_channel_channel_id_value->valuestring);

            //写入API-KEY
            sprintf(ApiKey, "%s%c", json_data_parse_channel_channel_write_key->valuestring, '\0');
            //printf("api key=%s\r\n", ApiKey);
            E2prom_Write(0x00, (uint8_t *)ApiKey, 32);

            //写入channelid
            sprintf(ChannelId, "%s%c", json_data_parse_channel_channel_id_value->valuestring, '\0');
            //printf("channel_id=%s\r\n", ChannelId);
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
                ;
                E2prom_Write(0x20, data_write3, 16);

                //fflush(stdout); //使stdout清空，就会立刻输出所有在缓冲区的内容。
                //esp_restart();  //芯片复位 函数位于esp_system.h
            }
        }
    }
    cJSON_Delete(json_data_parse);
    return 1;
}

esp_err_t parse_Uart0(char *json_data)
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
        need_send = 1;
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
                    if (((human_status == 1) && (strcmp(json_data_action->valuestring, "command") == 0)) || (work_status == LUNCHTIME)) //有人则退出
                    {

                        printf("Json Formatting error6\n");

                        cJSON_Delete(json_data_parse);
                        return 0;
                    }
                    else
                    {
                        //human_status = 0;
                        auto_ctl_count = 0;
                        strcpy(mqtt_json_s.mqtt_control_char, "command");

                        if ((json_data_mode_parse = cJSON_GetObjectItem(json_data_string_parse, "s_port")) != NULL)
                        {
                            if (strncmp("port1", json_data_mode_parse->valuestring, strlen("port1")) == 0) //上灯条
                            {
                                strcpy(mqtt_json_s.mqtt_mode_char, "port1");
                                if ((json_data_stage_parse = cJSON_GetObjectItem(json_data_string_parse, "set_state")) != NULL)
                                {
                                    if (strncmp("100", json_data_stage_parse->valuestring, strlen("100")) == 0)
                                    {
                                        strcpy(mqtt_json_s.mqtt_stage, "100");
                                        Up_Light_Status = 0;
                                        Led_UP_W(100, 10);
                                        Led_UP_Y(100, 10);
                                    }
                                    if (strncmp("0", json_data_stage_parse->valuestring, strlen("0")) == 0)
                                    {
                                        strcpy(mqtt_json_s.mqtt_stage, "0");
                                        Up_Light_Status = 1;
                                        temp_hour = -1;
                                    }
                                }
                            }
                            if (strncmp("port2", json_data_mode_parse->valuestring, strlen("port2")) == 0)
                            {
                                strcpy(mqtt_json_s.mqtt_mode_char, "port2");
                                if ((json_data_stage_parse = cJSON_GetObjectItem(json_data_string_parse, "set_state")) != NULL)
                                {
                                    if (strncmp("100", json_data_stage_parse->valuestring, strlen("100")) == 0)
                                    {
                                        strcpy(mqtt_json_s.mqtt_stage, "100");
                                        Down_Light_Status = 0;
                                        Led_DOWN_W(100, 10);
                                        Led_DOWN_Y(100, 10);
                                    }
                                    if (strncmp("0", json_data_stage_parse->valuestring, strlen("0")) == 0)
                                    {
                                        strcpy(mqtt_json_s.mqtt_stage, "0");
                                        Down_Light_Status = 1;
                                        temp_hour = -1;
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
    }
    return 1;
}

void create_http_json(uint8_t post_status, creat_json *pCreat_json)
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
        }
        else if (human_status == HAVEHUMAN)
        {
            cJSON_AddItemToObject(next, "field3", cJSON_CreateString("1"));
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
