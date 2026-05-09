/*
This LSM6DSO driver is based on STMicroelectronics stm32-lsm6dso / LSM6DSO component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef LSM6DSO_H
#define LSM6DSO_H

#include "LSM6DSO_IO.h"
#include "LSM6DSO_Util.h"

#include "LSM6DSO_Def.h"
#include "LSM6DSO_Register_Def.h"

#include "LSM6DSO_Register.h"

#define LSM6DSO_ACC_SENSITIVITY_FS_2G   0.061f
#define LSM6DSO_ACC_SENSITIVITY_FS_4G   0.122f
#define LSM6DSO_ACC_SENSITIVITY_FS_8G   0.244f
#define LSM6DSO_ACC_SENSITIVITY_FS_16G  0.488f

#define LSM6DSO_GYRO_SENSITIVITY_FS_125DPS    4.375f
#define LSM6DSO_GYRO_SENSITIVITY_FS_250DPS    8.750f
#define LSM6DSO_GYRO_SENSITIVITY_FS_500DPS   17.500f
#define LSM6DSO_GYRO_SENSITIVITY_FS_1000DPS  35.000f
#define LSM6DSO_GYRO_SENSITIVITY_FS_2000DPS  70.000f

#define LSM6DSO_INT1_PIN                     0
#define LSM6DSO_INT2_PIN                     1

/* Typedefs ------------------------------------------------------------------*/

typedef enum LSM6DSO_SensorIntPin_t
{
  LSM6DSO_INT1_Pin = 0,
  LSM6DSO_INT2_Pin = 1,
  LSM6DSO_INT_Pin_MAX = 2
} LSM6DSO_SensorIntPin;

typedef enum LSM6DSO_ACC_Operating_Mode_t
{
  LSM6DSO_ACC_HIGH_PERFORMANCE_MODE,
  LSM6DSO_ACC_LOW_POWER_NORMAL_MODE,
  LSM6DSO_ACC_ULTRA_LOW_POWER_MODE
} LSM6DSO_ACC_Operating_Mode;

typedef enum LSM6DSO_GYRO_Operating_Mode_t
{
  LSM6DSO_GYRO_HIGH_PERFORMANCE_MODE,
  LSM6DSO_GYRO_LOW_POWER_NORMAL_MODE
} LSM6DSO_GYRO_Operating_Mode;

typedef struct LSM6DSO_Event_Status_t
{
  unsigned int FreeFallStatus : 1;
  unsigned int TapStatus : 1;
  unsigned int DoubleTapStatus : 1;
  unsigned int WakeUpStatus : 1;
  unsigned int StepStatus : 1;
  unsigned int TiltStatus : 1;
  unsigned int D6DOrientationStatus : 1;
  unsigned int SleepStatus : 1;
} LSM6DSO_Event_Status;

LSM6DSO_OpStatus LSM6DSO_Init();
LSM6DSO_OpStatus LSM6DSO_DeInit();
LSM6DSO_OpStatus LSM6DSO_ReadID(uint8_t *Id);

LSM6DSO_OpStatus LSM6DSO_Enable_X();
LSM6DSO_OpStatus LSM6DSO_Disable_X();
LSM6DSO_OpStatus LSM6DSO_Get_X_Sensitivity(float *Sensitivity);
LSM6DSO_OpStatus LSM6DSO_Get_X_ODR(float *Odr);
LSM6DSO_OpStatus LSM6DSO_Set_X_ODR(float Odr);
LSM6DSO_OpStatus LSM6DSO_Set_X_ODR_With_Mode(float Odr, LSM6DSO_ACC_Operating_Mode Mode);
LSM6DSO_OpStatus LSM6DSO_Get_X_FS(int32_t *FullScale);
LSM6DSO_OpStatus LSM6DSO_Set_X_FS(int32_t FullScale);
LSM6DSO_OpStatus LSM6DSO_Get_X_AxesRaw(int16_t *rawX, int16_t *rawY, int16_t *rawZ);
LSM6DSO_OpStatus LSM6DSO_Get_X_Axes(float *accelerationX, float *accelerationY, float *accelerationZ);

LSM6DSO_OpStatus LSM6DSO_Enable_G();
LSM6DSO_OpStatus LSM6DSO_Disable_G();
LSM6DSO_OpStatus LSM6DSO_Get_G_Sensitivity(float *Sensitivity);
LSM6DSO_OpStatus LSM6DSO_Get_G_ODR(float *Odr);
LSM6DSO_OpStatus LSM6DSO_Set_G_ODR(float Odr);
LSM6DSO_OpStatus LSM6DSO_Set_G_ODR_With_Mode(float Odr, LSM6DSO_GYRO_Operating_Mode Mode);
LSM6DSO_OpStatus LSM6DSO_Get_G_FS(int32_t *FullScale);
LSM6DSO_OpStatus LSM6DSO_Set_G_FS(int32_t FullScale);
LSM6DSO_OpStatus LSM6DSO_Get_G_AxesRaw(int16_t *rawX, int16_t *rawY, int16_t *rawZ);
LSM6DSO_OpStatus LSM6DSO_Get_G_Axes(float *angularRateX, float *angularRateY, float *angularRateZ);

LSM6DSO_OpStatus LSM6DSO_Set_Interrupt_Latch(uint8_t Status);

LSM6DSO_OpStatus LSM6DSO_Enable_Free_Fall_Detection(LSM6DSO_SensorIntPin IntPin);
LSM6DSO_OpStatus LSM6DSO_Disable_Free_Fall_Detection();
LSM6DSO_OpStatus LSM6DSO_Set_Free_Fall_Threshold(uint8_t Threshold);
LSM6DSO_OpStatus LSM6DSO_Set_Free_Fall_Duration(uint8_t Duration);

