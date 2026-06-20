
#ifndef VL53L5CX_GLOBAL_H
#define VL53L5CX_GLOBAL_H

#include <stdint.h>
#include <stdbool.h>

#include "VL53L5CX_Def.h"

/**
 * @brief Macro VL53L5CX_MAX_RESULTS_SIZE indicates the maximum size used by
 * output through I2C. Value 40 corresponds to headers + meta-data + common-data
 * and 8 corresponds to the footer.
 */

#define VL53L5CX_MAX_RESULTS_SIZE ( 40U \
  + L5CX_AMB_SIZE + L5CX_SPAD_SIZE + L5CX_NTAR_SIZE + L5CX_SPS_SIZE \
  + L5CX_SIGR_SIZE + L5CX_DIST_SIZE + L5CX_RFLEST_SIZE + L5CX_STA_SIZE \
  + L5CX_MOT_SIZE + 8U)

#define VL53L5CX_TEMPORARY_BUFFER_SIZE ((uint32_t) VL53L5CX_MAX_RESULTS_SIZE)

#define VL53L5CX_NVM_DATA_SIZE      ((uint16_t)492U)
#define VL53L5CX_CONFIGURATION_SIZE   ((uint16_t)972U)
#define VL53L5CX_XTALK_CALIBRATE_SIZE    ((uint16_t)984U)

#define VL53L5CX_OFFSET_BUFFER_SIZE   ((uint16_t)488U)
#define VL53L5CX_XTALK_BUFFER_SIZE    ((uint16_t)776U)

extern uint32_t VL53L5CX_Data_Read_Size;
extern uint8_t VL53L5CX_Streamcount;
extern uint8_t VL53L5CX_Xtalk_Data[VL53L5CX_XTALK_BUFFER_SIZE];
extern uint8_t VL53L5CX_Offset_Data[VL53L5CX_OFFSET_BUFFER_SIZE];
extern uint8_t VL53L5CX_Temp_Buffer[VL53L5CX_TEMPORARY_BUFFER_SIZE];
extern const uint8_t VL53L5CX_DEFAULT_CONFIGURATION[];
extern const uint8_t VL53L5CX_DEFAULT_XTALK[];
extern const uint8_t VL53L5CX_CALIBRATE_XTALK[];

#endif  // VL53L5CX_GLOBAL_H
