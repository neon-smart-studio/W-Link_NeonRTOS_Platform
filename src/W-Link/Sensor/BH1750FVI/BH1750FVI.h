
#ifndef BH1750FVI_H
#define BH1750FVI_H

#include "Sensor_Config.h"

#define BH1750FVI_I2C_ADD_L                    0xB8
#define BH1750FVI_I2C_ADD_H                    0x46
#define BH1750FVI_I2C_OP_TIMEOUT               500

#ifdef CONFIG_BH1750FVI_I2C_ADDR_L
#define BH1750FVI_I2C_ADDRESS                  BH1750FVI_I2C_ADD_L
#endif

#ifdef CONFIG_BH1750FVI_I2C_ADDR_H
#define BH1750FVI_I2C_ADDRESS                  BH1750FVI_I2C_ADD_H
#endif

#ifndef BH1750FVI_I2C_ADDRESS
#define BH1750FVI_I2C_ADDRESS                  BH1750FVI_I2C_ADD_L
#endif

#ifndef CONFIG_BH1750FVI_I2C_INDEX
#define BH1750FVI_I2C_INDEX               hwI2C_Index_0
#else
#define BH1750FVI_I2C_INDEX               CONFIG_BH1750FVI_I2C_PORT_INDEX
#endif

typedef enum BH1750FVI_OpResult_t
{
  BH1750FVI_OK = 0,
  BH1750FVI_NotInit = -1,
  BH1750FVI_InvalidParameter = -2,
  BH1750FVI_MemoryError = -3,
  BH1750FVI_MutexTimeout = -4,
  BH1750FVI_IO_Error = -5,
  BH1750FVI_Unsupport = -6
} BH1750FVI_OpResult;

typedef enum BH1750FVI_Resolution_t
{
  BH1750FVI_Resolution_411x = 0,
  BH1750FVI_Resolution_0_511x = 1,
  BH1750FVI_Resolution_11x = 2,
  BH1750FVI_Resolution_MAX = 3,
} BH1750FVI_Resolution;

BH1750FVI_OpResult BH1750FVI_Init();
BH1750FVI_OpResult BH1750FVI_Read_Lux(bool continuous, BH1750FVI_Resolution resolution, uint16_t* lux);

#endif //BH1750FVI_H