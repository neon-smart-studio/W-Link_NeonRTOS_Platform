/**
 ******************************************************************************
 * @file    LIS3MDLSensor.h
 * @author  AST
 * @version V1.0.0
 * @date    7 September 2017
 * @brief   Abstract Class of an LIS3MDL magnetometer sensor.
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
 * Based on STMicroelectronics LIS3MDL driver
 * Modified by Neon Smart Studio for W-Link
 */


/* Prevent recursive inclusion -----------------------------------------------*/

#ifndef LIS3MDL_H
#define LIS3MDL_H

#include "LIS3MDL_IO.h"

#include "LIS3MDL_Def.h"

#define LIS3MDL_MAG_SENSITIVITY_FOR_FS_4G   0.14  /**< Sensitivity value for 4 gauss full scale [LSB/gauss] */
#define LIS3MDL_MAG_SENSITIVITY_FOR_FS_8G   0.29  /**< Sensitivity value for 8 gauss full scale [LSB/gauss] */
#define LIS3MDL_MAG_SENSITIVITY_FOR_FS_12G  0.43  /**< Sensitivity value for 12 gauss full scale [LSB/gauss] */
#define LIS3MDL_MAG_SENSITIVITY_FOR_FS_16G  0.58  /**< Sensitivity value for 16 gauss full scale [LSB/gauss] */

#ifdef __cplusplus
extern "C" {
#endif
LIS3MDL_OpResult LIS3MDL_Init(void);
LIS3MDL_OpResult LIS3MDL_DeInit(void);
LIS3MDL_OpResult LIS3MDL_Enable(void);
LIS3MDL_OpResult LIS3MDL_Disable(void);
LIS3MDL_OpResult LIS3MDL_ReadID(uint8_t *p_id);
LIS3MDL_OpResult LIS3MDL_GetAxes(int32_t *pData);
LIS3MDL_OpResult LIS3MDL_GetSensitivity(float *pfData);
LIS3MDL_OpResult LIS3MDL_GetAxesRaw(int16_t *pData);
LIS3MDL_OpResult LIS3MDL_GetODR(float *odr);
LIS3MDL_OpResult LIS3MDL_SetODR(float odr);
LIS3MDL_OpResult LIS3MDL_GetFS(float *fullScale);
LIS3MDL_OpResult LIS3MDL_SetFS(float fullScale);
LIS3MDL_OpResult LIS3MDL_ReadReg(uint8_t reg, uint8_t *data);
LIS3MDL_OpResult LIS3MDL_WriteReg(uint8_t reg, uint8_t data);
#ifdef __cplusplus
}
#endif

#endif // LIS3MDL_H