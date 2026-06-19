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

#ifndef VL53L8CX_XTALK_H
#define VL53L8CX_XTALK_H

#include <stdint.h>
#include <stdbool.h>

#include "VL53L8CX_Def.h"

#include "Sensor_Config.h"

#ifdef __cplusplus
extern "C" {
#endif

VL53L8CX_OpResult VL53L8CX_Calibrate_Xtalk(uint16_t reflectance_percent, uint8_t nb_samples, uint16_t distance_mm);
VL53L8CX_OpResult VL53L8CX_Get_Caldata_Xtalk(uint8_t *p_xtalk_data);
VL53L8CX_OpResult VL53L8CX_Set_Caldata_Xtalk(uint8_t *p_xtalk_data);
VL53L8CX_OpResult VL53L8CX_Get_Xtalk_Margin(uint32_t *p_xtalk_margin);
VL53L8CX_OpResult VL53L8CX_Set_Xtalk_Margin(uint32_t xtalk_margin);

#ifdef __cplusplus
}
#endif

#endif /* VL53L8CX_PLUGIN_XTALK_H_ */
