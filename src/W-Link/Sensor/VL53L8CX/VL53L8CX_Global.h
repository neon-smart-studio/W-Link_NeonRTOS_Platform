
#ifndef VL53L8CX_GLOBAL_H
#define VL53L8CX_GLOBAL_H

#include <stdint.h>
#include <stdbool.h>

#include "VL53L8CX_Def.h"

/**
 * @brief Macro VL53L8CX_MAX_RESULTS_SIZE indicates the maximum size used by
 * output through I2C. Value 40 corresponds to headers + meta-data + common-data
 * and 20 corresponds to the footer.
 */

#define VL53L8CX_MAX_RESULTS_SIZE ( 40U \
  + L5CX_AMB_SIZE + L5CX_SPAD_SIZE + L5CX_NTAR_SIZE + L5CX_SPS_SIZE \
  + L5CX_SIGR_SIZE + L5CX_DIST_SIZE + L5CX_RFLEST_SIZE + L5CX_STA_SIZE \
  + L5CX_MOT_SIZE + 20U)

/**
 * @brief Macro VL53L8CX_TEMPORARY_BUFFER_SIZE can be used to know the size of
 * the temporary buffer. The minimum size is 1024, and the maximum depends of
 * the output configuration.
 */

#if VL53L8CX_MAX_RESULTS_SIZE < 1024U
#define VL53L8CX_TEMPORARY_BUFFER_SIZE ((uint32_t) 1024U)
#else
#define VL53L8CX_TEMPORARY_BUFFER_SIZE ((uint32_t) VL53L8CX_MAX_RESULTS_SIZE)
#endif

#define VL53L8CX_NVM_DATA_SIZE      ((uint16_t)492U)
#define VL53L8CX_CONFIGURATION_SIZE   ((uint16_t)972U)
#define VL53L8CX_XTALK_CALIBRATE_SIZE    ((uint16_t)984U)

#define VL53L8CX_OFFSET_BUFFER_SIZE   ((uint16_t)488U)
#define VL53L8CX_XTALK_BUFFER_SIZE    ((uint16_t)776U)

extern bool VL53L8CX_Is_Auto_Stop_Enabled;
extern uint32_t VL53L8CX_Data_Read_Size;
extern uint8_t VL53L8CX_Streamcount;
extern uint8_t VL53L8CX_Xtalk_Data[VL53L8CX_XTALK_BUFFER_SIZE];
extern uint8_t VL53L8CX_Offset_Data[VL53L8CX_OFFSET_BUFFER_SIZE];
extern uint8_t VL53L8CX_Temp_Buffer[VL53L8CX_TEMPORARY_BUFFER_SIZE];
extern const uint8_t VL53L8CX_DEFAULT_CONFIGURATION[];
extern const uint8_t VL53L8CX_DEFAULT_XTALK[];
extern const uint8_t VL53L8CX_CALIBRATE_XTALK[];

#endif  // VL53L8CX_GLOBAL_H
