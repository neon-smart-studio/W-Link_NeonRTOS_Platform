/**
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/*
 * Based on STMicroelectronics VL53L8CX driver
 * Modified by Neon Smart Studio for W-Link
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "VL53L8CX_Def.h"
#include "VL53L8CX_Global.h"
#include "VL53L8CX_Utils.h"
#include "VL53L8CX_IO.h"
#include "VL53L8CX.h"

#include "VL53L8CX_Xtalk.h"

/**
 * @brief Inner Macro for plugin. Not for user, only for development.
 */

#define VL53L8CX_DCI_CAL_CFG        ((uint16_t)0x5470U)
#define VL53L8CX_DCI_XTALK_CFG        ((uint16_t)0xAD94U)

static const uint8_t VL53L8CX_GET_XTALK_CMD[] = {
  0x54, 0x00, 0x00, 0x40,
  0x9F, 0xD8, 0x00, 0xC0,
  0x9F, 0xE4, 0x01, 0x40,
  0x9F, 0xF8, 0x00, 0x40,
  0x9F, 0xFC, 0x04, 0x04,
  0xA0, 0xFC, 0x01, 0x00,
  0xA1, 0x0C, 0x01, 0x00,
  0xA1, 0x1C, 0x00, 0xC0,
  0xA1, 0x28, 0x09, 0x02,
  0xA2, 0x48, 0x00, 0x40,
  0xA2, 0x4C, 0x00, 0x81,
  0xA2, 0x54, 0x00, 0x81,
  0xA2, 0x5C, 0x00, 0x81,
  0xA2, 0x64, 0x00, 0x81,
  0xA2, 0x6C, 0x00, 0x84,
  0xA2, 0x8C, 0x00, 0x82,
  0x00, 0x00, 0x00, 0x0F,
  0x07, 0x02, 0x00, 0x44
};

static VL53L8CX_OpResult VL53L8CX_Poll_For_Answer_Xtalk(uint16_t address, uint8_t expected_value)
{
  VL53L8CX_OpResult status;
  uint8_t timeout = 0;

  do {
    status = VL53L8CX_IO_Read_Bytes(address, temp_buffer, 4);
    if(status < VL53L8CX_OK)
    {
      return status;
    }

    NeonRTOS_Sleep(10);

    /* 2s timeout or FW error*/
    if ((timeout >= (uint8_t)200) || (temp_buffer[2] >= (uint8_t) 0x7f))
    {
      return VL53L8CX_MCU_Error;
    }
    else
    {
      timeout++;
    }
  } while ((temp_buffer[0x1]) != expected_value);

  return VL53L8CX_OK;
}

