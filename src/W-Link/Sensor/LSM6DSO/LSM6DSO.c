/*
This LSM6DSO driver is based on STMicroelectronics stm32-lsm6dso / LSM6DSO component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "LSM6DSO.h"

#include "LSM6DSO_Register_Def.h"
#include "LSM6DSO_Register.h"

static uint8_t acc_is_enabled = false;
static uint8_t gyro_is_enabled = false;
static LSM6DSO_ODR_XL acc_odr;
static LSM6DSO_ODR_G gyro_odr;

LSM6DSO_OpStatus LSM6DSO_Set_X_ODR_When_Enabled(float Odr);
LSM6DSO_OpStatus LSM6DSO_Set_X_ODR_When_Disabled(float Odr);
LSM6DSO_OpStatus LSM6DSO_Set_G_ODR_When_Enabled(float Odr);
LSM6DSO_OpStatus LSM6DSO_Set_G_ODR_When_Disabled(float Odr);

//static LSM6DSO_Interrupt_Event_Callback LSM6DSO_INT_Event_Callback = NULL;

static void LSM6DSO_INT_Pending_Function_Call(void * p1, uint32_t p2)
{
  LSM6DSO_Interrupt_Index int_index = (LSM6DSO_Interrupt_Index)p2;
  /*
  if(LSM6DSO_INT_Event_Callback!=NULL)
  {
      LSM6DSO_INT_Event_Callback(int_index, status0, status1);
  }
  */
}

static void LSM6DSO_INT_IRQ_Handler(LSM6DSO_Interrupt_Index index)
{
	NeonRTOS_PendingFunctionCall(LSM6DSO_INT_Pending_Function_Call, NULL, index);
}

