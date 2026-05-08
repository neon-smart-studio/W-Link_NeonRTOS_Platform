
#ifndef HDC1080_H
#define HDC1080_H

#include "Sensor_Config.h"

#define HDC1080_I2C_ADDR                 0x80

#ifndef CONFIG_HDC1080_I2C_INDEX
#define HDC1080_I2C_INDEX           hwI2C_Index_0
#else
#define HDC1080_I2C_INDEX           CONFIG_HDC1080_I2C_INDEX
#endif

typedef enum HDC1080_OpStatus_t
{
  HDC1080_OK = 0,
  HDC1080_NotInit = -1,
  HDC1080_InvalidParameter = -2,
  HDC1080_MemoryError = -3,
  HDC1080_MutexTimeout = -4,
  HDC1080_IO_Error = -5,
  HDC1080_Unsupport = -6
} HDC1080_OpStatus;

typedef enum HDC1080_MeasurementResolution_t{
	HDC1080_MeasurementResolution_8Bit = 0,
	HDC1080_MeasurementResolution_11Bit = 1,
	HDC1080_MeasurementResolution_14Bit = 2,
	HDC1080_MeasurementResolution_MAX = 3,
} HDC1080_MeasurementResolution;

typedef union HDC1080_SerialNumber_t{
	uint8_t rawData[6];
	struct {
		uint16_t serialFirst;
		uint16_t serialMid;
		uint16_t serialLast;
	};
}HDC1080_SerialNumber;

HDC1080_OpStatus HDC1080_Init();
HDC1080_OpStatus HDC1080_SetResolution(HDC1080_MeasurementResolution ms_temp_resolution, HDC1080_MeasurementResolution ms_hum_resolution);
HDC1080_OpStatus HDC1080_ReadSerialNumber(HDC1080_SerialNumber* sn);
HDC1080_OpStatus HDC1080_HeatUp(uint8_t seconds);
HDC1080_OpStatus HDC1080_ReadTemperature(double* temperature);
HDC1080_OpStatus HDC1080_ReadHumidity(double* humidity);
HDC1080_OpStatus HDC1080_ReadManufacturerId(uint16_t* manuID);
HDC1080_OpStatus HDC1080_ReadDeviceId(uint16_t* devID);

#endif //HDC1080_H