static VL53L8CX_OpResult VL53L8CX_Program_Output_Config()
{
  VL53L8CX_OpResult status;
  uint8_t resolution;
  uint32_t i;
  uint32_t header_config[2] = {0, 0};
  union Block_header *bh_ptr;

  status = VL53L8CX_Get_Resolution(&resolution);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  data_read_size = 0;

  /* Enable mandatory output (meta and common data) */
  uint32_t output_bh_enable[] = {
    0x0001FFFFU,
    0x00000000U,
    0x00000000U,
    0xC0000000U
  };

  /* Send addresses of possible output */
  uint32_t output[] = {
    0x0000000DU,
    0x54000040U,
    0x9FD800C0U,
    0x9FE40140U,
    0x9FF80040U,
    0x9FFC0404U,
    0xA0FC0100U,
    0xA10C0100U,
    0xA11C00C0U,
    0xA1280902U,
    0xA2480040U,
    0xA24C0081U,
    0xA2540081U,
    0xA25C0081U,
    0xA2640081U,
    0xA26C0084U,
    0xA28C0082U
  };

  /* Update data size */
  for (i = 0; i < (uint32_t)(sizeof(output) / sizeof(uint32_t)); i++)
  {
    if ((output[i] == (uint8_t)0) || ((output_bh_enable[i / (uint32_t)32] & ((uint32_t)1 << (i % (uint32_t)32))) == (uint32_t)0))
    {
      continue;
    }

    bh_ptr = (union Block_header *) & (output[i]);
    if (((uint8_t)bh_ptr->type >= (uint8_t)0x1) && ((uint8_t)bh_ptr->type < (uint8_t)0x0d))
    {
      if ((bh_ptr->idx >= (uint16_t)0x54d0) && (bh_ptr->idx < (uint16_t)(0x54d0 + 960)))
      {
        bh_ptr->size = resolution;
      }
      else
      {
        bh_ptr->size = (uint8_t)(resolution * (uint8_t)VL53L8CX_NB_TARGET_PER_ZONE);
      }

      data_read_size += bh_ptr->type * bh_ptr->size;
    }
    else
    {
      data_read_size += bh_ptr->size;
    }

    data_read_size += (uint32_t)4;
  }
  data_read_size += (uint32_t)24;

  status = VL53L8CX_DCI_Write_Data((uint8_t *) & (output), VL53L8CX_DCI_OUTPUT_LIST, (uint16_t)sizeof(output));
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  header_config[0] = data_read_size;
  header_config[1] = i + (uint32_t)1;

  status = VL53L8CX_DCI_Write_Data((uint8_t *) & (header_config), VL53L8CX_DCI_OUTPUT_CONFIG, (uint16_t)sizeof(header_config));
  if(status < VL53L8CX_OK)
  {
    return status;
  }
  status = VL53L8CX_DCI_Write_Data((uint8_t *) & (output_bh_enable), VL53L8CX_DCI_OUTPUT_ENABLES, (uint16_t)sizeof(output_bh_enable));
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  return VL53L8CX_OK;
}

