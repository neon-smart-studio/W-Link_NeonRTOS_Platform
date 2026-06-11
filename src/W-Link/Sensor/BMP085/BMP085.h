
#ifndef BMP085_H
#define BMP085_H

#include "Sensor_Config.h"

#define BMP085_I2C_ADDR                  0xEE

#ifndef CONFIG_BMP085_I2C_INDEX
#define BMP085_I2C_INDEX            hwI2C_Index_0
#else
#define BMP085_I2C_INDEX            CONFIG_BMP085_I2C_PORT_INDEX
#endif

typedef enum BMP085_OpResult_t
{
  BMP085_OK = 0,
  BMP085_NotInit = -1,
  BMP085_InvalidParameter = -2,
  BMP085_MemoryError = -3,
  BMP085_MutexTimeout = -4,
  BMP085_IO_Error = -5,
  BMP085_Unsupport = -6
} BMP085_OpResult;

typedef enum BMP085_Resolution_t
{
  BMP085_Resolution_Low = 0,
  BMP085_Resolution_Standard = 1,
  BMP085_Resolution_High = 2,
  BMP085_Resolution_Ultra_High = 3,
  BMP085_Resolution_MAX = 4,
} BMP085_Resolution;

BMP085_OpResult BMP085_Init();
BMP085_OpResult BMP085_Calibration();
BMP085_OpResult BMP085_Read_Temperature(double* temperature);
BMP085_OpResult BMP085_Read_Measure_Pressure(BMP085_Resolution resolution, long* pressure);

#endif //BMP085_H