LSM6DSO_OpStatus LSM6DSO_Init()
{
  LSM6DSO_OpStatus op_status;

  /* Disable I3C */
  op_status = LSM6DSO_Reg_I3C_Disable_Set(LSM6DSO_I3C_DISABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Enable register address automatically incremented during a multiple byte
  access with a serial interface. */
  op_status = LSM6DSO_Reg_Auto_Increment_Set(PROPERTY_ENABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Enable BDU */
  op_status = LSM6DSO_Reg_Block_Data_Update_Set(PROPERTY_ENABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* FIFO mode selection */
  op_status = LSM6DSO_Reg_FIFO_Mode_Set(LSM6DSO_BYPASS_MODE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Select default output data rate. */
  acc_odr = LSM6DSO_XL_ODR_104Hz;

  /* Output data rate selection - power down. */
  op_status = LSM6DSO_Reg_XL_Data_Rate_Set(LSM6DSO_XL_ODR_OFF);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Full scale selection. */
  op_status = LSM6DSO_Reg_XL_Full_Scale_Set(LSM6DSO_2g);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Select default output data rate. */
  gyro_odr = LSM6DSO_GY_ODR_104Hz;

  /* Output data rate selection - power down. */
  op_status = LSM6DSO_Reg_Gy_Data_Rate_Set(LSM6DSO_GY_ODR_OFF);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Full scale selection. */
  op_status = LSM6DSO_Reg_Gy_Full_Scale_Set(LSM6DSO_2000dps);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Enable_X();
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Enable_G();
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }
	
	op_status = LSM6DSO_Register_Interrupt_Handler(LSM6DSO_INT_IRQ_Handler);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }
	
  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_DeInit()
{
  LSM6DSO_OpStatus op_status;

	op_status = LSM6DSO_UnRegister_Interrupt_Handler();
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }
	
  op_status = LSM6DSO_Disable_X();
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Disable_G();
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_ReadID(uint8_t *Id)
{
  if(Id==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  return (LSM6DSO_OpStatus)LSM6DSO_Reg_Device_ID_Get(Id);
}

LSM6DSO_OpStatus LSM6DSO_Enable_X()
{
  LSM6DSO_OpStatus op_status;

  /* Check if the component is already enabled */
  if (acc_is_enabled == 1U)
  {
    return LSM6DSO_OK;
  }

  /* Output data rate selection. */
  op_status = LSM6DSO_Reg_XL_Data_Rate_Set(acc_odr);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  acc_is_enabled = true;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Disable_X()
{
  LSM6DSO_OpStatus op_status;

  /* Check if the component is already disabled */
  if (acc_is_enabled == 0U)
  {
    return LSM6DSO_OK;
  }

  /* Get current output data rate. */
  op_status = LSM6DSO_Reg_XL_Data_Rate_Get(&acc_odr);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Output data rate selection - power down. */
  op_status = LSM6DSO_Reg_XL_Data_Rate_Set(LSM6DSO_XL_ODR_OFF);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  acc_is_enabled = false;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_X_Sensitivity(float *Sensitivity)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_FS_XL full_scale;

  if(Sensitivity==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }

  /* Read actual full scale selection from sensor. */
  op_status = LSM6DSO_Reg_XL_Full_Scale_Get(&full_scale);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Store the Sensitivity based on actual full scale. */
  switch (full_scale)
  {
    case LSM6DSO_2g:
      *Sensitivity = LSM6DSO_ACC_SENSITIVITY_FS_2G;
      break;

    case LSM6DSO_4g:
      *Sensitivity = LSM6DSO_ACC_SENSITIVITY_FS_4G;
      break;

    case LSM6DSO_8g:
      *Sensitivity = LSM6DSO_ACC_SENSITIVITY_FS_8G;
      break;

    case LSM6DSO_16g:
      *Sensitivity = LSM6DSO_ACC_SENSITIVITY_FS_16G;
      break;

    default:
      return LSM6DSO_InvalidParameter;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_X_ODR(float *Odr)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_ODR_XL odr_low_level;

  if(Odr==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  /* Get current output data rate. */
  op_status = LSM6DSO_Reg_XL_Data_Rate_Get(&odr_low_level);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  switch (odr_low_level)
  {
    case LSM6DSO_XL_ODR_OFF:
      *Odr = 0.0f;
      break;

    case LSM6DSO_XL_ODR_1Hz6:
      *Odr = 1.6f;
      break;

    case LSM6DSO_XL_ODR_12Hz5:
      *Odr = 12.5f;
      break;

    case LSM6DSO_XL_ODR_26Hz:
      *Odr = 26.0f;
      break;

    case LSM6DSO_XL_ODR_52Hz:
      *Odr = 52.0f;
      break;

    case LSM6DSO_XL_ODR_104Hz:
      *Odr = 104.0f;
      break;

    case LSM6DSO_XL_ODR_208Hz:
      *Odr = 208.0f;
      break;

    case LSM6DSO_XL_ODR_417Hz:
      *Odr = 417.0f;
      break;

    case LSM6DSO_XL_ODR_833Hz:
      *Odr = 833.0f;
      break;

    case LSM6DSO_XL_ODR_1667Hz:
      *Odr = 1667.0f;
      break;

    case LSM6DSO_XL_ODR_3333Hz:
      *Odr = 3333.0f;
      break;

    case LSM6DSO_XL_ODR_6667Hz:
      *Odr = 6667.0f;
      break;

    default:
      return LSM6DSO_InvalidParameter;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_X_ODR(float Odr)
{
  return LSM6DSO_Set_X_ODR_With_Mode(Odr, LSM6DSO_ACC_HIGH_PERFORMANCE_MODE);
}

LSM6DSO_OpStatus LSM6DSO_Set_X_ODR_With_Mode(float Odr, LSM6DSO_ACC_Operating_Mode Mode)
{
  LSM6DSO_OpStatus op_status;

  switch (Mode)
  {
    case LSM6DSO_ACC_HIGH_PERFORMANCE_MODE:
    {
      /* We must uncheck Low Power and Ultra Low Power bits if they are enabled */
      LSM6DSO_Ctrl5_C val1;
      LSM6DSO_Ctrl6_C val2;

      op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL5_C, (uint8_t *)&val1, 1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      if (val1.xl_ulp_en)
      {
        /* Power off the accelerometer */
        if (acc_is_enabled == 1U)
        {
          op_status = LSM6DSO_Reg_XL_Data_Rate_Set(LSM6DSO_XL_ODR_OFF);
          if(op_status<LSM6DSO_OK)
          {
            return op_status;
          }
        }

        val1.xl_ulp_en = 0;

        op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL5_C, (uint8_t *)&val1, 1);
        if(op_status<LSM6DSO_OK)
        {
          return op_status;
        }
      }

      op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL6_C, (uint8_t *)&val2, 1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      if (val2.xl_hm_mode)
      {
        val2.xl_hm_mode = 0;

        op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL6_C, (uint8_t *)&val2, 1);
        if(op_status<LSM6DSO_OK)
        {
          return op_status;
        }
      }

      /* ODR should be at least 12.5Hz */
      if (Odr < 12.5f)
      {
        Odr = 12.5f;
      }
      break;
    }
    case LSM6DSO_ACC_LOW_POWER_NORMAL_MODE:
    {
      /* We must uncheck Ultra Low Power bit if it is enabled */
      /* and check the Low Power bit if it is unchecked       */
      LSM6DSO_Ctrl5_C val1;
      LSM6DSO_Ctrl6_C val2;

      op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL5_C, (uint8_t *)&val1, 1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      if (val1.xl_ulp_en)
      {
        /* Power off the accelerometer */
        if (acc_is_enabled == 1U)
        {
          op_status = LSM6DSO_Reg_XL_Data_Rate_Set(LSM6DSO_XL_ODR_OFF);
          if(op_status<LSM6DSO_OK)
          {
            return op_status;
          }
        }

        val1.xl_ulp_en = 0;

        op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL5_C, (uint8_t *)&val1, 1);
        if(op_status<LSM6DSO_OK)
        {
          return op_status;
        }
      }

      op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL6_C, (uint8_t *)&val2, 1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      if (!val2.xl_hm_mode)
      {
        val2.xl_hm_mode = 1U;

        op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL6_C, (uint8_t *)&val2, 1);
        if(op_status<LSM6DSO_OK)
        {
          return op_status;
        }
      }

      /* Now we need to limit the ODR to 208 Hz if it is higher */
      if (Odr > 208.0f)
      {
        Odr = 208.0f;
      }
      break;
    }
    case LSM6DSO_ACC_ULTRA_LOW_POWER_MODE:
    {
      /* We must uncheck Low Power bit if it is enabled                   */
      /* and check the Ultra Low Power bit if it is unchecked             */
      /* We must switch off gyro otherwise Ultra Low Power does not work  */
      LSM6DSO_Ctrl5_C val1;
      LSM6DSO_Ctrl6_C val2;

      op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL6_C, (uint8_t *)&val2, 1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      if (val2.xl_hm_mode)
      {
        val2.xl_hm_mode = 0;

        op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL6_C, (uint8_t *)&val2, 1);
        if(op_status<LSM6DSO_OK)
        {
          return op_status;
        }
      }

      /* Disable Gyro */
      if (gyro_is_enabled == 1U)
      {
        op_status = LSM6DSO_Disable_G();
        if (op_status < LSM6DSO_OK)
        {
          return op_status;
        }
      }

      op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL5_C, (uint8_t *)&val1, 1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      if (!val1.xl_ulp_en)
      {
        /* Power off the accelerometer */
        if (acc_is_enabled == 1U)
        {
          op_status = LSM6DSO_Reg_XL_Data_Rate_Set(LSM6DSO_XL_ODR_OFF);
          if(op_status<LSM6DSO_OK)
          {
            return op_status;
          }
        }

        val1.xl_ulp_en = 1U;
        
        op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL5_C, (uint8_t *)&val1, 1);
        if(op_status<LSM6DSO_OK)
        {
          return op_status;
        }
      }

      /* Now we need to limit the ODR to 208 Hz if it is higher */
      if (Odr > 208.0f)
      {
        Odr = 208.0f;
      }
      break;
    }
    default:
      return LSM6DSO_InvalidParameter;
  }
  
  /* Check if the component is enabled */
  if (acc_is_enabled == 1U)
  {
    return LSM6DSO_Set_X_ODR_When_Enabled(Odr);
  }
  else
  {
    return LSM6DSO_Set_X_ODR_When_Disabled(Odr);
  }
}

LSM6DSO_OpStatus LSM6DSO_Set_X_ODR_When_Enabled(float Odr)
{
  LSM6DSO_ODR_XL new_odr;
  LSM6DSO_OpStatus op_status;

  new_odr = (Odr <=    1.6f) ? LSM6DSO_XL_ODR_1Hz6
          : (Odr <=   12.5f) ? LSM6DSO_XL_ODR_12Hz5
          : (Odr <=   26.0f) ? LSM6DSO_XL_ODR_26Hz
          : (Odr <=   52.0f) ? LSM6DSO_XL_ODR_52Hz
          : (Odr <=  104.0f) ? LSM6DSO_XL_ODR_104Hz
          : (Odr <=  208.0f) ? LSM6DSO_XL_ODR_208Hz
          : (Odr <=  417.0f) ? LSM6DSO_XL_ODR_417Hz
          : (Odr <=  833.0f) ? LSM6DSO_XL_ODR_833Hz
          : (Odr <= 1667.0f) ? LSM6DSO_XL_ODR_1667Hz
          : (Odr <= 3333.0f) ? LSM6DSO_XL_ODR_3333Hz
          :                    LSM6DSO_XL_ODR_6667Hz;

  /* Output data rate selection. */
  op_status = LSM6DSO_Reg_XL_Data_Rate_Set(new_odr);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_X_ODR_When_Disabled(float Odr)
{
  acc_odr = (Odr <=    1.6f) ? LSM6DSO_XL_ODR_1Hz6
          : (Odr <=   12.5f) ? LSM6DSO_XL_ODR_12Hz5
          : (Odr <=   26.0f) ? LSM6DSO_XL_ODR_26Hz
          : (Odr <=   52.0f) ? LSM6DSO_XL_ODR_52Hz
          : (Odr <=  104.0f) ? LSM6DSO_XL_ODR_104Hz
          : (Odr <=  208.0f) ? LSM6DSO_XL_ODR_208Hz
          : (Odr <=  417.0f) ? LSM6DSO_XL_ODR_417Hz
          : (Odr <=  833.0f) ? LSM6DSO_XL_ODR_833Hz
          : (Odr <= 1667.0f) ? LSM6DSO_XL_ODR_1667Hz
          : (Odr <= 3333.0f) ? LSM6DSO_XL_ODR_3333Hz
          :                    LSM6DSO_XL_ODR_6667Hz;

  return LSM6DSO_OK;
}


LSM6DSO_OpStatus LSM6DSO_Get_X_FS(int32_t *FullScale)
{
  LSM6DSO_FS_XL fs_low_level;
  LSM6DSO_OpStatus op_status;

  if(FullScale==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  /* Read actual full scale selection from sensor. */
  op_status = LSM6DSO_Reg_XL_Full_Scale_Get(&fs_low_level);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  switch (fs_low_level)
  {
    case LSM6DSO_2g:
      *FullScale =  2;
      break;

    case LSM6DSO_4g:
      *FullScale =  4;
      break;

    case LSM6DSO_8g:
      *FullScale =  8;
      break;

    case LSM6DSO_16g:
      *FullScale = 16;
      break;

    default:
      return LSM6DSO_InvalidParameter;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_X_FS(int32_t FullScale)
{
  LSM6DSO_FS_XL new_fs;
  LSM6DSO_OpStatus op_status;

  /* Seems like MISRA C-2012 rule 14.3a violation but only from single file statical analysis point of view because
     the parameter passed to the function is not known at the moment of analysis */
  new_fs = (FullScale <= 2) ? LSM6DSO_2g
         : (FullScale <= 4) ? LSM6DSO_4g
         : (FullScale <= 8) ? LSM6DSO_8g
         :                    LSM6DSO_16g;

  op_status = LSM6DSO_Reg_XL_Full_Scale_Set(new_fs);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_X_AxesRaw(int16_t *rawX, int16_t *rawY, int16_t *rawZ)
{
  axis3bit16_t data_raw;
  LSM6DSO_OpStatus op_status;

  if(rawX==NULL || rawY==NULL || rawZ==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  /* Read raw data values. */
  op_status = LSM6DSO_Reg_Acceleration_Raw_Get(data_raw.u8bit);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Format the data. */
  *rawX = data_raw.i16bit[0];
  *rawY = data_raw.i16bit[1];
  *rawZ = data_raw.i16bit[2];

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_X_Axes(float *accelerationX, float *accelerationY, float *accelerationZ)
{
  axis3bit16_t data_raw;
  LSM6DSO_OpStatus op_status;
  float sensitivity = 0.0f;

  if(accelerationX==NULL || accelerationY==NULL || accelerationZ==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  /* Read raw data values. */
  op_status = LSM6DSO_Reg_Acceleration_Raw_Get(data_raw.u8bit);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Get LSM6DSO actual sensitivity. */
  op_status = LSM6DSO_Get_X_Sensitivity(&sensitivity);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Calculate the data. */
  *accelerationX = (float)((float)data_raw.i16bit[0] * sensitivity);
  *accelerationY = (float)((float)data_raw.i16bit[1] * sensitivity);
  *accelerationZ = (float)((float)data_raw.i16bit[2] * sensitivity);

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Enable_G()
{
  LSM6DSO_OpStatus op_status;

  /* Check if the component is already enabled */
  if (gyro_is_enabled == 1U)
  {
    return LSM6DSO_OK;
  }

  /* Output data rate selection. */
  op_status = LSM6DSO_Reg_Gy_Data_Rate_Set(gyro_odr);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  gyro_is_enabled = true;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Disable_G()
{
  LSM6DSO_OpStatus op_status;

  /* Check if the component is already disabled */
  if (gyro_is_enabled == 0U)
  {
    return LSM6DSO_OK;
  }

  /* Get current output data rate. */
  op_status = LSM6DSO_Reg_Gy_Data_Rate_Get(&gyro_odr);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Output data rate selection - power down. */
  op_status = LSM6DSO_Reg_Gy_Data_Rate_Set(LSM6DSO_GY_ODR_OFF);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  gyro_is_enabled = false;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_G_Sensitivity(float *Sensitivity)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_FS_G full_scale;

  if(Sensitivity==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  /* Read actual full scale selection from sensor. */
  op_status = LSM6DSO_Reg_Gy_Full_Scale_Get(&full_scale);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Store the sensitivity based on actual full scale. */
  switch (full_scale)
  {
    case LSM6DSO_125dps:
      *Sensitivity = LSM6DSO_GYRO_SENSITIVITY_FS_125DPS;
      break;

    case LSM6DSO_250dps:
      *Sensitivity = LSM6DSO_GYRO_SENSITIVITY_FS_250DPS;
      break;

    case LSM6DSO_500dps:
      *Sensitivity = LSM6DSO_GYRO_SENSITIVITY_FS_500DPS;
      break;

    case LSM6DSO_1000dps:
      *Sensitivity = LSM6DSO_GYRO_SENSITIVITY_FS_1000DPS;
      break;

    case LSM6DSO_2000dps:
      *Sensitivity = LSM6DSO_GYRO_SENSITIVITY_FS_2000DPS;
      break;

    default:
      return LSM6DSO_InvalidParameter;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_G_ODR(float *Odr)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_ODR_G odr_low_level;

  if(Odr==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  /* Get current output data rate. */
  op_status = LSM6DSO_Reg_Gy_Data_Rate_Get(&odr_low_level);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  switch (odr_low_level)
  {
    case LSM6DSO_GY_ODR_OFF:
      *Odr = 0.0f;
      break;

    case LSM6DSO_GY_ODR_12Hz5:
      *Odr = 12.5f;
      break;

    case LSM6DSO_GY_ODR_26Hz:
      *Odr = 26.0f;
      break;

    case LSM6DSO_GY_ODR_52Hz:
      *Odr = 52.0f;
      break;

    case LSM6DSO_GY_ODR_104Hz:
      *Odr = 104.0f;
      break;

    case LSM6DSO_GY_ODR_208Hz:
      *Odr = 208.0f;
      break;

    case LSM6DSO_GY_ODR_417Hz:
      *Odr = 417.0f;
      break;

    case LSM6DSO_GY_ODR_833Hz:
      *Odr = 833.0f;
      break;

    case LSM6DSO_GY_ODR_1667Hz:
      *Odr =  1667.0f;
      break;

    case LSM6DSO_GY_ODR_3333Hz:
      *Odr =  3333.0f;
      break;

    case LSM6DSO_GY_ODR_6667Hz:
      *Odr =  6667.0f;
      break;

    default:
      return LSM6DSO_InvalidParameter;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_G_ODR(float Odr)
{
  return LSM6DSO_Set_G_ODR_With_Mode(Odr, LSM6DSO_GYRO_HIGH_PERFORMANCE_MODE);
}

LSM6DSO_OpStatus LSM6DSO_Set_G_ODR_With_Mode(float Odr, LSM6DSO_GYRO_Operating_Mode Mode)
{
  LSM6DSO_OpStatus op_status;

  switch (Mode)
  {
    case LSM6DSO_GYRO_HIGH_PERFORMANCE_MODE:
    {
      /* We must uncheck Low Power bit if it is enabled */
      LSM6DSO_Ctrl7_G val1;

      op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL7_G, (uint8_t *)&val1, 1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      if (val1.g_hm_mode)
      {
        val1.g_hm_mode = 0;

        op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL7_G, (uint8_t *)&val1, 1);
        if(op_status<LSM6DSO_OK)
        {
          return op_status;
        }
      }
      break;
    }
    case LSM6DSO_GYRO_LOW_POWER_NORMAL_MODE:
    {
      /* We must check the Low Power bit if it is unchecked */
      LSM6DSO_Ctrl7_G val1;

      op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL7_G, (uint8_t *)&val1, 1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      if (!val1.g_hm_mode)
      {
        val1.g_hm_mode = 1U;
        
        op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL7_G, (uint8_t *)&val1, 1);
        if(op_status<LSM6DSO_OK)
        {
          return op_status;
        }
      }

      /* Now we need to limit the ODR to 208 Hz if it is higher */
      if (Odr > 208.0f)
      {
        Odr = 208.0f;
      }
      break;
    }
    default:
      return LSM6DSO_InvalidParameter;
  }

  /* Check if the component is enabled */
  if (gyro_is_enabled == 1U)
  {
    return LSM6DSO_Set_G_ODR_When_Enabled(Odr);
  }
  else
  {
    return LSM6DSO_Set_G_ODR_When_Disabled(Odr);
  }
}

LSM6DSO_OpStatus LSM6DSO_Set_G_ODR_When_Enabled(float Odr)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_ODR_G new_odr;

  new_odr = (Odr <=   12.5f) ? LSM6DSO_GY_ODR_12Hz5
          : (Odr <=   26.0f) ? LSM6DSO_GY_ODR_26Hz
          : (Odr <=   52.0f) ? LSM6DSO_GY_ODR_52Hz
          : (Odr <=  104.0f) ? LSM6DSO_GY_ODR_104Hz
          : (Odr <=  208.0f) ? LSM6DSO_GY_ODR_208Hz
          : (Odr <=  417.0f) ? LSM6DSO_GY_ODR_417Hz
          : (Odr <=  833.0f) ? LSM6DSO_GY_ODR_833Hz
          : (Odr <= 1667.0f) ? LSM6DSO_GY_ODR_1667Hz
          : (Odr <= 3333.0f) ? LSM6DSO_GY_ODR_3333Hz
          :                    LSM6DSO_GY_ODR_6667Hz;

  /* Output data rate selection. */
  op_status = LSM6DSO_Reg_Gy_Data_Rate_Set(new_odr);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_G_ODR_When_Disabled(float Odr)
{
  gyro_odr = (Odr <=   12.5f) ? LSM6DSO_GY_ODR_12Hz5
           : (Odr <=   26.0f) ? LSM6DSO_GY_ODR_26Hz
           : (Odr <=   52.0f) ? LSM6DSO_GY_ODR_52Hz
           : (Odr <=  104.0f) ? LSM6DSO_GY_ODR_104Hz
           : (Odr <=  208.0f) ? LSM6DSO_GY_ODR_208Hz
           : (Odr <=  417.0f) ? LSM6DSO_GY_ODR_417Hz
           : (Odr <=  833.0f) ? LSM6DSO_GY_ODR_833Hz
           : (Odr <= 1667.0f) ? LSM6DSO_GY_ODR_1667Hz
           : (Odr <= 3333.0f) ? LSM6DSO_GY_ODR_3333Hz
           :                    LSM6DSO_GY_ODR_6667Hz;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_G_FS(int32_t *FullScale)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_FS_G fs_low_level;

  if(FullScale==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  /* Read actual full scale selection from sensor. */
  op_status = LSM6DSO_Reg_Gy_Full_Scale_Get(&fs_low_level);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  switch (fs_low_level)
  {
    case LSM6DSO_125dps:
      *FullScale =  125;
      break;

    case LSM6DSO_250dps:
      *FullScale =  250;
      break;

    case LSM6DSO_500dps:
      *FullScale =  500;
      break;

    case LSM6DSO_1000dps:
      *FullScale = 1000;
      break;

    case LSM6DSO_2000dps:
      *FullScale = 2000;
      break;

    default:
      return LSM6DSO_InvalidParameter;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_G_FS(int32_t FullScale)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_FS_G new_fs;

  new_fs = (FullScale <= 125)  ? LSM6DSO_125dps
         : (FullScale <= 250)  ? LSM6DSO_250dps
         : (FullScale <= 500)  ? LSM6DSO_500dps
         : (FullScale <= 1000) ? LSM6DSO_1000dps
         :                       LSM6DSO_2000dps;

  op_status = LSM6DSO_Reg_Gy_Full_Scale_Set(new_fs);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_G_AxesRaw(int16_t *rawX, int16_t *rawY, int16_t *rawZ)
{
  LSM6DSO_OpStatus op_status;
  axis3bit16_t data_raw;

  if(rawX==NULL || rawY==NULL || rawZ==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  /* Read raw data values. */
  op_status = LSM6DSO_Reg_Angular_Rate_Raw_Get(data_raw.u8bit);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Format the data. */
  *rawX = data_raw.i16bit[0];
  *rawY = data_raw.i16bit[1];
  *rawZ = data_raw.i16bit[2];

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_G_Axes(float *angularRateX, float *angularRateY, float *angularRateZ)
{
  LSM6DSO_OpStatus op_status;
  axis3bit16_t data_raw;
  float sensitivity;

  if(angularRateX==NULL || angularRateY==NULL || angularRateZ==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  /* Read raw data values. */
  op_status = LSM6DSO_Reg_Angular_Rate_Raw_Get(data_raw.u8bit);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Get LSM6DSO actual sensitivity. */
  op_status = LSM6DSO_Get_G_Sensitivity(&sensitivity);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Calculate the data. */
  *angularRateX = (float)((float)data_raw.i16bit[0] * sensitivity);
  *angularRateY = (float)((float)data_raw.i16bit[1] * sensitivity);
  *angularRateZ = (float)((float)data_raw.i16bit[2] * sensitivity);

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_Interrupt_Latch(uint8_t Status)
{
  LSM6DSO_OpStatus op_status;

  if (Status > 1U)
  {
    return LSM6DSO_InvalidParameter;
  }

  op_status = LSM6DSO_Reg_Int_Notification_Set((LSM6DSO_LIR)Status);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Enable_Free_Fall_Detection(LSM6DSO_SensorIntPin IntPin)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Pin_Int1_Route val1;
  LSM6DSO_Pin_Int2_Route val2;

  /* Output Data Rate selection */
  op_status = LSM6DSO_Set_X_ODR(416.0f);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Full scale selection */
  op_status = LSM6DSO_Set_X_FS(2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* FF_DUR setting */
  op_status = LSM6DSO_Reg_FF_Dur_Set(0x06);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* WAKE_DUR setting */
  op_status = LSM6DSO_Reg_WakeUp_Dur_Set(0x00);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* SLEEP_DUR setting */
  op_status = LSM6DSO_Reg_Act_Sleep_Dur_Set(0x00);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* FF_THS setting */
  op_status = LSM6DSO_Reg_FF_Threshold_Set(LSM6DSO_FF_TSH_312mg);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Enable free fall event on either INT1 or INT2 pin */
  switch (IntPin)
  {
    case LSM6DSO_INT1_PIN:
      op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&val1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      val1.free_fall = PROPERTY_ENABLE;

      op_status = LSM6DSO_Reg_Pin_Int1_Route_Set(val1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }
      break;

    case LSM6DSO_INT2_PIN:
      op_status = LSM6DSO_Reg_Pin_Int2_Route_Get(&val2);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      val2.free_fall = PROPERTY_ENABLE;

      op_status = LSM6DSO_Reg_Pin_Int2_Route_Set(val2);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }
      break;

    default:
      return LSM6DSO_InvalidParameter;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Disable_Free_Fall_Detection()
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Pin_Int1_Route val1;
  LSM6DSO_Pin_Int2_Route val2;

  /* Disable free fall event on both INT1 and INT2 pins */
  op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&val1);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  val1.free_fall = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Pin_Int1_Route_Set(val1);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Reg_Pin_Int2_Route_Get(&val2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  val2.free_fall = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Pin_Int2_Route_Set(val2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* FF_DUR setting */
  op_status = LSM6DSO_Reg_FF_Dur_Set(0x00);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* FF_THS setting */
  op_status = LSM6DSO_Reg_FF_Threshold_Set(LSM6DSO_FF_TSH_156mg);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_Free_Fall_Threshold(uint8_t Threshold)
{
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_FF_Threshold_Set((LSM6DSO_FF_Ths)Threshold);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_Free_Fall_Duration(uint8_t Duration)
{
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_FF_Dur_Set(Duration);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Enable_Pedometer()
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Pin_Int1_Route val;
  LSM6DSO_Emb_Sens emb_sens;

  /* Output Data Rate selection */
  op_status = LSM6DSO_Set_X_ODR(26.0f);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Full scale selection */
  op_status = LSM6DSO_Set_X_FS(2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Save current embedded features */
  op_status = LSM6DSO_Reg_Embedded_Sens_Get(&emb_sens);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Turn off embedded features */
  op_status = LSM6DSO_Reg_Embedded_Sens_Off();
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Wait for 10 ms */
  NeonRTOS_Sleep(10);

  /* Enable pedometer algorithm. */
  emb_sens.step = PROPERTY_ENABLE;

  op_status = LSM6DSO_Reg_Pedo_Sens_Set(LSM6DSO_PEDO_BASE_MODE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Turn on embedded features */
  op_status = LSM6DSO_Reg_Embedded_Sens_Set(&emb_sens);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Enable step detector on INT1 pin */
  op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&val);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  val.step_detector = PROPERTY_ENABLE;

  op_status = LSM6DSO_Reg_Pin_Int1_Route_Set(val);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Disable_Pedometer()
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Pin_Int1_Route val1;
  LSM6DSO_Emb_Sens emb_sens;

  /* Disable step detector on INT1 pin */
  op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&val1);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  val1.step_detector = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Pin_Int1_Route_Set(val1);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Save current embedded features */
  op_status = LSM6DSO_Reg_Embedded_Sens_Get(&emb_sens);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Disable pedometer algorithm. */
  emb_sens.step = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Embedded_Sens_Set(&emb_sens);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_Step_Count(uint16_t *StepCount)
{
  LSM6DSO_OpStatus op_status;
  
  if(StepCount==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Reg_Number_Of_Steps_Get((uint8_t *)StepCount);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Step_Counter_Reset()
{
  LSM6DSO_OpStatus op_status;
  
  op_status = LSM6DSO_Reg_Steps_Reset();
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Enable_Tilt_Detection(LSM6DSO_SensorIntPin IntPin)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Pin_Int1_Route val1;
  LSM6DSO_Pin_Int2_Route val2;
  LSM6DSO_Emb_Sens emb_sens;

  /* Output Data Rate selection */
  op_status = LSM6DSO_Set_X_ODR(26.0f);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Full scale selection */
  op_status = LSM6DSO_Set_X_FS(2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Save current embedded features */
  op_status = LSM6DSO_Reg_Embedded_Sens_Set(&emb_sens);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Turn off embedded features */
  op_status = LSM6DSO_Reg_Embedded_Sens_Off();
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Wait for 10 ms */
  NeonRTOS_Sleep(10);

  /* Enable tilt algorithm. */
  emb_sens.tilt = PROPERTY_ENABLE;

  /* Turn on embedded features */
  op_status = LSM6DSO_Reg_Embedded_Sens_Set(&emb_sens);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Enable tilt event on either INT1 or INT2 pin */
  switch (IntPin)
  {
    case LSM6DSO_INT1_PIN:
      op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&val1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      val1.tilt = PROPERTY_ENABLE;

      op_status = LSM6DSO_Reg_Pin_Int1_Route_Set(val1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }
      break;

    case LSM6DSO_INT2_PIN:
      op_status = LSM6DSO_Reg_Pin_Int2_Route_Get(&val2);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      val2.tilt = PROPERTY_ENABLE;

      op_status = LSM6DSO_Reg_Pin_Int2_Route_Set(val2);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }
      break;

    default:
      return LSM6DSO_InvalidParameter;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Disable_Tilt_Detection()
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Pin_Int1_Route val1;
  LSM6DSO_Pin_Int2_Route val2;
  LSM6DSO_Emb_Sens emb_sens;

  /* Disable tilt event on both INT1 and INT2 pins */
  op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&val1);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  val1.tilt = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Pin_Int1_Route_Set(val1);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Reg_Pin_Int2_Route_Get(&val2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  val2.tilt = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Pin_Int2_Route_Set(val2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Save current embedded features */
  op_status = LSM6DSO_Reg_Embedded_Sens_Get(&emb_sens);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Disable tilt algorithm. */
  emb_sens.tilt = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Embedded_Sens_Set(&emb_sens);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Enable_Wake_Up_Detection(LSM6DSO_SensorIntPin IntPin)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Pin_Int1_Route val1;
  LSM6DSO_Pin_Int2_Route val2;

  /* Output Data Rate selection */
  op_status = LSM6DSO_Set_X_ODR(26.0f);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Full scale selection */
  op_status = LSM6DSO_Set_X_FS(2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* WAKE_DUR setting */
  op_status = LSM6DSO_Reg_WakeUp_Dur_Set(0x00);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Set wake up threshold. */
  op_status = LSM6DSO_Reg_WakeUp_Threshold_Set(0x02);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Enable wake up event on either INT1 or INT2 pin */
  switch (IntPin)
  {
    case LSM6DSO_INT1_PIN:
      op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&val1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      val1.wake_up = PROPERTY_ENABLE;

      op_status = LSM6DSO_Reg_Pin_Int1_Route_Set(val1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }
      break;

    case LSM6DSO_INT2_PIN:
      op_status = LSM6DSO_Reg_Pin_Int2_Route_Get(&val2);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      val2.wake_up = PROPERTY_ENABLE;

      op_status = LSM6DSO_Reg_Pin_Int2_Route_Set(val2);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }
      break;

    default:
      return LSM6DSO_InvalidParameter;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Disable_Wake_Up_Detection()
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Pin_Int1_Route val1;
  LSM6DSO_Pin_Int2_Route val2;

  /* Disable wake up event on both INT1 and INT2 pins */
  op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&val1);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  val1.wake_up = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Pin_Int1_Route_Set(val1);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Reg_Pin_Int2_Route_Get(&val2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  val2.wake_up = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Pin_Int2_Route_Set(val2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Reset wake up threshold. */
  op_status = LSM6DSO_Reg_WakeUp_Threshold_Set(0x00);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* WAKE_DUR setting */
  op_status = LSM6DSO_Reg_WakeUp_Dur_Set(0x00);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_Wake_Up_Threshold(uint8_t Threshold)
{
  LSM6DSO_OpStatus op_status;

  /* Set wake up threshold. */
  op_status = LSM6DSO_Reg_WakeUp_Threshold_Set(Threshold);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_Wake_Up_Duration(uint8_t Duration)
{
  LSM6DSO_OpStatus op_status;

  /* Set wake up duration. */
  op_status = LSM6DSO_Reg_WakeUp_Dur_Set(Duration);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Enable_Single_Tap_Detection(LSM6DSO_SensorIntPin IntPin)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Pin_Int1_Route val1;
  LSM6DSO_Pin_Int2_Route val2;

  /* Output Data Rate selection */
  op_status = LSM6DSO_Set_X_ODR(416.0f);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Full scale selection */
  op_status = LSM6DSO_Set_X_FS(2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Enable X direction in tap recognition. */
  op_status = LSM6DSO_Reg_Tap_Detection_On_X_Set(PROPERTY_ENABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Enable Y direction in tap recognition. */
  op_status = LSM6DSO_Reg_Tap_Detection_On_Y_Set(PROPERTY_ENABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Enable Z direction in tap recognition. */
  op_status = LSM6DSO_Reg_Tap_Detection_On_Z_Set(PROPERTY_ENABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Set tap threshold. */
  op_status = LSM6DSO_Reg_Tap_Threshold_X_Set(0x08);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Set tap shock time window. */
  op_status = LSM6DSO_Reg_Tap_Shock_Set(0x02);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Set tap quiet time window. */
  op_status = LSM6DSO_Reg_Tap_Quiet_Set(0x01);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* _NOTE_: Tap duration time window - don't care for single tap. */

  /* _NOTE_: Single/Double Tap event - don't care of this flag for single tap. */

  /* Enable single tap event on either INT1 or INT2 pin */
  switch (IntPin)
  {
    case LSM6DSO_INT1_PIN:
      op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&val1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      val1.single_tap = PROPERTY_ENABLE;

      op_status = LSM6DSO_Reg_Pin_Int1_Route_Set(val1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }
      break;

    case LSM6DSO_INT2_PIN:
      op_status = LSM6DSO_Reg_Pin_Int2_Route_Get(&val2);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      val2.single_tap = PROPERTY_ENABLE;

      op_status = LSM6DSO_Reg_Pin_Int2_Route_Set(val2);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }
      break;

    default:
      return LSM6DSO_InvalidParameter;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Disable_Single_Tap_Detection()
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Pin_Int1_Route val1;
  LSM6DSO_Pin_Int2_Route val2;

  /* Disable single tap event on both INT1 and INT2 pins */
  op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&val1);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  val1.single_tap = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Pin_Int1_Route_Set(val1);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Reg_Pin_Int2_Route_Get(&val2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  val2.single_tap = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Pin_Int2_Route_Set(val2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Reset tap quiet time window. */
  op_status = LSM6DSO_Reg_Tap_Quiet_Set(0x00);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Reset tap shock time window. */
  op_status = LSM6DSO_Reg_Tap_Shock_Set(0x00);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Reset tap threshold. */
  op_status = LSM6DSO_Reg_Tap_Threshold_X_Set(0x00);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Disable Z direction in tap recognition. */
  op_status = LSM6DSO_Reg_Tap_Detection_On_Z_Set(PROPERTY_DISABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Disable Y direction in tap recognition. */
  op_status = LSM6DSO_Reg_Tap_Detection_On_Y_Set(PROPERTY_DISABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Disable X direction in tap recognition. */
  op_status = LSM6DSO_Reg_Tap_Detection_On_X_Set(PROPERTY_DISABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Enable_Double_Tap_Detection(LSM6DSO_SensorIntPin IntPin)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Pin_Int1_Route val1;
  LSM6DSO_Pin_Int2_Route val2;

  /* Output Data Rate selection */
  op_status = LSM6DSO_Set_X_ODR(416.0f);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Full scale selection */
  op_status = LSM6DSO_Set_X_FS(2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Enable X direction in tap recognition. */
  op_status = LSM6DSO_Reg_Tap_Detection_On_X_Set(PROPERTY_ENABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Enable Y direction in tap recognition. */
  op_status = LSM6DSO_Reg_Tap_Detection_On_Y_Set(PROPERTY_ENABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Enable Z direction in tap recognition. */
  op_status = LSM6DSO_Reg_Tap_Detection_On_Z_Set(PROPERTY_ENABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Set tap threshold. */
  op_status = LSM6DSO_Reg_Tap_Threshold_X_Set(0x08);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Set tap shock time window. */
  op_status = LSM6DSO_Reg_Tap_Shock_Set(0x03);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Set tap quiet time window. */
  op_status = LSM6DSO_Reg_Tap_Quiet_Set(0x03);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Set tap duration time window. */
  op_status = LSM6DSO_Reg_Tap_Dur_Set(0x08);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Single and double tap enabled. */
  op_status = LSM6DSO_Reg_Tap_Mode_Set(LSM6DSO_BOTH_SINGLE_DOUBLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Enable double tap event on either INT1 or INT2 pin */
  switch (IntPin)
  {
    case LSM6DSO_INT1_PIN:
      op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&val1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      val1.double_tap = PROPERTY_ENABLE;

      op_status = LSM6DSO_Reg_Pin_Int1_Route_Set(val1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }
      break;

    case LSM6DSO_INT2_PIN:
      op_status = LSM6DSO_Reg_Pin_Int2_Route_Get(&val2);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      val2.double_tap = PROPERTY_ENABLE;

      op_status = LSM6DSO_Reg_Pin_Int2_Route_Set(val2);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }
      break;

    default:
      return LSM6DSO_InvalidParameter;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Disable_Double_Tap_Detection()
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Pin_Int1_Route val1;
  LSM6DSO_Pin_Int2_Route val2;

  /* Disable double tap event on both INT1 and INT2 pins */
  op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&val1);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  val1.double_tap = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Pin_Int1_Route_Set(val1);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Reg_Pin_Int2_Route_Get(&val2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  val2.double_tap = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Pin_Int2_Route_Set(val2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Only single tap enabled. */
  op_status = LSM6DSO_Reg_Tap_Mode_Set(LSM6DSO_ONLY_SINGLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Reset tap duration time window. */
  op_status = LSM6DSO_Reg_Tap_Dur_Set(0x00);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Reset tap quiet time window. */
  op_status = LSM6DSO_Reg_Tap_Quiet_Set(0x00);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Reset tap shock time window. */
  op_status = LSM6DSO_Reg_Tap_Shock_Set(0x00);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Reset tap threshold. */
  op_status = LSM6DSO_Reg_Tap_Threshold_X_Set(0x00);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Disable Z direction in tap recognition. */
  op_status = LSM6DSO_Reg_Tap_Detection_On_Z_Set(PROPERTY_DISABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Disable Y direction in tap recognition. */
  op_status = LSM6DSO_Reg_Tap_Detection_On_Y_Set(PROPERTY_DISABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Disable X direction in tap recognition. */
  op_status = LSM6DSO_Reg_Tap_Detection_On_X_Set(PROPERTY_DISABLE);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_Tap_Threshold(uint8_t Threshold)
{
  LSM6DSO_OpStatus op_status;
  
  /* Set tap threshold. */
  op_status = LSM6DSO_Reg_Tap_Threshold_X_Set(Threshold);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_Tap_Shock_Time(uint8_t Time)
{
  LSM6DSO_OpStatus op_status;

  /* Set tap shock time window. */
  op_status = LSM6DSO_Reg_Tap_Shock_Set(Time);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_Tap_Quiet_Time(uint8_t Time)
{
  LSM6DSO_OpStatus op_status;

  /* Set tap quiet time window. */
  op_status = LSM6DSO_Reg_Tap_Quiet_Set(Time);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_Tap_Duration_Time(uint8_t Time)
{
  LSM6DSO_OpStatus op_status;

  /* Set tap duration time window. */
  op_status = LSM6DSO_Reg_Tap_Dur_Set(Time);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Enable_6D_Orientation(LSM6DSO_SensorIntPin IntPin)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Pin_Int1_Route val1;
  LSM6DSO_Pin_Int2_Route val2;

  /* Output Data Rate selection */
  op_status = LSM6DSO_Set_X_ODR(416.0f);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Full scale selection */
  op_status = LSM6DSO_Set_X_FS(2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* 6D orientation enabled. */
  op_status = LSM6DSO_Reg_6D_Threshold_Set(LSM6DSO_DEG_60);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  /* Enable 6D orientation event on either INT1 or INT2 pin */
  switch (IntPin)
  {
    case LSM6DSO_INT1_PIN:
      op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&val1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      val1.six_d = PROPERTY_ENABLE;

      op_status = LSM6DSO_Reg_Pin_Int1_Route_Set(val1);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }
      break;

    case LSM6DSO_INT2_PIN:
      op_status = LSM6DSO_Reg_Pin_Int2_Route_Get(&val2);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }

      val2.six_d = PROPERTY_ENABLE;

      op_status = LSM6DSO_Reg_Pin_Int2_Route_Set(val2);
      if(op_status<LSM6DSO_OK)
      {
        return op_status;
      }
      break;

    default:
      return LSM6DSO_InvalidParameter;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Disable_6D_Orientation()
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Pin_Int1_Route val1;
  LSM6DSO_Pin_Int2_Route val2;

  /* Disable 6D orientation event on both INT1 and INT2 pins */
  op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&val1);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  val1.six_d = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Pin_Int1_Route_Set(val1);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Reg_Pin_Int2_Route_Get(&val2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  val2.six_d = PROPERTY_DISABLE;

  op_status = LSM6DSO_Reg_Pin_Int2_Route_Set(val2);
  if(op_status<LSM6DSO_OK)
  {
    return op_status;
  }

  /* Reset 6D orientation. */
  op_status = LSM6DSO_Reg_6D_Threshold_Set(LSM6DSO_DEG_80);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_6D_Orientation_Threshold(uint8_t Threshold)
{
  LSM6DSO_OpStatus op_status;
  
  op_status = LSM6DSO_Reg_6D_Threshold_Set((LSM6DSO_6D_Ths)Threshold);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_6D_Orientation_XL(uint8_t *XLow)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_D6D_Src data;

  if(XLow==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_D6D_SRC, (uint8_t *)&data, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  *XLow = data.xl;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_6D_Orientation_XH(uint8_t *XHigh)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_D6D_Src data;

  if(XHigh==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_D6D_SRC, (uint8_t *)&data, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  *XHigh = data.xh;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_6D_Orientation_YL(uint8_t *YLow)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_D6D_Src data;

  if(YLow==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_D6D_SRC, (uint8_t *)&data, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  *YLow = data.yl;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_6D_Orientation_YH(uint8_t *YHigh)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_D6D_Src data;

  if(YHigh==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_D6D_SRC, (uint8_t *)&data, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  *YHigh = data.yh;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_6D_Orientation_ZL(uint8_t *ZLow)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_D6D_Src data;

  if(ZLow==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_D6D_SRC, (uint8_t *)&data, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  *ZLow = data.zl;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_6D_Orientation_ZH(uint8_t *ZHigh)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_D6D_Src data;

  if(ZHigh==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_D6D_SRC, (uint8_t *)&data, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  *ZHigh = data.zh;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_X_DRDY_Status(uint8_t *Status)
{
  LSM6DSO_OpStatus op_status;
  
  if(Status==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Reg_XL_Flag_Data_Ready_Get(Status);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_X_Event_Status(LSM6DSO_Event_Status *Status)
{
  uint8_t tilt_ia = 0U;
  LSM6DSO_Wake_Up_Src wake_up_src;
  LSM6DSO_Tap_Src tap_src;
  LSM6DSO_D6D_Src d6d_src;
  LSM6DSO_EMB_Func_Src func_src;
  LSM6DSO_MD1_Cfg md1_cfg;
  LSM6DSO_MD2_Cfg md2_cfg;
  LSM6DSO_EMB_Func_Int1 int1_ctrl;
  LSM6DSO_EMB_Func_Int2 int2_ctrl;
  LSM6DSO_OpStatus op_status;

  if(Status==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  (void)memset((void *)Status, 0x0, sizeof(LSM6DSO_Event_Status));

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_SRC, (uint8_t *)&wake_up_src, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_SRC, (uint8_t *)&tap_src, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_D6D_SRC, (uint8_t *)&d6d_src, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_SRC, (uint8_t *)&func_src, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_INT1, (uint8_t *)&int1_ctrl, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_INT2, (uint8_t *)&int2_ctrl, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_MD1_CFG, (uint8_t *)&md1_cfg, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_MD2_CFG, (uint8_t *)&md2_cfg, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  op_status = LSM6DSO_Reg_Tilt_Flag_Data_Ready_Get(&tilt_ia);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  if ((md1_cfg.int1_ff == 1U) || (md2_cfg.int2_ff == 1U))
  {
    if (wake_up_src.ff_ia == 1U)
    {
      Status->FreeFallStatus = 1;
    }
  }

  if ((md1_cfg.int1_wu == 1U) || (md2_cfg.int2_wu == 1U))
  {
    if (wake_up_src.wu_ia == 1U)
    {
      Status->WakeUpStatus = 1;
    }
  }

  if ((md1_cfg.int1_single_tap == 1U) || (md2_cfg.int2_single_tap == 1U))
  {
    if (tap_src.single_tap == 1U)
    {
      Status->TapStatus = 1;
    }
  }

  if ((md1_cfg.int1_double_tap == 1U) || (md2_cfg.int2_double_tap == 1U))
  {
    if (tap_src.double_tap == 1U)
    {
      Status->DoubleTapStatus = 1;
    }
  }

  if ((md1_cfg.int1_6d == 1U) || (md2_cfg.int2_6d == 1U))
  {
    if (d6d_src.d6d_ia == 1U)
    {
      Status->D6DOrientationStatus = 1;
    }
  }

  if (int1_ctrl.int1_step_detector == 1U)
  {
    if (func_src.step_detected == 1U)
    {
      Status->StepStatus = 1;
    }
  }

  if ((int1_ctrl.int1_tilt == 1U) || (int2_ctrl.int2_tilt == 1U))
  {
    if (tilt_ia == 1U)
    {
      Status->TiltStatus = 1;
    }
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_X_SelfTest(uint8_t val)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_ST_XL reg;

  reg = (val == 0U)  ? LSM6DSO_XL_ST_DISABLE
        : (val == 1U)  ? LSM6DSO_XL_ST_POSITIVE
        : (val == 2U)  ? LSM6DSO_XL_ST_NEGATIVE
        :                LSM6DSO_XL_ST_DISABLE;

  op_status = LSM6DSO_Reg_XL_Self_Test_Set(reg);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_G_DRDY_Status(uint8_t *Status)
{
  LSM6DSO_OpStatus op_status;
  
  if(Status==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Reg_Gy_Flag_Data_Ready_Get(Status);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_G_SelfTest(uint8_t val)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_ST_G reg;

  reg = (val == 0U)  ? LSM6DSO_GY_ST_DISABLE
        : (val == 1U)  ? LSM6DSO_GY_ST_POSITIVE
        : (val == 2U)  ? LSM6DSO_GY_ST_NEGATIVE
        :                LSM6DSO_GY_ST_DISABLE;

  op_status = LSM6DSO_Reg_Gy_Self_Test_Set(reg);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_FIFO_Num_Samples(uint16_t *NumSamples)
{
  LSM6DSO_OpStatus op_status;
  
  if(NumSamples==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Reg_FIFO_Data_Level_Get(NumSamples);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_FIFO_Full_Status(uint8_t *Status)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Reg reg;

  if(Status==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_STATUS2, &reg.byte, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  *Status = reg.fifo_status2.fifo_full_ia;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_FIFO_INT1_FIFO_Full(uint8_t Status)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Reg reg;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT1_CTRL, &reg.byte, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  reg.int1_ctrl.int1_fifo_full = Status;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_INT1_CTRL, &reg.byte, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_FIFO_INT2_FIFO_Full(uint8_t Status)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_Reg reg;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT2_CTRL, &reg.byte, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  reg.int2_ctrl.int2_fifo_full = Status;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_INT2_CTRL, &reg.byte, 1);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_FIFO_Watermark_Level(uint16_t Watermark)
{
  LSM6DSO_OpStatus op_status;
  
  op_status = LSM6DSO_Reg_FIFO_WaterMark_Set(Watermark);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_FIFO_Watermark_Level(uint16_t* Watermark)
{
  LSM6DSO_OpStatus op_status;
  
  if(Watermark==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Reg_FIFO_WaterMark_Get(Watermark);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_FIFO_Stop_On_Fth(uint8_t Status)
{
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_FIFO_Stop_On_WTM_Set(Status);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_FIFO_Mode(uint8_t Mode)
{
  LSM6DSO_OpStatus op_status;

  /* Verify that the passed parameter contains one of the valid values. */
  switch ((LSM6DSO_FIFO_Mode)Mode)
  {
    case LSM6DSO_BYPASS_MODE:
    case LSM6DSO_FIFO_MODE:
    case LSM6DSO_STREAM_TO_FIFO_MODE:
    case LSM6DSO_BYPASS_TO_STREAM_MODE:
    case LSM6DSO_STREAM_MODE:
      break;

    default:
      return LSM6DSO_InvalidParameter;
  }

  op_status = LSM6DSO_Reg_FIFO_Mode_Set((LSM6DSO_FIFO_Mode)Mode);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_FIFO_Tag(uint8_t *Tag)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_FIFO_Tag tag_local;

  if(Tag==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Reg_FIFO_Sensor_Tag_Get(&tag_local);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

	*Tag = (uint8_t)tag_local;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_FIFO_Data_Raw(uint8_t *Data)
{
  LSM6DSO_OpStatus op_status;

  if(Data==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_DATA_OUT_X_L, Data, 6);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_FIFO_X_Axes(float *accelerationX, float *accelerationY, float *accelerationZ)
{
  uint8_t data[6];
  int16_t data_raw[3];
  float sensitivity = 0.0f;
  float acceleration_float[3];
  LSM6DSO_OpStatus op_status;

  if(accelerationX==NULL || accelerationY==NULL || accelerationZ==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Get_FIFO_Data_Raw(data);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  data_raw[0] = ((int16_t)data[1] << 8) | data[0];
  data_raw[1] = ((int16_t)data[3] << 8) | data[2];
  data_raw[2] = ((int16_t)data[5] << 8) | data[4];

  op_status = LSM6DSO_Get_X_Sensitivity(&sensitivity);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  *accelerationX = (float)data_raw[0] * sensitivity;
  *accelerationY = (float)data_raw[1] * sensitivity;
  *accelerationZ = (float)data_raw[2] * sensitivity;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_FIFO_X_BDR(float Bdr)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_BDR_XL new_bdr;

  new_bdr = (Bdr <=    0.0f) ? LSM6DSO_XL_NOT_BATCHED
            : (Bdr <=   12.5f) ? LSM6DSO_XL_BATCHED_AT_12Hz5
            : (Bdr <=   26.0f) ? LSM6DSO_XL_BATCHED_AT_26Hz
            : (Bdr <=   52.0f) ? LSM6DSO_XL_BATCHED_AT_52Hz
            : (Bdr <=  104.0f) ? LSM6DSO_XL_BATCHED_AT_104Hz
            : (Bdr <=  208.0f) ? LSM6DSO_XL_BATCHED_AT_208Hz
            : (Bdr <=  416.0f) ? LSM6DSO_XL_BATCHED_AT_417Hz
            : (Bdr <=  833.0f) ? LSM6DSO_XL_BATCHED_AT_833Hz
            : (Bdr <= 1660.0f) ? LSM6DSO_XL_BATCHED_AT_1667Hz
            : (Bdr <= 3330.0f) ? LSM6DSO_XL_BATCHED_AT_3333Hz
            :                    LSM6DSO_XL_BATCHED_AT_6667Hz;

  op_status = LSM6DSO_Reg_FIFO_XL_Batch_Set(new_bdr);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Get_FIFO_G_Axes(float *angularVelocityX, float *angularVelocityY, float *angularVelocityZ)
{
  uint8_t data[6];
  int16_t data_raw[3];
  float sensitivity = 0.0f;
  LSM6DSO_OpStatus op_status;

  if(angularVelocityX==NULL || angularVelocityY==NULL || angularVelocityZ==NULL)
  {
    return LSM6DSO_InvalidParameter;
  }
  
  op_status = LSM6DSO_Get_FIFO_Data_Raw(data);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  data_raw[0] = ((int16_t)data[1] << 8) | data[0];
  data_raw[1] = ((int16_t)data[3] << 8) | data[2];
  data_raw[2] = ((int16_t)data[5] << 8) | data[4];

  op_status = LSM6DSO_Get_G_Sensitivity(&sensitivity);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  *angularVelocityX = (float)data_raw[0] * sensitivity;
  *angularVelocityY = (float)data_raw[1] * sensitivity;
  *angularVelocityZ = (float)data_raw[2] * sensitivity;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Set_FIFO_G_BDR(float Bdr)
{
  LSM6DSO_OpStatus op_status;
  LSM6DSO_BDR_Gy new_bdr;

  new_bdr = (Bdr <=    0.0f) ? LSM6DSO_GY_NOT_BATCHED
            : (Bdr <=   12.5f) ? LSM6DSO_GY_BATCHED_AT_12Hz5
            : (Bdr <=   26.0f) ? LSM6DSO_GY_BATCHED_AT_26Hz
            : (Bdr <=   52.0f) ? LSM6DSO_GY_BATCHED_AT_52Hz
            : (Bdr <=  104.0f) ? LSM6DSO_GY_BATCHED_AT_104Hz
            : (Bdr <=  208.0f) ? LSM6DSO_GY_BATCHED_AT_208Hz
            : (Bdr <=  416.0f) ? LSM6DSO_GY_BATCHED_AT_417Hz
            : (Bdr <=  833.0f) ? LSM6DSO_GY_BATCHED_AT_833Hz
            : (Bdr <= 1660.0f) ? LSM6DSO_GY_BATCHED_AT_1667Hz
            : (Bdr <= 3330.0f) ? LSM6DSO_GY_BATCHED_AT_3333Hz
            :                    LSM6DSO_GY_BATCHED_AT_6667Hz;

  op_status = LSM6DSO_Reg_FIFO_Gy_Batch_Set(new_bdr);
  if (op_status < LSM6DSO_OK)
  {
    return op_status;
  }

  return LSM6DSO_OK;
}
