/**
 ******************************************************************************
 * @file    LSM303AGR_MAG_Sensor.h
 * @author  AST
 * @version V1.0.0
 * @date    7 September 2017
 * @brief   Abstract Class of an LSM303AGR magnetometer sensor.
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
 * Based on STMicroelectronics LSM303AGR driver
 * Modified by Neon Smart Studio for W-Link
 */


/* Prevent recursive inclusion -----------------------------------------------*/

#ifndef __LSM303AGR_MAG_Sensor_H__
#define __LSM303AGR_MAG_Sensor_H__

#include "LSM303AGR_IO.h"

#include "LSM303AGR_Def.h"

#ifdef __cplusplus
extern "C" {
#endif

LSM303AGR_OpStatus LSM303AGR_MAG_Init(void);
LSM303AGR_OpStatus LSM303AGR_MAG_DeInit(void);
LSM303AGR_OpStatus LSM303AGR_MAG_Enable(void);
LSM303AGR_OpStatus LSM303AGR_MAG_Disable(void);
LSM303AGR_OpStatus LSM303AGR_MAG_ReadID(uint8_t *p_id);
LSM303AGR_OpStatus LSM303AGR_MAG_GetAxes(int32_t *pData);
LSM303AGR_OpStatus LSM303AGR_MAG_GetSensitivity (float *pfData);
LSM303AGR_OpStatus LSM303AGR_MAG_GetAxesRaw(int16_t *pData);
LSM303AGR_OpStatus LSM303AGR_MAG_GetODR(float *odr);
LSM303AGR_OpStatus LSM303AGR_MAG_SetODR(float odr);
LSM303AGR_OpStatus LSM303AGR_MAG_GetFS(float *fullScale);
LSM303AGR_OpStatus LSM303AGR_MAG_SetFS(float fullScale);
LSM303AGR_OpStatus LSM303AGR_MAG_ReadReg(uint8_t reg, uint8_t *data);
LSM303AGR_OpStatus LSM303AGR_MAG_WriteReg(uint8_t reg, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif