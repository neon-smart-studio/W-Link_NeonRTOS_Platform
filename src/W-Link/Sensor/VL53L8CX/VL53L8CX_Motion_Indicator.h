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

#ifndef VL53L8CX_MOTION_INDICATOR_H
#define VL53L8CX_MOTION_INDICATOR_H

#include <stdint.h>
#include <stdbool.h>

#include "VL53L8CX_Def.h"

#include "Sensor_Config.h"

/**
 * @brief Motion indicator internal configuration structure.
 */

typedef struct {
  int32_t  ref_bin_offset;
  uint32_t detection_threshold;
  uint32_t extra_noise_sigma;
  uint32_t null_den_clip_value;
  uint8_t  mem_update_mode;
  uint8_t  mem_update_choice;
  uint8_t  sum_span;
  uint8_t  feature_length;
  uint8_t  nb_of_aggregates;
  uint8_t  nb_of_temporal_accumulations;
  uint8_t  min_nb_for_global_detection;
  uint8_t  global_indicator_format_1;
  uint8_t  global_indicator_format_2;
  uint8_t  spare_1;
  uint8_t  spare_2;
  uint8_t  spare_3;
  int8_t   map_id[64];
  uint8_t  indicator_format_1[32];
  uint8_t  indicator_format_2[32];
} VL53L8CX_Motion_Configuration;

#ifdef __cplusplus
extern "C" {
#endif

VL53L8CX_OpResult VL53L8CX_Motion_Indicator_Init(VL53L8CX_Motion_Configuration *p_motion_config, uint8_t resolution);
VL53L8CX_OpResult VL53L8CX_Motion_Indicator_Set_Distance_Motion(VL53L8CX_Motion_Configuration *p_motion_config, uint16_t distance_min_mm, uint16_t distance_max_mm);
VL53L8CX_OpResult VL53L8CX_Motion_Indicator_Set_Resolution(VL53L8CX_Motion_Configuration *p_motion_config, uint8_t resolution);

#ifdef __cplusplus
}
#endif

#endif // VL53L8CX_MOTION_INDICATOR_H
