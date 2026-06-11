
#ifndef SI7021_H
#define SI7021_H

#include "Sensor_Config.h"

#define SI7021_I2C_ADDR                         0x40

#ifndef CONFIG_SI7021_I2C_INDEX
#define SI7021_I2C_INDEX                   hwI2C_Index_0
#else
#define SI7021_I2C_INDEX                   CONFIG_SI7021_I2C_INDEX
#endif

typedef enum Si7021_OpResult_t
{
  Si7021_OK = 0,
  Si7021_NotInit = -1,
  Si7021_InvalidParameter = -2,
  Si7021_MemoryError = -3,
  Si7021_MutexTimeout = -4,
  Si7021_IO_Error = -5,
  Si7021_Unsupport = -6
} Si7021_OpResult;

typedef enum Si_Sensor_Model_t {
  Si_Sensor_Model_Engineering_Samples = 0,
  Si_Sensor_Model_7013 = 1,
  Si_Sensor_Model_7020 = 2,
  Si_Sensor_Model_7021 = 3,
  Si_Sensor_Model_MAX = 4,
}Si_Sensor_Model;

typedef enum Si_Sensor_Rev_t {
  Si_Sensor_Rev_1 = 0,
  Si_Sensor_Rev_2 = 1,
  Si_Sensor_Rev_MAX = 2,
}Si_Sensor_Rev;

Si7021_OpResult Si7021_Init();
Si7021_OpResult Si7021_ReadMeasureTemperature(float* temperature);
Si7021_OpResult Si7021_ReadMeasureHumidity(float* humidity);
Si7021_OpResult Si7021_ReadPreviousMeasureTemperature(float* temperature);
Si7021_OpResult Si7021_ReadRevision(Si_Sensor_Rev* revision);
Si7021_OpResult Si7021_ReadSerialNumber(Si_Sensor_Model* model, uint8_t ID_A[4], uint8_t ID_B[4]);

#endif //SI7021_H