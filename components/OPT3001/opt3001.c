/*******************************************************************************
  * @file       OPT3001 Light Sensor DRIVER APPLICATION      
  * @author 
  * @version
  * @date 
  * @brief
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/i2c.h"

#include "opt3001.h"
#include "E2prom.h"
#include "math.h"

static esp_err_t OPT3001_Write(uint8_t sla_addr, uint8_t reg_addr, uint8_t *dat, int length)
{
    int ret;
    int i;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 2 * sla_addr, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    for (i = 0; i < length; i++)
    {
        i2c_master_write_byte(cmd, *dat, ACK_CHECK_EN);
        dat++;
    }
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t OPT3001_Read(uint8_t sla_addr, uint8_t reg_addr, uint8_t *dat, int length)
{
    int ret;
    int i;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 2 * sla_addr, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 2 * sla_addr + 1, ACK_CHECK_EN);

    for (i = 0; i < length; i++)
    {
        if (i != length - 1)
        {
            i2c_master_read_byte(cmd, dat, ACK_VAL);
            dat++;
        }
        else
        {
            i2c_master_read_byte(cmd, dat, NACK_VAL);
        }
    }
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

void MulTry_I2C_RD_mulReg(uint8_t sla_addr, uint8_t reg_addr, uint8_t *dat, uint8_t length)
{
    uint8_t n_try;
    for (n_try = 0; n_try < RETRY_TIME_OUT; n_try++)
    {
        if (OPT3001_Read(sla_addr, reg_addr, dat, length) == 1)
        {
            break;
        }
        vTaskDelay(3 / portTICK_RATE_MS);
    }
}

void MulTry_I2C_WR_mulReg(uint8_t sla_addr, uint8_t reg_addr, uint8_t *dat, uint8_t length)
{
    uint8_t n_try;
    for (n_try = 0; n_try < RETRY_TIME_OUT; n_try++)
    {
        if (OPT3001_Write(sla_addr, reg_addr, dat, length) == 1)
        {
            break;
        }
        vTaskDelay(6 / portTICK_RATE_MS);
    }
}

/*******************************************************************************
  write opt3001 light sensor register
*******************************************************************************/
static void OPT3001_WriteReg(uint8_t regaddr, uint16_t val)
{
    uint8_t reg[2];

    reg[0] = val / 256; //value high 8bit

    reg[1] = val % 256; //value low 8bit

    MulTry_I2C_WR_mulReg(OPT3001_ADDR, regaddr, reg, 2); //write data
}

/*******************************************************************************
  read write opt3001 light sensor register
*******************************************************************************/
static uint16_t OPT3001_ReadReg(uint8_t regaddr)
{
    uint8_t regdata[2];

    MulTry_I2C_RD_mulReg(OPT3001_ADDR, regaddr, regdata, 2); //read data

    return (uint16_t)((regdata[0] << 8) + regdata[1]);
}

/*******************************************************************************
  opt3001 light sensor init
*******************************************************************************/
short OPT3001_Init(void)
{
    if (OPT3001_ReadReg(DeviceID) == 0x3001) //read device id
    {
        OPT3001_WriteReg(LowLimit, 0x0064); //low limit 1.00-lux

        OPT3001_WriteReg(HighLimit, 0xBFFF); //high limit 83865.60-lux

        OPT3001_WriteReg(Configuration, 0xCE1A); //15:12-C: automatic full-scale,800ms conversion time field,continuous

        return 1;
    }
    return 0;
}

/*******************************************************************************
//measure the light value
*******************************************************************************/
void OPT3001_value(float *lightvalue)
{
    double e_val;
    float lsbsize;
    uint16_t retry;
    uint16_t read_val;

    if (OPT3001_ReadReg(DeviceID) == 0x3001) //read device id
    {
        for (retry = 0; retry < 1000; retry++)
        {
            if ((OPT3001_ReadReg(Configuration) & 0x0080) == 0x0080) //wait the conversion completed
            {
                read_val = OPT3001_ReadReg(Result); //read the result register

                e_val = (read_val >> 12) & 0x000f;

                lsbsize = 0.01 * pow(2.0, (double)e_val);

                *lightvalue = (float)lsbsize * (float)(read_val & 0x0fff); //read result

                break;
            }
            else
            {
                *lightvalue = ERROR_CODE;
            }

            vTaskDelay(4.5 / portTICK_RATE_MS); //MAP_UtilsDelay(60000);  //delay about 4.5ms
        }
    }
    else
    {
        *lightvalue = ERROR_CODE;
    }
}

void osi_OPT3001_value(float *lightvalue)
{
    OPT3001_value(lightvalue);
}

/*******************************************************************************
                                      END         
*******************************************************************************/
