/*
EEPROM读写程序AT24C08
创建日期：2018年10月29日
作者：孙浩
更新日期：2018年10月31日
作者：孙浩
EEPROM写入和读取长度增加大于16个字节
更新日期：2018年10月31日
作者：孙浩
EEPROM写入和读取起始字节必必须是16的倍数

E2prom_Init()
用于EEPROM的初始化，主要包括GPIO初始化和IIC初始化，在初始化模块中调用
SCL_IO=14               
SDA_IO=18      

int E2prom_Write(uint8_t addr,uint8_t*data_write,int len);
用于写入EEPROM，参数：写入数据地址、指针和写入数据长度，操作扇区为0号扇区
地址范围0x00-0xff
返回值
ESP_OK 写入成功
其他 写入失败

int E2prom_Read(uint8_t addr,uint8_t*data_read,int len);
用于在0号扇区读出数据，参数：读出数据数据地址、指针和读出长度
地址范围0x00-0xff
返回值
ESP_OK 读取成功
其他 读取失败

*/

#ifndef _E2PROM_H_
#define _E2PROM_H_

#include "freertos/FreeRTOS.h"

#define I2C_MASTER_SCL1_IO 33        /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA1_IO 32        /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM0 I2C_NUM_0    /*!< I2C port number for master dev */
#define I2C_MASTER_TX1_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_MASTER_RX1_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ 100000    /*!< I2C master clock frequency */

#define ACK_CHECK_EN 0x1  /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0 /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0       /*!< I2C ack value */
#define NACK_VAL 0x1      /*!< I2C nack value */

extern void E2prom_Init(void);
extern int E2prom_Write(uint8_t addr, uint8_t *data_write, int len);
extern int E2prom_Read(uint8_t addr, uint8_t *data_read, int len);
extern int E2prom_BluWrite(uint8_t addr, uint8_t *data_write, int len);
//extern int E2prom_BluRead(uint8_t addr,uint8_t*data_read,int len);
extern int E2prom_BluRead(uint8_t *data_read);
#endif
