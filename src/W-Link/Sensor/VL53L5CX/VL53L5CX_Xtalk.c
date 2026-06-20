/**
 ******************************************************************************
 * @file    VL53L5CX_plugin_xtalk.cpp
 * @author  STMicroelectronics
 * @version V1.0.0
 * @date    11 November 2021
 * @brief   Implementation of the VL53L5CX APIs for xtalk calibration.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2021 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/*
 * Based on STMicroelectronics VL53L5CX driver
 * Modified by Neon Smart Studio for W-Link
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "VL53L5CX_Def.h"
#include "VL53L5CX_Global.h"
#include "VL53L5CX_Utils.h"
#include "VL53L5CX_IO.h"
#include "VL53L5CX.h"

#include "VL53L5CX_Xtalk.h"

/**
 * @brief Command used to get Xtalk calibration data
 */

static const uint8_t VL53L5CX_GET_XTALK_CMD[] = {
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

/**
 * @brief Inner Macro for plugin. Not for user, only for development.
 */

#define VL53L5CX_DCI_CAL_CFG        ((uint16_t)0x5470U)
#define VL53L5CX_DCI_XTALK_CFG        ((uint16_t)0xAD94U)

static VL53L5CX_OpResult VL53L5CX_Poll_For_Answer_Xtalk(uint16_t address, uint8_t expected_value)
{
  VL53L5CX_OpResult status;
  uint8_t timeout = 0;

  do {
    status = VL53L5CX_IO_Read_Bytes(address, VL53L5CX_Temp_Buffer, 4);
    if(status < VL53L5CX_OK)
    {
      return status;
    }

    NeonRTOS_Sleep(10);

    /* 2s timeout or FW error*/
    if ((timeout >= (uint8_t)200) || (VL53L5CX_Temp_Buffer[2] >= (uint8_t) 0x7f)) {
      return VL53L5CX_MCU_Error;
    } else {
      timeout++;
    }
  } while ((VL53L5CX_Temp_Buffer[0x1]) != expected_value);

  return VL53L5CX_OK;
}

/*
 * Inner function, not available outside this file. This function is used to
 * program the output using the macro defined into the 'platform.h' file.
 */

static VL53L5CX_OpResult VL53L5CX_Program_Output_Config()
{
  VL53L5CX_OpResult status;
  uint8_t resolution;
  uint32_t i;
  uint64_t header_config;
  union Block_header *bh_ptr;

  status = VL53L5CX_Get_Resolution(&resolution);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  VL53L5CX_Data_Read_Size = 0;

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
  for (i = 0; i < (uint32_t)(sizeof(output) / sizeof(uint32_t)); i++) {
    if ((output[i] == (uint8_t)0) || ((output_bh_enable[i / (uint32_t)32] & ((uint32_t)1 << (i % (uint32_t)32))) == (uint32_t)0)) {
      continue;
    }

    bh_ptr = (union Block_header *) & (output[i]);
    if (((uint8_t)bh_ptr->type >= (uint8_t)0x1) && ((uint8_t)bh_ptr->type < (uint8_t)0x0d)) {
      if ((bh_ptr->idx >= (uint16_t)0x54d0) && (bh_ptr->idx < (uint16_t)(0x54d0 + 960))) {
        bh_ptr->size = resolution;
      } else {
        bh_ptr->size = (uint8_t)(resolution
                                 * (uint8_t)VL53L5CX_NB_TARGET_PER_ZONE);
      }

      VL53L5CX_Data_Read_Size += bh_ptr->type * bh_ptr->size;
    } else {
      VL53L5CX_Data_Read_Size += bh_ptr->size;
    }

    VL53L5CX_Data_Read_Size += (uint32_t)4;
  }
  VL53L5CX_Data_Read_Size += (uint32_t)20;

  status = VL53L5CX_DCI_Write_Data((uint8_t *) & (output), VL53L5CX_DCI_OUTPUT_LIST, (uint16_t)sizeof(output));
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  header_config = (uint64_t)i + (uint64_t)1;
  header_config = header_config << 32;
  header_config += (uint64_t)VL53L5CX_Data_Read_Size;

  status = VL53L5CX_DCI_Write_Data((uint8_t *) & (header_config), VL53L5CX_DCI_OUTPUT_CONFIG, (uint16_t)sizeof(header_config));
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  status = VL53L5CX_DCI_Write_Data((uint8_t *) & (output_bh_enable), VL53L5CX_DCI_OUTPUT_ENABLES, (uint16_t)sizeof(output_bh_enable));
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Calibrate_Xtalk(uint16_t reflectance_percent, uint8_t nb_samples, uint16_t distance_mm)
{
  VL53L5CX_OpResult status;
  uint16_t timeout = 0;
  uint8_t cmd[] = {0x00, 0x03, 0x00, 0x00};
  uint8_t footer[] = {0x00, 0x00, 0x00, 0x0F, 0x00, 0x01, 0x03, 0x04};

  uint8_t resolution, frequency, target_order, sharp_prct, ranging_mode;
  uint32_t integration_time_ms, xtalk_margin;

  uint16_t reflectance = reflectance_percent;
  uint8_t samples = nb_samples;
  uint16_t distance = distance_mm;

  /* Get initial configuration */
  status = VL53L5CX_Get_Resolution(&resolution);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Get_Ranging_Frequency_Hz(&frequency);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Get_Integration_Time_mS(&integration_time_ms);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Get_Sharpener_Percent(&sharp_prct);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Get_Target_Order(&target_order);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Get_Xtalk_Margin(&xtalk_margin);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Get_Ranging_Mode(&ranging_mode);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  /* Check input arguments validity */
  if (((reflectance < (uint16_t)1) || (reflectance > (uint16_t)99)) || ((distance < (uint16_t)600) || (distance > (uint16_t)3000)) || ((samples < (uint8_t)1) || (samples > (uint8_t)16)))
  {
    return VL53L5CX_InvalidParameter;
  }

  status = VL53L5CX_Set_Resolution(VL53L5CX_RESOLUTION_8X8);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  /* Send Xtalk calibration buffer */
  status = VL53L5CX_IO_Write_Bytes(0x2c28, VL53L5CX_CALIBRATE_XTALK, VL53L5CX_XTALK_CALIBRATE_SIZE);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Poll_For_Answer_Xtalk(VL53L5CX_UI_CMD_STATUS, 0x3);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  /* Format input argument */
  reflectance = reflectance * (uint16_t)16;
  distance = distance * (uint16_t)4;

  /* Update required fields */
  status = VL53L5CX_DCI_Replace_Data(VL53L5CX_Temp_Buffer, VL53L5CX_DCI_CAL_CFG, 8, (uint8_t *)&distance, 2, 0x00);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  status = VL53L5CX_DCI_Replace_Data(VL53L5CX_Temp_Buffer, VL53L5CX_DCI_CAL_CFG, 8, (uint8_t *)&reflectance, 2, 0x02);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  status = VL53L5CX_DCI_Replace_Data(VL53L5CX_Temp_Buffer, VL53L5CX_DCI_CAL_CFG, 8, (uint8_t *)&samples, 1, 0x04);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  /* Program output for Xtalk calibration */
  status = VL53L5CX_Program_Output_Config();
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  /* Start ranging session */
  status = VL53L5CX_IO_Write_Bytes(VL53L5CX_UI_CMD_END - (uint16_t)(4 - 1), (uint8_t *)cmd, sizeof(cmd));
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Poll_For_Answer_Xtalk(VL53L5CX_UI_CMD_STATUS, 0x3);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  /* Wait for end of calibration */
  do {
    status = VL53L5CX_IO_Read_Bytes(0x0, VL53L5CX_Temp_Buffer, 4);
    if(status < VL53L5CX_OK)
    {
      return status;
    }

    if (VL53L5CX_Temp_Buffer[0] != VL53L5CX_STATUS_ERROR)
    {
      /* Coverglass too good for Xtalk calibration */
      if ((VL53L5CX_Temp_Buffer[2] >= (uint8_t)0x7f) && (((uint16_t)(VL53L5CX_Temp_Buffer[3] & (uint16_t)0x80) >> 7) == (uint16_t)1)) {
        (void)memcpy(VL53L5CX_Xtalk_Data, VL53L5CX_DEFAULT_XTALK, VL53L5CX_XTALK_BUFFER_SIZE);
      }
      break;
    }
    else if (timeout >= (uint16_t)400)
    {
      return VL53L5CX_Status_Error;
    }
    else
    {
      timeout++;
      NeonRTOS_Sleep(50);
    }

  } while (1);

  /* Save Xtalk data into the Xtalk buffer */
  status = VL53L5CX_IO_Write_Bytes(0x2fb8, VL53L5CX_GET_XTALK_CMD, (uint16_t)sizeof(VL53L5CX_GET_XTALK_CMD));
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  status = VL53L5CX_Poll_For_Answer_Xtalk(VL53L5CX_UI_CMD_STATUS, 0x03);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  status = VL53L5CX_IO_Read_Bytes(VL53L5CX_UI_CMD_START, VL53L5CX_Temp_Buffer, VL53L5CX_XTALK_BUFFER_SIZE + (uint16_t)4);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  (void)memcpy(&(VL53L5CX_Xtalk_Data[0]), &(VL53L5CX_Temp_Buffer[8]), VL53L5CX_XTALK_BUFFER_SIZE - (uint16_t)8);
  (void)memcpy(&(VL53L5CX_Xtalk_Data[VL53L5CX_XTALK_BUFFER_SIZE - (uint16_t)8]), footer, sizeof(footer));

  /* Reset default buffer */
  status = VL53L5CX_IO_Write_Bytes(0x2c34, VL53L5CX_DEFAULT_CONFIGURATION, VL53L5CX_CONFIGURATION_SIZE);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Poll_For_Answer_Xtalk(VL53L5CX_UI_CMD_STATUS, 0x03);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  /* Reset initial configuration */
  status = VL53L5CX_Set_Resolution(resolution);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Set_Ranging_Frequency_Hz(frequency);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Set_Integration_Time_mS(integration_time_ms);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Set_Sharpener_Percent(sharp_prct);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Set_Target_Order(target_order);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Set_Xtalk_Margin(xtalk_margin);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Set_Ranging_Mode(ranging_mode);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return status;
}

VL53L5CX_OpResult VL53L5CX_Get_Caldata_Xtalk(uint8_t *p_VL53L5CX_Xtalk_Data)
{
  VL53L5CX_OpResult status;
  uint8_t resolution;
  uint8_t footer[] = {0x00, 0x00, 0x00, 0x0F, 0x00, 0x01, 0x03, 0x04};

  status = VL53L5CX_Get_Resolution(&resolution);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Set_Resolution(VL53L5CX_RESOLUTION_8X8);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  status = VL53L5CX_IO_Write_Bytes(0x2fb8, VL53L5CX_GET_XTALK_CMD, sizeof(VL53L5CX_GET_XTALK_CMD));
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Poll_For_Answer_Xtalk(VL53L5CX_UI_CMD_STATUS, 0x03);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_IO_Read_Bytes(VL53L5CX_UI_CMD_START, VL53L5CX_Temp_Buffer, VL53L5CX_XTALK_BUFFER_SIZE + (uint16_t)4);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  (void)memcpy(&(p_VL53L5CX_Xtalk_Data[0]), &(VL53L5CX_Temp_Buffer[8]), VL53L5CX_XTALK_BUFFER_SIZE - (uint16_t)8);
  (void)memcpy(&(p_VL53L5CX_Xtalk_Data[VL53L5CX_XTALK_BUFFER_SIZE - (uint16_t)8]), footer, sizeof(footer));

  status = VL53L5CX_Set_Resolution(resolution);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return status;
}

VL53L5CX_OpResult VL53L5CX_Set_Caldata_Xtalk(uint8_t *p_VL53L5CX_Xtalk_Data)
{
  VL53L5CX_OpResult status;
  uint8_t resolution;

  status = VL53L5CX_Get_Resolution(&resolution);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  (void)memcpy(VL53L5CX_Xtalk_Data, p_VL53L5CX_Xtalk_Data, VL53L5CX_XTALK_BUFFER_SIZE);

  status = VL53L5CX_Set_Resolution(resolution);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Get_Xtalk_Margin(uint32_t *p_xtalk_margin)
{
  VL53L5CX_OpResult status;

  status = VL53L5CX_DCI_Read_Data((uint8_t *)VL53L5CX_Temp_Buffer, VL53L5CX_DCI_XTALK_CFG, 16);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  (void)memcpy(p_xtalk_margin, VL53L5CX_Temp_Buffer, 4);
  *p_xtalk_margin = *p_xtalk_margin / (uint32_t)2048;

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_set_xtalk_margin(uint32_t xtalk_margin)
{
  VL53L5CX_OpResult status;
  uint32_t margin_kcps = xtalk_margin;

  if (margin_kcps > (uint32_t)10000)
  {
    return VL53L5CX_InvalidParameter;
  }

  margin_kcps = margin_kcps * (uint32_t)2048;

  status = VL53L5CX_DCI_Replace_Data(VL53L5CX_Temp_Buffer, VL53L5CX_DCI_XTALK_CFG, 16, (uint8_t *)&margin_kcps, 4, 0x00);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}
