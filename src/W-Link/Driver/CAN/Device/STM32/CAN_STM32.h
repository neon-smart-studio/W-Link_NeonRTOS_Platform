#ifndef CAN_STM32_H
#define CAN_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "CAN/CAN.h"

#if defined(CAN1_BASE) || defined(CAN2_BASE) || defined(CAN3_BASE)

#include "CAN/Pin/STM32/CAN_Pin_STM32_Def.h"

#define CAN_IRQ_NVIC_PRIORITY      5
#define CAN_IRQ_NVIC_SUB_PRIORITY  0

extern CAN_HandleTypeDef g_can[hwCAN_Index_MAX];

CAN_TypeDef *CAN_Map_Soc_Base(hwCAN_Index index);
uint32_t CAN_GetAF(hwCAN_Index can, hwGPIO_Pin pin);
uint32_t CAN_GetClock(hwCAN_Index index);

hwCAN_OpResult CAN_Instance_Init(hwCAN_Index index, uint32_t bitrate);
hwCAN_OpResult CAN_Instance_DeInit(hwCAN_Index index);

hwCAN_OpResult CAN_ConfigFilter(hwCAN_Index index);
hwCAN_OpResult CAN_StartHardware(hwCAN_Index index);
hwCAN_OpResult CAN_StopHardware(hwCAN_Index index);

void CAN_NVIC_Init(hwCAN_Index index);
void CAN_NVIC_DeInit(hwCAN_Index index);

hwCAN_Index CAN_IndexFromHandle(CAN_HandleTypeDef *hcan);

#endif // CAN1_BASE || CAN2_BASE || CAN3_BASE

#endif