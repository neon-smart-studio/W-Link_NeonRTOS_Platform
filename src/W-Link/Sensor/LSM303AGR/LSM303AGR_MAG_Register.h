/**
 ******************************************************************************
 * @file    LSM303AGR_MAG_driver.h
 * @author  MEMS Application Team
 * @version V1.1
 * @date    25-February-2016
 * @brief   LSM303AGR Magnetometer header driver file
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

#ifndef __LSM303AGR_MAG_DRIVER__H
#define __LSM303AGR_MAG_DRIVER__H

#include "LSM303AGR_IO.h"

#include "LSM303AGR_Def.h"

#include "LSM303AGR_MAG_Register_Def.h"

#ifdef __cplusplus
extern "C" {
#endif

LSM303AGR_OpStatus LSM303AGR_MAG_W_OFF_X_L(uint8_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_OFF_X_L(uint8_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_OFF_X_H(uint8_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_OFF_X_H(uint8_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_OFF_Y_L(uint8_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_OFF_Y_L(uint8_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_OFF_Y_H(uint8_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_OFF_Y_H(uint8_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_OFF_Z_L(uint8_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_OFF_Z_L(uint8_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_OFF_Z_H(uint8_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_OFF_Z_H(uint8_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_Get_MagOff(uint16_t *magx_off, uint16_t *magy_off, uint16_t *magz_off);
LSM303AGR_OpStatus LSM303AGR_MAG_Set_MagOff(uint16_t magx_off, uint16_t magy_off, uint16_t magz_off);

LSM303AGR_OpStatus LSM303AGR_MAG_R_WHO_AM_I(uint8_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_MD(LSM303AGR_MAG_MD_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_MD(LSM303AGR_MAG_MD_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_ODR(LSM303AGR_MAG_ODR_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_ODR(LSM303AGR_MAG_ODR_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_LP(LSM303AGR_MAG_LP_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_LP(LSM303AGR_MAG_LP_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_SOFT_RST(LSM303AGR_MAG_SOFT_RST_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_SOFT_RST(LSM303AGR_MAG_SOFT_RST_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_LPF(LSM303AGR_MAG_LPF_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_LPF(LSM303AGR_MAG_LPF_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_OFF_CANC(LSM303AGR_MAG_OFF_CANC_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_OFF_CANC(LSM303AGR_MAG_OFF_CANC_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_SET_FREQ(LSM303AGR_MAG_SET_FREQ_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_SET_FREQ(LSM303AGR_MAG_SET_FREQ_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_INT_ON_DATAOFF(LSM303AGR_MAG_INT_ON_DATAOFF_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_INT_ON_DATAOFF(LSM303AGR_MAG_INT_ON_DATAOFF_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_INT_MAG(LSM303AGR_MAG_INT_MAG_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_INT_MAG(LSM303AGR_MAG_INT_MAG_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_ST(LSM303AGR_MAG_ST_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_ST(LSM303AGR_MAG_ST_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_BLE(LSM303AGR_MAG_BLE_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_BLE(LSM303AGR_MAG_BLE_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_BDU(LSM303AGR_MAG_BDU_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_BDU(LSM303AGR_MAG_BDU_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_I2C_DIS(LSM303AGR_MAG_I2C_DIS_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_I2C_DIS(LSM303AGR_MAG_I2C_DIS_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_INT_MAG_PIN(LSM303AGR_MAG_INT_MAG_PIN_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_INT_MAG_PIN(LSM303AGR_MAG_INT_MAG_PIN_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_IEN(LSM303AGR_MAG_IEN_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_IEN(LSM303AGR_MAG_IEN_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_IEL(LSM303AGR_MAG_IEL_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_IEL(LSM303AGR_MAG_IEL_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_IEA(LSM303AGR_MAG_IEA_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_IEA(LSM303AGR_MAG_IEA_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_ZIEN(LSM303AGR_MAG_ZIEN_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_ZIEN(LSM303AGR_MAG_ZIEN_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_YIEN(LSM303AGR_MAG_YIEN_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_YIEN(LSM303AGR_MAG_YIEN_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_W_XIEN(LSM303AGR_MAG_XIEN_t newValue);
LSM303AGR_OpStatus LSM303AGR_MAG_R_XIEN(LSM303AGR_MAG_XIEN_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_INT(LSM303AGR_MAG_INT_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_MROI(LSM303AGR_MAG_MROI_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_N_TH_S_Z(LSM303AGR_MAG_N_TH_S_Z_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_N_TH_S_Y(LSM303AGR_MAG_N_TH_S_Y_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_N_TH_S_X(LSM303AGR_MAG_N_TH_S_X_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_P_TH_S_Z(LSM303AGR_MAG_P_TH_S_Z_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_P_TH_S_Y(LSM303AGR_MAG_P_TH_S_Y_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_P_TH_S_X(LSM303AGR_MAG_P_TH_S_X_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_XDA(LSM303AGR_MAG_XDA_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_YDA(LSM303AGR_MAG_YDA_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_ZDA(LSM303AGR_MAG_ZDA_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_ZYXDA(LSM303AGR_MAG_ZYXDA_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_XOR(LSM303AGR_MAG_XOR_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_YOR(LSM303AGR_MAG_YOR_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_ZOR(LSM303AGR_MAG_ZOR_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_R_ZYXOR(LSM303AGR_MAG_ZYXOR_t *p_value);

LSM303AGR_OpStatus LSM303AGR_MAG_Get_Raw_Magnetic(uint8_t *buff);
LSM303AGR_OpStatus LSM303AGR_MAG_Get_Magnetic(int *buff);

LSM303AGR_OpStatus LSM303AGR_MAG_Get_IntThreshld(uint8_t *buff); 
LSM303AGR_OpStatus LSM303AGR_MAG_Set_IntThreshld(uint8_t *buff);

#ifdef __cplusplus
}
#endif

#endif