LSM6DSO_OpStatus LSM6DSO_Enable_Pedometer();
LSM6DSO_OpStatus LSM6DSO_Disable_Pedometer();
LSM6DSO_OpStatus LSM6DSO_Get_Step_Count(uint16_t *StepCount);
LSM6DSO_OpStatus LSM6DSO_Step_Counter_Reset();

LSM6DSO_OpStatus LSM6DSO_Enable_Tilt_Detection(LSM6DSO_SensorIntPin IntPin);
LSM6DSO_OpStatus LSM6DSO_Disable_Tilt_Detection();

LSM6DSO_OpStatus LSM6DSO_Enable_Wake_Up_Detection(LSM6DSO_SensorIntPin IntPin);
LSM6DSO_OpStatus LSM6DSO_Disable_Wake_Up_Detection();
LSM6DSO_OpStatus LSM6DSO_Set_Wake_Up_Threshold(uint8_t Threshold);
LSM6DSO_OpStatus LSM6DSO_Set_Wake_Up_Duration(uint8_t Duration);

LSM6DSO_OpStatus LSM6DSO_Enable_Single_Tap_Detection(LSM6DSO_SensorIntPin IntPin);
LSM6DSO_OpStatus LSM6DSO_Disable_Single_Tap_Detection();
LSM6DSO_OpStatus LSM6DSO_Enable_Double_Tap_Detection(LSM6DSO_SensorIntPin IntPin);
LSM6DSO_OpStatus LSM6DSO_Disable_Double_Tap_Detection();
LSM6DSO_OpStatus LSM6DSO_Set_Tap_Threshold(uint8_t Threshold);
LSM6DSO_OpStatus LSM6DSO_Set_Tap_Shock_Time(uint8_t Time);
LSM6DSO_OpStatus LSM6DSO_Set_Tap_Quiet_Time(uint8_t Time);
LSM6DSO_OpStatus LSM6DSO_Set_Tap_Duration_Time(uint8_t Time);

LSM6DSO_OpStatus LSM6DSO_Enable_6D_Orientation(LSM6DSO_SensorIntPin IntPin);
LSM6DSO_OpStatus LSM6DSO_Disable_6D_Orientation();
LSM6DSO_OpStatus LSM6DSO_Set_6D_Orientation_Threshold(uint8_t Threshold);
LSM6DSO_OpStatus LSM6DSO_Get_6D_Orientation_XL(uint8_t *XLow);
LSM6DSO_OpStatus LSM6DSO_Get_6D_Orientation_XH(uint8_t *XHigh);
LSM6DSO_OpStatus LSM6DSO_Get_6D_Orientation_YL(uint8_t *YLow);
LSM6DSO_OpStatus LSM6DSO_Get_6D_Orientation_YH(uint8_t *YHigh);
LSM6DSO_OpStatus LSM6DSO_Get_6D_Orientation_ZL(uint8_t *ZLow);
LSM6DSO_OpStatus LSM6DSO_Get_6D_Orientation_ZH(uint8_t *ZHigh);

LSM6DSO_OpStatus LSM6DSO_Get_X_DRDY_Status(uint8_t *Status);
LSM6DSO_OpStatus LSM6DSO_Get_X_Event_Status(LSM6DSO_Event_Status *Status);
LSM6DSO_OpStatus LSM6DSO_Set_X_SelfTest(uint8_t Status);

LSM6DSO_OpStatus LSM6DSO_Get_G_DRDY_Status(uint8_t *Status);
LSM6DSO_OpStatus LSM6DSO_Set_G_SelfTest(uint8_t Status);

LSM6DSO_OpStatus LSM6DSO_Get_FIFO_Num_Samples(uint16_t *NumSamples);
LSM6DSO_OpStatus LSM6DSO_Get_FIFO_Full_Status(uint8_t *Status);
LSM6DSO_OpStatus LSM6DSO_Set_FIFO_INT1_FIFO_Full(uint8_t Status);
LSM6DSO_OpStatus LSM6DSO_Set_FIFO_INT2_FIFO_Full(uint8_t Status);
LSM6DSO_OpStatus LSM6DSO_Set_FIFO_Watermark_Level(uint16_t Watermark);
LSM6DSO_OpStatus LSM6DSO_Get_FIFO_Watermark_Level(uint16_t* Watermark);
LSM6DSO_OpStatus LSM6DSO_Set_FIFO_Stop_On_Fth(uint8_t Status);
LSM6DSO_OpStatus LSM6DSO_Set_FIFO_Mode(uint8_t Mode);
LSM6DSO_OpStatus LSM6DSO_Get_FIFO_Tag(uint8_t *Tag);
LSM6DSO_OpStatus LSM6DSO_Get_FIFO_Data_Raw(uint8_t *Data);
LSM6DSO_OpStatus LSM6DSO_Get_FIFO_X_Axes(float *accelerationX, float *accelerationY, float *accelerationZ);
LSM6DSO_OpStatus LSM6DSO_Set_FIFO_X_BDR(float Bdr);
LSM6DSO_OpStatus LSM6DSO_Get_FIFO_G_Axes(float *angularVelocityX, float *angularVelocityY, float *angularVelocityZ);
LSM6DSO_OpStatus LSM6DSO_Set_FIFO_G_BDR(float Bdr);

#endif //LSM6DSO_H