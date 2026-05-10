/**
 ******************************************************************************
 * @file    LIS3MDL_MAG_driver.h
 * @author  MEMS Application Team
 * @version V1.2
 * @date    9-August-2016
 * @brief   LIS3MDL driver header file
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
#ifndef LIS3MDL_MAG_REGISTER_H
#define LIS3MDL_MAG_REGISTER_H

#include <stdint.h>

#include <stdint.h>

#include "LIS3MDL_IO.h"

#include "LIS3MDL_Def.h"

#include "LIS3MDL_MAG_Register_Def.h"

#ifdef __cplusplus
extern "C" {
#endif

LIS3MDL_OpStatus LIS3MDL_MAG_WriteReg(uint8_t Reg, uint8_t *Bufp, uint16_t len );
LIS3MDL_OpStatus LIS3MDL_MAG_ReadReg(uint8_t Reg, uint8_t *Bufp, uint16_t len );

LIS3MDL_OpStatus LIS3MDL_MAG_R_WHO_AM_I_(uint8_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_SystemOperatingMode(LIS3MDL_MAG_MD_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_SystemOperatingMode(LIS3MDL_MAG_MD_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_BlockDataUpdate(LIS3MDL_MAG_BDU_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_BlockDataUpdate(LIS3MDL_MAG_BDU_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_FullScale(LIS3MDL_MAG_FS_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_FullScale(LIS3MDL_MAG_FS_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_OutputDataRate(LIS3MDL_MAG_DO_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_OutputDataRate(LIS3MDL_MAG_DO_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_Get_Magnetic(uint8_t *buff);

LIS3MDL_OpStatus LIS3MDL_MAG_W_SelfTest(LIS3MDL_MAG_ST_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_SelfTest(LIS3MDL_MAG_ST_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_OperatingModeXY(LIS3MDL_MAG_OM_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_OperatingModeXY(LIS3MDL_MAG_OM_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_TemperatureSensor(LIS3MDL_MAG_TEMP_EN_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_TemperatureSensor(LIS3MDL_MAG_TEMP_EN_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_SoftRST(LIS3MDL_MAG_SOFT_RST_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_SoftRST(LIS3MDL_MAG_SOFT_RST_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_Reboot(LIS3MDL_MAG_REBOOT_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_Reboot(LIS3MDL_MAG_REBOOT_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_SerialInterfaceMode(LIS3MDL_MAG_SIM_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_SerialInterfaceMode(LIS3MDL_MAG_SIM_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_FastLowPowerXYZ(LIS3MDL_MAG_LP_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_FastLowPowerXYZ(LIS3MDL_MAG_LP_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_LittleBigEndianInversion(LIS3MDL_MAG_BLE_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_LittleBigEndianInversion(LIS3MDL_MAG_BLE_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_OperatingModeZ(LIS3MDL_MAG_OMZ_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_OperatingModeZ(LIS3MDL_MAG_OMZ_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_R_NewXData(LIS3MDL_MAG_XDA_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_R_NewYData(LIS3MDL_MAG_YDA_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_R_NewZData(LIS3MDL_MAG_ZDA_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_R_NewXYZData(LIS3MDL_MAG_ZYXDA_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_R_DataXOverrun(LIS3MDL_MAG_XOR_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_R_DataYOverrun(LIS3MDL_MAG_YOR_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_R_DataZOverrun(LIS3MDL_MAG_ZOR_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_R_DataXYZOverrun(LIS3MDL_MAG_ZYXOR_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_InterruptEnable(LIS3MDL_MAG_IEN_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_InterruptEnable(LIS3MDL_MAG_IEN_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_LatchInterruptRq(LIS3MDL_MAG_LIR_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_LatchInterruptRq(LIS3MDL_MAG_LIR_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_InterruptActive(LIS3MDL_MAG_IEA_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_InterruptActive(LIS3MDL_MAG_IEA_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_InterruptOnZ(LIS3MDL_MAG_ZIEN_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_InterruptOnZ(LIS3MDL_MAG_ZIEN_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_InterruptOnY(LIS3MDL_MAG_YIEN_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_InterruptOnY(LIS3MDL_MAG_YIEN_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_InterruptOnX(LIS3MDL_MAG_XIEN_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_InterruptOnX(LIS3MDL_MAG_XIEN_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_InterruptFlag(LIS3MDL_MAG_INT_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_InterruptFlag(LIS3MDL_MAG_INT_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_MagneticFieldOverflow(LIS3MDL_MAG_MROI_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_MagneticFieldOverflow(LIS3MDL_MAG_MROI_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_NegativeThresholdFlagZ(LIS3MDL_MAG_NTH_Z_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_NegativeThresholdFlagZ(LIS3MDL_MAG_NTH_Z_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_NegativeThresholdFlagY(LIS3MDL_MAG_NTH_Y_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_NegativeThresholdFlagY(LIS3MDL_MAG_NTH_Y_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_NegativeThresholdFlagX(LIS3MDL_MAG_NTH_X_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_NegativeThresholdFlagX(LIS3MDL_MAG_NTH_X_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_PositiveThresholdFlagZ(LIS3MDL_MAG_PTH_Z_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_PositiveThresholdFlagZ(LIS3MDL_MAG_PTH_Z_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_PositiveThresholdFlagY(LIS3MDL_MAG_PTH_Y_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_PositiveThresholdFlagY(LIS3MDL_MAG_PTH_Y_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_W_PositiveThresholdFlagX(LIS3MDL_MAG_PTH_X_t newValue);
LIS3MDL_OpStatus LIS3MDL_MAG_R_PositiveThresholdFlagX(LIS3MDL_MAG_PTH_X_t *value);

LIS3MDL_OpStatus LIS3MDL_MAG_Get_Temperature(uint8_t *buff); 

LIS3MDL_OpStatus LIS3MDL_MAG_Set_MagneticThreshold(uint8_t *buff);
LIS3MDL_OpStatus LIS3MDL_MAG_Get_MagneticThreshold(uint8_t *buff); 

void LIS3MDL_MAG_SwapHighLowByte(uint8_t *bufferToSwap, uint8_t numberOfByte, uint8_t dimension); 

#ifdef __cplusplus
}
#endif

#endif // LIS3MDL_MAG_REGISTER_H
