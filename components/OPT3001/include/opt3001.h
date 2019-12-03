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
#include "stdint.h"

#define I2C_MASTER_SCL_IO 33        /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 32        /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0    /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */

#define ACK_CHECK_EN 0x1  /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0 /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0       /*!< I2C ack value */
#define NACK_VAL 0x1      /*!< I2C nack value */

#define RETRY_TIME_OUT 3

#define OPT3001_ADDR 0X45
#define Result 0x00
#define Configuration 0x01
#define LowLimit 0x02
#define HighLimit 0x03
#define ManufacturerID 0x7e
#define DeviceID 0x7f

#define ERROR_CODE 0xffff

float *lightvalue;

/*******************************************************************************
 FUNCTION PROTOTYPES
*******************************************************************************/
extern short OPT3001_Init(void); //opt3001 light sensor init

//extern void OPT3001_value(float *lightvalue);  //measure the light value

extern void osi_OPT3001_value(float *lightvalue);

/*******************************************************************************
                                      END         
*******************************************************************************/