VL53L8CX_OpResult VL53L8CX_Calibrate_Xtalk(uint16_t reflectance_percent, uint8_t nb_samples, uint16_t distance_mm)
{
  VL53L8CX_OpResult status;
  uint16_t timeout = 0;
  uint8_t cmd[] = {0x00, 0x03, 0x00, 0x00};
  uint8_t footer[] = {0x00, 0x00, 0x00, 0x0F, 0x00, 0x01, 0x03, 0x04};

  uint8_t resolution, frequency, target_order, sharp_prct, ranging_mode;
  uint32_t integration_time_ms, xtalk_margin;

  uint16_t reflectance = reflectance_percent;
  uint8_t samples = nb_samples;
  uint16_t distance = distance_mm;

  /* Get initial configuration */
  status = VL53L8CX_Get_Resolution(&resolution);
  if(status < VL53L8CX_OK)
  {
    return status;
  }
  status = VL53L8CX_Get_Ranging_Frequency_Hz(&frequency);
  if(status < VL53L8CX_OK)
  {
    return status;
  }
  status = VL53L8CX_Get_Integration_Time_mS(&integration_time_ms);
  if(status < VL53L8CX_OK)
  {
    return status;
  }
  status = VL53L8CX_Get_Sharpener_Percent(&sharp_prct);
  if(status < VL53L8CX_OK)
  {
    return status;
  }
  status = VL53L8CX_Get_Target_Order(&target_order);
  if(status < VL53L8CX_OK)
  {
    return status;
  }
  status = VL53L8CX_Get_Xtalk_Margin(&xtalk_margin);
  if(status < VL53L8CX_OK)
  {
    return status;
  }
  status = VL53L8CX_Get_Ranging_Mode(&ranging_mode);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  /* Check input arguments validity */
  if (((reflectance < (uint16_t)1) || (reflectance > (uint16_t)99)) || ((distance < (uint16_t)600) || (distance > (uint16_t)3000)) || ((samples < (uint8_t)1) || (samples > (uint8_t)16)))
  {
    return VL53L8CX_InvalidParameter;
  }

  status = VL53L8CX_Set_Resolution(VL53L8CX_RESOLUTION_8X8);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  /* Send Xtalk calibration buffer */
  (void)memcpy(temp_buffer, VL53L8CX_CALIBRATE_XTALK, VL53L8CX_XTALK_CALIBRATE_SIZE);
  
  status = VL53L8CX_IO_Write_Bytes(0x2c28, temp_buffer, VL53L8CX_XTALK_CALIBRATE_SIZE);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  status = VL53L8CX_Poll_For_Answer_Xtalk(VL53L8CX_UI_CMD_STATUS, 0x3);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  /* Format input argument */
  reflectance = reflectance * (uint16_t)16;
  distance = distance * (uint16_t)4;

  /* Update required fields */
  status = VL53L8CX_DCI_Replace_Data(temp_buffer, VL53L8CX_DCI_CAL_CFG, 8, (uint8_t *)&distance, 2, 0x00);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  status = VL53L8CX_DCI_Replace_Data(temp_buffer, VL53L8CX_DCI_CAL_CFG, 8, (uint8_t *)&reflectance, 2, 0x02);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  status = VL53L8CX_DCI_Replace_Data(temp_buffer, VL53L8CX_DCI_CAL_CFG, 8, (uint8_t *)&samples, 1, 0x04);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  /* Program output for Xtalk calibration */
  status = VL53L8CX_Program_Output_Config();
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  /* Start ranging session */
  status = VL53L8CX_IO_Write_Bytes(VL53L8CX_UI_CMD_END - (uint16_t)(4 - 1), (uint8_t *)cmd, sizeof(cmd));
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  status = VL53L8CX_Poll_For_Answer_Xtalk(VL53L8CX_UI_CMD_STATUS, 0x3);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  /* Wait for end of calibration */
  do {
    status = VL53L8CX_IO_Read_Bytes(0x0, temp_buffer, 4);
    if(status < VL53L8CX_OK)
    {
      return status;
    }

    if (temp_buffer[0] != VL53L8CX_STATUS_ERROR) {
      /* Coverglass too good for Xtalk calibration */
      if ((temp_buffer[2] >= (uint8_t)0x7f) && (((uint16_t)(temp_buffer[3] & (uint16_t)0x80) >> 7) == (uint16_t)1))
      {
        (void)memcpy(xtalk_data, VL53L8CX_DEFAULT_XTALK, sizeof(xtalk_data));
        return VL53L8CX_Xtalk_Failed;
      }
      break;
    }
    else if (timeout >= (uint16_t)400)
    {
      return VL53L8CX_Status_Error;
    }
    else
    {
      timeout++;
      NeonRTOS_Sleep(50);
    }
  } while (1);

  /* Save Xtalk data into the Xtalk buffer */
  (void)memcpy(temp_buffer, VL53L8CX_GET_XTALK_CMD, sizeof(VL53L8CX_GET_XTALK_CMD));
  
  status = VL53L8CX_IO_Write_Bytes(0x2fb8, temp_buffer, (uint16_t)sizeof(VL53L8CX_GET_XTALK_CMD));
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  status = VL53L8CX_Poll_For_Answer_Xtalk(VL53L8CX_UI_CMD_STATUS, 0x03);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  status = VL53L8CX_IO_Read_Bytes(VL53L8CX_UI_CMD_START, temp_buffer, VL53L8CX_XTALK_BUFFER_SIZE + (uint16_t)4);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  (void)memcpy(xtalk_data, &(temp_buffer[8]), VL53L8CX_XTALK_BUFFER_SIZE - (uint16_t)8);
  (void)memcpy(&(xtalk_data[VL53L8CX_XTALK_BUFFER_SIZE - (uint16_t)8]), footer, sizeof(footer));

  /* Reset default buffer */
  status = VL53L8CX_IO_Write_Bytes(0x2c34, VL53L8CX_DEFAULT_CONFIGURATION, VL53L8CX_CONFIGURATION_SIZE);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  status = VL53L8CX_Poll_For_Answer_Xtalk(VL53L8CX_UI_CMD_STATUS, 0x03);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  /* Reset initial configuration */
  status = VL53L8CX_Set_Resolution(resolution);
  if(status < VL53L8CX_OK)
  {
    return status;
  }
  status = VL53L8CX_Set_Ranging_Frequency_Hz(frequency);
  if(status < VL53L8CX_OK)
  {
    return status;
  }
  status = VL53L8CX_Set_integration_Time_mS(integration_time_ms);
  if(status < VL53L8CX_OK)
  {
    return status;
  }
  status = VL53L8CX_Set_Sharpener_Percent(sharp_prct);
  if(status < VL53L8CX_OK)
  {
    return status;
  }
  status = VL53L8CX_Set_Target_Order(target_order);
  if(status < VL53L8CX_OK)
  {
    return status;
  }
  status = VL53L8CX_Set_Xtalk_Margin(xtalk_margin);
  if(status < VL53L8CX_OK)
  {
    return status;
  }
  status = VL53L8CX_Set_Ranging_Mode(ranging_mode);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  return VL53L8CX_OK;
}

