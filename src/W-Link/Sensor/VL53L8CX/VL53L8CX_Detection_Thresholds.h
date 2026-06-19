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

#ifndef VL53L8CX_DETECTION_THRESHOLDS_H
#define VL53L8CX_DETECTION_THRESHOLDS_H

#include <stdint.h>
#include <stdbool.h>

#include "VL53L8CX_Def.h"

#include "Sensor_Config.h"

/**
 * @brief Macro VL53L8CX_NB_THRESHOLDS indicates the number of checkers. This
 * value cannot be changed.
 */

#define VL53L8CX_NB_THRESHOLDS        ((uint8_t)64U)

/**
 * @brief Inner Macro for API. Not for user, only for development.
 */

#define VL53L8CX_DCI_DET_THRESH_CONFIG      ((uint16_t)0x5488U)
#define VL53L8CX_DCI_DET_THRESH_GLOBAL_CONFIG ((uint16_t)0xB6E0U)
#define VL53L8CX_DCI_DET_THRESH_START     ((uint16_t)0xB6E8U)
#define VL53L8CX_DCI_DET_THRESH_VALID_STATUS  ((uint16_t)0xB9F0U)

/**
 * @brief Macro VL53L8CX_LAST_THRESHOLD is used to indicate the end of checkers
 * programming.
 */

#define VL53L8CX_LAST_THRESHOLD       ((uint8_t)128U)

/**
 * @brief The following macro are used to define the 'param_type' of a checker.
 * They indicate what is the measurement to catch.
 */

#define VL53L8CX_DISTANCE_MM        ((uint8_t)1U)
#define VL53L8CX_SIGNAL_PER_SPAD_KCPS     ((uint8_t)2U)
#define VL53L8CX_RANGE_SIGMA_MM       ((uint8_t)4U)
#define VL53L8CX_AMBIENT_PER_SPAD_KCPS    ((uint8_t)8U)
#define VL53L8CX_NB_TARGET_DETECTED     ((uint8_t)9U)
#define VL53L8CX_TARGET_STATUS        ((uint8_t)12U)
#define VL53L8CX_NB_SPADS_ENABLED     ((uint8_t)13U)
#define VL53L8CX_MOTION_INDICATOR           ((uint8_t)19U)

/**
 * @brief The following macro are used to define the 'type' of a checker.
 * They indicate the window of measurements, defined by low and a high
 * thresholds.
 */

#define VL53L8CX_IN_WINDOW          ((uint8_t)0U)
#define VL53L8CX_OUT_OF_WINDOW        ((uint8_t)1U)
#define VL53L8CX_LESS_THAN_EQUAL_MIN_CHECKER ((uint8_t)2U)
#define VL53L8CX_GREATER_THAN_MAX_CHECKER ((uint8_t)3U)
#define VL53L8CX_EQUAL_MIN_CHECKER      ((uint8_t)4U)
#define VL53L8CX_NOT_EQUAL_MIN_CHECKER    ((uint8_t)5U)

/**
 * @brief The following macro are used to define multiple checkers in the same
 * zone, using operators. Please note that the first checker MUST always be a OR
 * operation.
 */

#define VL53L8CX_OPERATION_NONE       ((uint8_t)0U)
#define VL53L8CX_OPERATION_OR       ((uint8_t)0U)
#define VL53L8CX_OPERATION_AND        ((uint8_t)2U)

/**
 * @brief Structure VL53L8CX_DetectionThresholds contains a single threshold.
 * This structure  is never used alone, it must be used as an array of 64
 * thresholds (defined by macro VL53L8CX_NB_THRESHOLDS).
 */

typedef struct {

  /* Low threshold */
  int32_t   param_low_thresh;
  /* High threshold */
  int32_t   param_high_thresh;
  /* Measurement to catch (VL53L8CX_MEDIAN_RANGE_MM,...)*/
  uint8_t   measurement;
  /* Windows type (VL53L8CX_IN_WINDOW, VL53L8CX_OUT_WINDOW, ...) */
  uint8_t   type;
  /* Zone id. Please read VL53L5 user manual to find the zone id.Set macro
   * VL53L8CX_LAST_THRESHOLD to indicates the end of checkers */
  uint8_t   zone_num;
  /* Mathematics operation (AND/OR). The first threshold is always OR.*/
  uint8_t   mathematic_operation;
} VL53L8CX_DetectionThresholds;

#ifdef __cplusplus
extern "C" {
#endif

VL53L8CX_OpResult VL53L8CX_Get_Detection_Thresholds_Enable(uint8_t *p_enabled);
VL53L8CX_OpResult VL53L8CX_Set_Detection_Thresholds_Enable(uint8_t enabled);
VL53L8CX_OpResult VL53L8CX_Get_Detection_Thresholds(VL53L8CX_DetectionThresholds *p_thresholds);
VL53L8CX_OpResult VL53L8CX_Set_Detection_Thresholds(VL53L8CX_DetectionThresholds *p_thresholds);
VL53L8CX_OpResult VL53L8CX_Get_Detection_Thresholds_Auto_Stop(uint8_t *p_auto_stop);
VL53L8CX_OpResult VL53L8CX_Set_Detection_Thresholds_Auto_Stop(uint8_t auto_stop);

#ifdef __cplusplus
}
#endif

#endif // VL53L8CX_DETECTION_THRESHOLDS_H
