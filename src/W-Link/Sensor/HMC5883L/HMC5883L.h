
#ifndef HMC5883L_H
#define HMC5883L_H

#include "Sensor_Config.h"

#define HMC5883L_I2C_ADDR                 0x1E

#ifndef CONFIG_HMC5883L_I2C_INDEX
#define HMC5883L_I2C_INDEX           hwI2C_Index_0
#else
#define HMC5883L_I2C_INDEX           CONFIG_HMC5883L_I2C_INDEX
#endif

typedef enum HMC5883L_OpResult_t
{
  HMC5883L_OK = 0,
  HMC5883L_NotInit = -1,
  HMC5883L_InvalidParameter = -2,
  HMC5883L_MemoryError = -3,
  HMC5883L_MutexTimeout = -5,
  HMC5883L_IO_Error = -5,
  HMC5883L_Unsupport = -6
} HMC5883L_OpResult;

typedef enum HMC5883L_Samples_t
{
    HMC5883L_Samples_1     = 0,
    HMC5883L_Samples_2     = 1,
    HMC5883L_Samples_4     = 2,
    HMC5883L_Samples_8     = 3,
    HMC5883L_Samples_MAX   = 4
} HMC5883L_Samples;

typedef enum HMC5883L_DataRate_t
{
    HMC5883L_DataRate_0_75_Hz    = 0,
    HMC5883L_DataRate_1_5Hz      = 1,
    HMC5883L_DataRate_3Hz        = 2,
    HMC5883L_DataRate_7_5Hz      = 3,
    HMC5883L_DataRate_15Hz       = 4,
    HMC5883L_DataRate_30Hz       = 5,
    HMC5883L_DataRate_75Hz       = 6,
    HMC5883L_DataRate_MAX        = 7
} HMC5883L_DataRate;

typedef enum HMC5883L_Range_t
{
    HMC5883L_Range_0_88Ga    = 0,
    HMC5883L_Range_1_3Ga     = 1,
    HMC5883L_Range_1_9Ga     = 2,
    HMC5883L_Range_2_5Ga     = 3,
    HMC5883L_Range_4Ga       = 4,
    HMC5883L_Range_4_7Ga     = 5,
    HMC5883L_Range_5_6Ga     = 6,
    HMC5883L_Range_8_1Ga     = 7,
    HMC5883L_Range_MAX       = 8
} HMC5883L_Range;

typedef enum HMC5883L_Mode_t
{
    HMC5883L_Mode_Continous     = 0,
    HMC5883L_Mode_Single        = 1,
    HMC5883L_Mode_Idle          = 2,
    HMC5883L_Mode_MAX           = 3
} HMC5883L_Mode;

typedef struct HMC5883L_Vector_t
{
    float XAxis;
    float YAxis;
    float ZAxis;
} HMC5883L_Vector;

HMC5883L_OpResult HMC5883L_Init();
HMC5883L_OpResult HMC5883L_ReadRaw(HMC5883L_Vector* vector);
HMC5883L_OpResult HMC5883L_ReadNormalize(HMC5883L_Vector* vector);
HMC5883L_OpResult HMC5883L_SetOffset(int xOff, int yOff);
HMC5883L_OpResult HMC5883L_SetRange(HMC5883L_Range range);
HMC5883L_OpResult HMC5883L_GetRange(HMC5883L_Range* range);
HMC5883L_OpResult HMC5883L_SetMeasurementMode(HMC5883L_Mode mode);
HMC5883L_OpResult HMC5883L_GetMeasurementMode(HMC5883L_Mode* mode);
HMC5883L_OpResult HMC5883L_SetDataRate(HMC5883L_DataRate dataRate);
HMC5883L_OpResult HMC5883L_GetDataRate(HMC5883L_DataRate* dataRate);
HMC5883L_OpResult HMC5883L_SetSamples(HMC5883L_Samples samples);
HMC5883L_OpResult HMC5883L_GetSamples(HMC5883L_Samples* samples);

#endif //HMC5883L_H