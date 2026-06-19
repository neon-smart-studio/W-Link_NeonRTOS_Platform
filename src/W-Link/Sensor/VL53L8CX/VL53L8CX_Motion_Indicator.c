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

#include "VL53L8CX_Motion_Indicator.h"

VL53L8CX_OpResult VL53L8CX_Motion_Indicator_Init(VL53L8CX_Motion_Configuration *p_motion_config, uint8_t resolution)
{
  VL53L8CX_OpResult status;

  (void)memset(p_motion_config, 0, sizeof(VL53L8CX_Motion_Configuration));

  p_motion_config->ref_bin_offset = 13633;
  p_motion_config->detection_threshold = 2883584;
  p_motion_config->extra_noise_sigma = 0;
  p_motion_config->null_den_clip_value = 0;
  p_motion_config->mem_update_mode = 6;
  p_motion_config->mem_update_choice = 2;
  p_motion_config->sum_span = 4;
  p_motion_config->feature_length = 9;
  p_motion_config->nb_of_aggregates = 16;
  p_motion_config->nb_of_temporal_accumulations = 16;
  p_motion_config->min_nb_for_global_detection = 1;
  p_motion_config->global_indicator_format_1 = 8;
  p_motion_config->global_indicator_format_2 = 0;
  p_motion_config->spare_1 = 0;
  p_motion_config->spare_2 = 0;
  p_motion_config->spare_3 = 0;

  status = VL53L8CX_Motion_Indicator_Set_Resolution(p_motion_config, resolution);
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  return VL53L8CX_OK;
}

VL53L8CX_OpResult VL53L8CX_Motion_Indicator_Set_Distance_Motion(VL53L8CX_Motion_Configuration *p_motion_config, uint16_t distance_min_mm, uint16_t distance_max_mm)
{
  VL53L8CX_OpResult status;
  float_t tmp;

  if (((distance_max_mm - distance_min_mm) > (uint16_t)1500) || (distance_min_mm < (uint16_t)400) || (distance_max_mm > (uint16_t)4000))
  {
    return VL53L8CX_InvalidParameter;
  }

  tmp = (float_t)((((float_t)distance_min_mm / (float_t)37.5348) - (float_t)4.0) * (float_t)2048.5);
  p_motion_config->ref_bin_offset = (int32_t)tmp;

  tmp = (float_t)((((((float_t)distance_max_mm - (float_t)distance_min_mm) / (float_t)10.0) + (float_t)30.02784) / ((float_t)15.01392)) + (float_t)0.5);
  p_motion_config->feature_length = (uint8_t)tmp;

  status = VL53L8CX_DCI_Write_Data((uint8_t *)(p_motion_config), VL53L8CX_DCI_MOTION_DETECTOR_CFG, (uint16_t)sizeof(*p_motion_config));
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  return VL53L8CX_OK;
}

VL53L8CX_OpResult VL53L8CX_Motion_Indicator_Set_Resolution(VL53L8CX_Motion_Configuration *p_motion_config, uint8_t resolution)
{
  VL53L8CX_OpResult status;
  uint8_t i;

  switch (resolution) {
    case VL53L8CX_RESOLUTION_4X4:
      for (i = 0; i < (uint8_t)VL53L8CX_RESOLUTION_4X4; i++)
      {
        p_motion_config->map_id[i] = (int8_t)i;
      }
      (void)memset(p_motion_config->map_id + 16, -1, 48);
      break;

    case VL53L8CX_RESOLUTION_8X8:
      for (i = 0; i < (uint8_t)VL53L8CX_RESOLUTION_8X8; i++)
      {
        p_motion_config->map_id[i] = (int8_t)((((int8_t) i % 8) / 2) + (4 * ((int8_t)i / 16)));
      }
      break;

    default:
      return VL53L8CX_Status_Error;
  }

  status = VL53L8CX_DCI_Write_Data((uint8_t *)(p_motion_config), VL53L8CX_DCI_MOTION_DETECTOR_CFG, (uint16_t)sizeof(*p_motion_config));
  if(status < VL53L8CX_OK)
  {
    return status;
  }

  return VL53L8CX_OK;
}
