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

#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "VL53L8CX_Def.h"
#include "VL53L8CX_Global.h"
#include "VL53L8CX_Utils.h"
#include "VL53L8CX_IO.h"
#include "VL53L8CX.h"

#include "VL53L8CX_Detection_Thresholds.h"

VL53L8CX_OpResult VL53L8CX_Get_Detection_Thresholds_Enable(uint8_t *p_enabled)
{
  VL53L8CX_OpResult status;

  status = VL53L8CX_DCI_Read_Data((uint8_t *)temp_buffer, VL53L8CX_DCI_DET_THRESH_GLOBAL_CONFIG, 8);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  *p_enabled = temp_buffer[0x1];

  return VL53L8CX_OK;
}

VL53L8CX_OpResult VL53L8CX_Set_Detection_Thresholds_Enable(uint8_t enabled)
{
  VL53L8CX_OpResult status;
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
  status = VL53L8CX_DCI_Replace_Data(temp_buffer, VL53L8CX_DCI_DET_THRESH_GLOBAL_CONFIG, 8, (uint8_t *)&grp_global_config, 4, 0x00);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  /* Update interrupt config */
  status = VL53L8CX_DCI_Replace_Data(temp_buffer, VL53L8CX_DCI_DET_THRESH_CONFIG, 20, (uint8_t *)&tmp, 1, 0x11);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  return VL53L8CX_OK;
}

VL53L8CX_OpResult VL53L8CX_Get_Detection_Thresholds(VL53L8CX_DetectionThresholds *p_thresholds)
{
  VL53L8CX_OpResult status;
  uint8_t i;

  /* Get thresholds configuration */
  status = VL53L8CX_DCI_Read_Data((uint8_t *)p_thresholds,  VL53L8CX_DCI_DET_THRESH_START, (uint16_t)VL53L8CX_NB_THRESHOLDS * (uint16_t)sizeof(VL53L8CX_DetectionThresholds));
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  for (i = 0; i < (uint8_t)VL53L8CX_NB_THRESHOLDS; i++) {
    switch (p_thresholds[i].measurement) {
      case VL53L8CX_DISTANCE_MM:
        p_thresholds[i].param_low_thresh  /= 4;
        p_thresholds[i].param_high_thresh /= 4;
        break;
      case VL53L8CX_SIGNAL_PER_SPAD_KCPS:
        p_thresholds[i].param_low_thresh  /= 2048;
        p_thresholds[i].param_high_thresh /= 2048;
        break;
      case VL53L8CX_RANGE_SIGMA_MM:
        p_thresholds[i].param_low_thresh  /= 128;
        p_thresholds[i].param_high_thresh /= 128;
        break;
      case VL53L8CX_AMBIENT_PER_SPAD_KCPS:
        p_thresholds[i].param_low_thresh  /= 2048;
        p_thresholds[i].param_high_thresh /= 2048;
        break;
      case VL53L8CX_NB_SPADS_ENABLED:
        p_thresholds[i].param_low_thresh  /= 256;
        p_thresholds[i].param_high_thresh /= 256;
        break;
      case VL53L8CX_MOTION_INDICATOR:
        p_thresholds[i].param_low_thresh  /= 65535;
        p_thresholds[i].param_high_thresh /= 65535;
        break;
      default:
        break;
    }
  }

  return VL53L8CX_OK;
}

VL53L8CX_OpResult VL53L8CX_Set_Detection_Thresholds(VL53L8CX_DetectionThresholds *p_thresholds)
{
  VL53L8CX_OpResult status;
  uint8_t i;
  uint8_t grp_valid_target_cfg[] = {0x05, 0x05, 0x05, 0x05,
                                    0x05, 0x05, 0x05, 0x05
                                   };

  for (i = 0; i < (uint8_t) VL53L8CX_NB_THRESHOLDS; i++) {
    switch (p_thresholds->measurement) {
      case VL53L8CX_DISTANCE_MM:
        p_thresholds[i].param_low_thresh  *= 4;
        p_thresholds[i].param_high_thresh *= 4;
        break;
      case VL53L8CX_SIGNAL_PER_SPAD_KCPS:
        p_thresholds[i].param_low_thresh  *= 2048;
        p_thresholds[i].param_high_thresh *= 2048;
        break;
      case VL53L8CX_RANGE_SIGMA_MM:
        p_thresholds[i].param_low_thresh  *= 128;
        p_thresholds[i].param_high_thresh *= 128;
        break;
      case VL53L8CX_AMBIENT_PER_SPAD_KCPS:
        p_thresholds[i].param_low_thresh  *= 2048;
        p_thresholds[i].param_high_thresh *= 2048;
        break;
      case VL53L8CX_NB_SPADS_ENABLED:
        p_thresholds[i].param_low_thresh  *= 256;
        p_thresholds[i].param_high_thresh *= 256;
        break;
      case VL53L8CX_MOTION_INDICATOR:
        p_thresholds[i].param_low_thresh  *= 65535;
        p_thresholds[i].param_high_thresh *= 65535;
        break;
      default:
        break;
    }
  }

  /* Set valid target list */
  status = VL53L8CX_DCI_Write_Data((uint8_t *)grp_valid_target_cfg, VL53L8CX_DCI_DET_THRESH_VALID_STATUS, (uint16_t)sizeof(grp_valid_target_cfg));
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  /* Set thresholds configuration */
  status = VL53L8CX_DCI_Write_Data((uint8_t *)p_thresholds, VL53L8CX_DCI_DET_THRESH_START, (uint16_t)(VL53L8CX_NB_THRESHOLDS * sizeof(VL53L8CX_DetectionThresholds)));
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  return VL53L8CX_OK;
}

VL53L8CX_OpResult VL53L8CX_Get_Detection_Thresholds_Auto_Stop(uint8_t *p_auto_stop)
{
  VL53L8CX_OpResult status;

  status = VL53L8CX_DCI_Read_Data((uint8_t *)temp_buffer, VL53L8CX_DCI_PIPE_CONTROL, 4);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  *p_auto_stop = temp_buffer[0x3];

  return VL53L8CX_OK;
}

VL53L8CX_OpResult VL53L8CX_Set_Detection_Thresholds_Auto_Stop(uint8_t auto_stop)
{
  VL53L8CX_OpResult status;

  status = VL53L8CX_DCI_Replace_Data(temp_buffer, VL53L8CX_DCI_PIPE_CONTROL, 4, (uint8_t *)&auto_stop, 1, 0x03);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  is_auto_stop_enabled = (uint8_t)auto_stop;

  return VL53L8CX_OK;
}
