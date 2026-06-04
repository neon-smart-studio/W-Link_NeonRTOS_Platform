
#ifndef CAN_H
#define CAN_H

#include <stdbool.h>
#include <stdint.h>

#include "soc.h"  

#include "CAN_Def.h"

#include "CAN_Index.h"

#include "Driver_Config.h"

#ifdef	__cplusplus
extern "C" {
#endif

hwCAN_OpResult CAN_Init(hwCAN_Index index);
hwCAN_OpResult CAN_DeInit(hwCAN_Index index);
hwCAN_OpResult CAN_Read(hwCAN_Index index, uint8_t *buf, uint32_t timeout);
hwCAN_OpResult CAN_Write(hwCAN_Index index, uint32_t id, uint8_t *data, uint8_t len, uint32_t timeout);
bool CAN_isInit(hwCAN_Index index);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // CAN_H