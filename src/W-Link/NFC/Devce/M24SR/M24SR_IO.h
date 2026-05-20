/**
  ******************************************************************************
  * @file    m24sr.h
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage M24SR
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/*
 * Based on STMicroelectronics M24SR driver
 * Modified by Neon Smart Studio for W-Link
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef M24SR_IO_H
#define M24SR_IO_H

#include "NFC_Config.h"

#define M24SR_I2C_ADDR                   0xAC

#define M24SR_I2C_OP_TIMEOUT             500

#ifndef CONFIG_M24SR_I2C_INDEX
#define M24SR_I2C_INDEX                  hwI2C_Index_0
#else
#define M24SR_I2C_INDEX                  CONFIG_M24SR_I2C_INDEX
#endif

#ifndef CONFIG_M24SR_GPIO_GPO_PIN
#define M24SR_GPIO_GPO_PIN               hwGPIO_Int_Pin_E4
#else
#define M24SR_GPIO_GPO_PIN               CONFIG_M24SR_GPIO_GPO_PIN
#endif

#ifndef CONFIG_M24SR_GPIO_RFDISABLE_PIN
#define M24SR_GPIO_RFDISABLE_PIN         hwGPIO_Pin_E2
#else
#define M24SR_GPIO_RFDISABLE_PIN         CONFIG_M24SR_GPIO_RFDISABLE_PIN
#endif

#ifdef __cplusplus
 extern "C" {
#endif

typedef void (*M24SR_GPO_Event_Handler)();

NFC_OpResult M24SR_IO_Init(M24SR_GPO_Event_Handler cb);
NFC_OpResult M24SR_IO_DeInit();
NFC_OpResult M24SR_IO_ReadMultiple(uint8_t *pBuffer, uint16_t len);
NFC_OpResult M24SR_IO_WriteMultiple(uint8_t *pBuffer, uint16_t len);
NFC_OpResult M24SR_IO_RF_Enable();
NFC_OpResult M24SR_IO_RF_Disable();

#ifdef __cplusplus
}
#endif

#endif // M24SR_IO_H




/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
