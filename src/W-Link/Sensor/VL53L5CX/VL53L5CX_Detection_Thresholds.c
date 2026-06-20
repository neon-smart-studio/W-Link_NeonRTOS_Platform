/**
 ******************************************************************************
 * @file    vl53l5cx_plugin_detection_thresholds.cpp
 * @author  STMicroelectronics
 * @version V1.0.0
 * @date    11 November 2021
 * @brief   Implementation of the VL53L5CX APIs for thresholds detection.
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

#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "VL53L5CX_Def.h"
#include "VL53L5CX_Global.h"
#include "VL53L5CX_Utils.h"
#include "VL53L5CX_IO.h"
#include "VL53L5CX.h"

#include "VL53L5CX_Detection_Thresholds.h"

VL53L5CX_OpResult VL53L5CX_Get_Detection_Thresholds_Enable(uint8_t *p_enabled)
{
  VL53L5CX_OpResult status;

  status = VL53L5CX_DCI_Read_Data((uint8_t *)VL53L5CX_Temp_Buffer, VL53L5CX_DCI_DET_THRESH_GLOBAL_CONFIG, 8);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  *p_enabled = VL53L5CX_Temp_Buffer[0x1];

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Set_Detection_Thresholds_Enable(uint8_t enabled)
{
  VL53L5CX_OpResult status;
  uint8_t tmp;
  uint8_t grp_global_config[] = {0x01, 0x00, 0x01, 0x00};

  if (enabled == (uint8_t)1) {
    grp_global_config[0x01] = 0x01;
    tmp = 0x04;
  } else {
    grp_global_config[0x01] = 0x00;
    tmp = 0x0C;
  }

  /* Set global interrupt config */
  status = VL53L5CX_DCI_Replace_Data(VL53L5CX_Temp_Buffer, VL53L5CX_DCI_DET_THRESH_GLOBAL_CONFIG, 8, (uint8_t *)&grp_global_config, 4, 0x00);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  /* Update interrupt config */
  status = VL53L5CX_DCI_Replace_Data(VL53L5CX_Temp_Buffer, VL53L5CX_DCI_DET_THRESH_CONFIG, 20, (uint8_t *)&tmp, 1, 0x11);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Get_Detection_Thresholds(VL53L5CX_DetectionThresholds *p_thresholds)
{
  VL53L5CX_OpResult status;
  uint8_t i;

  /* Get thresholds configuration */
  status = VL53L5CX_DCI_Read_Data((uint8_t *)p_thresholds, VL53L5CX_DCI_DET_THRESH_START, (uint16_t)VL53L5CX_NB_THRESHOLDS * (uint16_t)sizeof(VL53L5CX_DetectionThresholds));
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  for (i = 0; i < (uint8_t)VL53L5CX_NB_THRESHOLDS; i++) {
    switch (p_thresholds[i].measurement) {
      case VL53L5CX_DISTANCE_MM:
        p_thresholds[i].param_low_thresh  /= 4;
        p_thresholds[i].param_high_thresh /= 4;
        break;
      case VL53L5CX_SIGNAL_PER_SPAD_KCPS:
        p_thresholds[i].param_low_thresh  /= 2048;
        p_thresholds[i].param_high_thresh /= 2048;
        break;
      case VL53L5CX_RANGE_SIGMA_MM:
        p_thresholds[i].param_low_thresh  /= 128;
        p_thresholds[i].param_high_thresh /= 128;
        break;
      case VL53L5CX_AMBIENT_PER_SPAD_KCPS:
        p_thresholds[i].param_low_thresh  /= 2048;
        p_thresholds[i].param_high_thresh /= 2048;
        break;
      case VL53L5CX_NB_SPADS_ENABLED:
        p_thresholds[i].param_low_thresh  /= 256;
        p_thresholds[i].param_high_thresh /= 256;
        break;
      case VL53L5CX_MOTION_INDICATOR:
        p_thresholds[i].param_low_thresh  /= 65535;
        p_thresholds[i].param_high_thresh /= 65535;
        break;
      default:
        break;
    }
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Set_Detection_Thresholds(VL53L5CX_DetectionThresholds *p_thresholds)
{
  VL53L5CX_OpResult status;
  uint8_t i;
  uint8_t grp_valid_target_cfg[] = {0x05, 0x05, 0x05, 0x05,
                                    0x05, 0x05, 0x05, 0x05
                                   };

  for (i = 0; i < (uint8_t) VL53L5CX_NB_THRESHOLDS; i++) {
    switch (p_thresholds[i].measurement) {
      case VL53L5CX_DISTANCE_MM:
        p_thresholds[i].param_low_thresh  *= 4;
        p_thresholds[i].param_high_thresh *= 4;
        break;
      case VL53L5CX_SIGNAL_PER_SPAD_KCPS:
        p_thresholds[i].param_low_thresh  *= 2048;
        p_thresholds[i].param_high_thresh *= 2048;
        break;
      case VL53L5CX_RANGE_SIGMA_MM:
        p_thresholds[i].param_low_thresh  *= 128;
        p_thresholds[i].param_high_thresh *= 128;
        break;
      case VL53L5CX_AMBIENT_PER_SPAD_KCPS:
        p_thresholds[i].param_low_thresh  *= 2048;
        p_thresholds[i].param_high_thresh *= 2048;
        break;
      case VL53L5CX_NB_SPADS_ENABLED:
        p_thresholds[i].param_low_thresh  *= 256;
        p_thresholds[i].param_high_thresh *= 256;
        break;
      case VL53L5CX_MOTION_INDICATOR:
        p_thresholds[i].param_low_thresh  *= 65535;
        p_thresholds[i].param_high_thresh *= 65535;
        break;
      default:
        break;
    }
  }

  /* Set valid target list */
  status = VL53L5CX_DCI_Write_Data((uint8_t *)grp_valid_target_cfg, VL53L5CX_DCI_DET_THRESH_VALID_STATUS, (uint16_t)sizeof(grp_valid_target_cfg));
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  /* Set thresholds configuration */
  status = VL53L5CX_DCI_Write_Data((uint8_t *)p_thresholds, VL53L5CX_DCI_DET_THRESH_START, (uint16_t)(VL53L5CX_NB_THRESHOLDS * sizeof(VL53L5CX_DetectionThresholds)));
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}
