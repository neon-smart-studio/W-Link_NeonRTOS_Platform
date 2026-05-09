/**
 ******************************************************************************
 * @file    HTS221Sensor.h
 * @author  AST
 * @version V1.0.0
 * @date    7 September 2017
 * @brief   Abstract class of a HTS221 Humidity and Temperature sensor.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/*
 * Based on STMicroelectronics HTS221 driver
 * Modified by Neon Smart Studio for W-Link
 */
#ifndef HTS221_H
#define HTS221_H

#include "HTS221_IO.h"

#include "HTS221_Def.h"

typedef enum
{
  HTS221_STATUS_OK = 0,
  HTS221_STATUS_ERROR,
  HTS221_STATUS_TIMEOUT,
  HTS221_STATUS_NOT_IMPLEMENTED
} HTS221StatusTypeDef;

#ifdef __cplusplus
extern "C" {
#endif

HTS221_OpStatus HTS221_Init(void);
HTS221_OpStatus HTS221_DeInit(void);
HTS221_OpStatus HTS221_Enable(void);
HTS221_OpStatus HTS221_Disable(void);
HTS221_OpStatus HTS221_ReadID(uint8_t *ht_id);
HTS221_OpStatus HTS221_Reset(void);
HTS221_OpStatus HTS221_GetHumidity(float *pfData);
HTS221_OpStatus HTS221_GetTemperature(float *pfData);
HTS221_OpStatus HTS221_GetODR(float *odr);
HTS221_OpStatus HTS221_SetODR(float odr);

#ifdef __cplusplus
}
#endif

#endif // HTS221_H