#ifndef CAN_NUVOTON_H
#define CAN_NUVOTON_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "CAN/CAN.h"

#include "CAN/Pin/STM32/CAN_Pin_STM32_Def.h"

CAN_T *CAN_Map_Soc_Base(hwCAN_Index index);

void CAN_TxMailbox0CompleteCallback(hwCAN_Index index);
void CAN_RxFifo0MsgPendingCallback(hwCAN_Index index, uint8_t data[8]);

void CAN_GPIO_ConfigAF(hwCAN_Index index);
void CAN_GPIO_DeConfigAF(hwCAN_Index index);

hwCAN_OpResult CAN_Instance_Init(hwCAN_Index index, uint32_t baudrate);
hwCAN_OpResult CAN_Instance_DeInit(hwCAN_Index index);
hwCAN_OpResult CAN_ConfigFilter(hwCAN_Index index);
hwCAN_OpResult CAN_StartHardware(hwCAN_Index index);
hwCAN_OpResult CAN_StopHardware(hwCAN_Index index);

void CAN_NVIC_Init(hwCAN_Index index);
void CAN_NVIC_DeInit(hwCAN_Index index);

#endif //CAN_NUVOTON_H