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

#ifndef VL53L8CX_PLUGIN_XTALK_H_
#define VL53L8CX_PLUGIN_XTALK_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "vl53l8cx_api.h"

/**
 * @brief Inner Macro for plugin. Not for user, only for development.
 */

#define VL53L8CX_DCI_CAL_CFG        ((uint16_t)0x5470U)
#define VL53L8CX_DCI_XTALK_CFG        ((uint16_t)0xAD94U)


/**
 * @brief This function starts the VL53L8CX sensor in order to calibrate Xtalk.
 * This calibration is recommended is user wants to use a coverglass.
 * @param (VL53L8CX_Configuration) *p_dev : VL53L8CX configuration structure.
 * @param (uint16_t) reflectance_percent : Target reflectance in percent. This
 * value is include between 1 and 99%. For a better efficiency, ST recommends a
 * 3% target reflectance.
 * @param (uint8_t) nb_samples : Nb of samples used for calibration. A higher
 * number of samples means a higher accuracy, but it increases the calibration
 * time. Minimum is 1 and maximum is 16.
 * @param (uint16_t) distance_mm : Target distance in mm. The minimum allowed
 * distance is 600mm, and maximum is 3000mm. The target must stay in Full FOV,
 * so short distance are easier for calibration.
 * @return (uint8_t) status : 0 if calibration OK, 127 if an argument has an
 * incorrect value, or 255 is something failed.
 */

uint8_t vl53l8cx_calibrate_xtalk(
  VL53L8CX_Configuration    *p_dev,
  uint16_t      reflectance_percent,
  uint8_t       nb_samples,
  uint16_t      distance_mm);

/**
 * @brief This function gets the Xtalk buffer. The buffer is available after
 * using the function vl53l8cx_calibrate_xtalk().
 * @param (VL53L8CX_Configuration) *p_dev : VL53L5 configuration structure.
 * @param (uint8_t) *p_xtalk_data : Buffer with a size defined by
 * macro VL53L8CX_XTALK_SIZE.
 * @return (uint8_t) status : 0 if buffer reading OK
 */

uint8_t vl53l8cx_get_caldata_xtalk(
  VL53L8CX_Configuration    *p_dev,
  uint8_t       *p_xtalk_data);

/**
 * @brief This function sets the Xtalk buffer. This function can be used to
 * override default Xtalk buffer.
 * @param (VL53L8CX_Configuration) *p_dev : VL53L5 configuration structure.
 * @param (uint8_t) *p_xtalk_data : Buffer with a size defined by
 * macro VL53L8CX_XTALK_SIZE.
 * @return (uint8_t) status : 0 if buffer OK
 */

uint8_t vl53l8cx_set_caldata_xtalk(
  VL53L8CX_Configuration    *p_dev,
  uint8_t       *p_xtalk_data);

/**
 * @brief This function gets the Xtalk margin. This margin is used to increase
 * the Xtalk threshold. It can also be used to avoid false positives after the
 * Xtalk calibration. The default value is 50 kcps/spads.
 * @param (VL53L8CX_Configuration) *p_dev : VL53L8CX configuration structure.
 * @param (uint32_t) *p_xtalk_margin : Xtalk margin in kcps/spads.
 * @return (uint8_t) status : 0 if reading OK
 */

uint8_t vl53l8cx_get_xtalk_margin(
  VL53L8CX_Configuration    *p_dev,
  uint32_t      *p_xtalk_margin);

/**
 * @brief This function sets the Xtalk margin. This margin is used to increase
 * the Xtalk threshold. It can also be used to avoid false positives after the
 * Xtalk calibration. The default value is 50 kcps/spads.
 * @param (VL53L8CX_Configuration) *p_dev : VL53L8CX configuration structure.
 * @param (uint32_t) xtalk_margin : New Xtalk margin in kcps/spads. Min value is
 * 0 kcps/spads, and max is 10.000 kcps/spads
 * @return (uint8_t) status : 0 if set margin is OK, or 127 is the margin is
 * invalid.
 */

uint8_t vl53l8cx_set_xtalk_margin(
  VL53L8CX_Configuration    *p_dev,
  uint32_t      xtalk_margin);

/**
 * @brief Command used to get Xtalk calibration data
 */

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

#ifdef __cplusplus
}
#endif

#endif /* VL53L8CX_PLUGIN_XTALK_H_ */