VL53L8CX_OpResult VL53L8CX_Get_Caldata_Xtalk(uint8_t *p_xtalk_data)
{
  VL53L8CX_OpResult status;
  uint8_t resolution;
  uint8_t footer[] = {0x00, 0x00, 0x00, 0x0F, 0x00, 0x01, 0x03, 0x04};

  status = VL53L8CX_Get_Resolution(&resolution);
  if(status < VL53L8CX_OK)
  {
    return status;
  }
  status = VL53L8CX_Set_Resolution(VL53L8CX_RESOLUTION_8X8);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  (void)memcpy(temp_buffer, VL53L8CX_GET_XTALK_CMD, sizeof(VL53L8CX_GET_XTALK_CMD));

  status = VL53L8CX_IO_Write_Bytes(0x2fb8, temp_buffer,  sizeof(VL53L8CX_GET_XTALK_CMD));
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  status = VL53L8CX_Poll_For_Answer(VL53L8CX_UI_CMD_STATUS, 0x03);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  status = VL53L8CX_IO_Read_Bytes(VL53L8CX_UI_CMD_START, temp_buffer, VL53L8CX_XTALK_BUFFER_SIZE + (uint16_t)4);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  (void)memcpy(&(p_xtalk_data[0]), &(temp_buffer[8]), VL53L8CX_XTALK_BUFFER_SIZE - (uint16_t)8);
  (void)memcpy(&(p_xtalk_data[VL53L8CX_XTALK_BUFFER_SIZE - (uint16_t)8]), footer, sizeof(footer));

  status = VL53L8CX_Set_Resolution(resolution);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  return VL53L8CX_OK;
}

VL53L8CX_OpResult VL53L8CX_Set_Caldata_Xtalk(uint8_t *p_xtalk_data)
{
  VL53L8CX_OpResult status;
  uint8_t resolution;

  status = VL53L8CX_Get_Resolution(&resolution);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  (void)memcpy(xtalk_data, p_xtalk_data, VL53L8CX_XTALK_BUFFER_SIZE);

  status = VL53L8CX_Set_Resolution(resolution);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  return VL53L8CX_OK;
}

VL53L8CX_OpResult VL53L8CX_Get_Xtalk_Margin(uint32_t *p_xtalk_margin)
{
  VL53L8CX_OpResult status;

  status = VL53L8CX_DCI_Read_Data((uint8_t *)temp_buffer, VL53L8CX_DCI_XTALK_CFG, 16);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  (void)memcpy(p_xtalk_margin, temp_buffer, 4);

  *p_xtalk_margin = *p_xtalk_margin / (uint32_t)2048;

  return VL53L8CX_OK;
}

VL53L8CX_OpResult VL53L8CX_Set_Xtalk_Margin(uint32_t xtalk_margin)
{
  VL53L8CX_OpResult status;
  uint32_t margin_kcps = xtalk_margin;

  if (margin_kcps > (uint32_t)10000)
  {
    return VL53L8CX_InvalidParameter;
  }

  margin_kcps = margin_kcps * (uint32_t)2048;

  status = VL53L8CX_DCI_Replace_Data(temp_buffer, VL53L8CX_DCI_XTALK_CFG, 16, (uint8_t *)&margin_kcps, 4, 0x00);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  return VL53L8CX_OK;
}
