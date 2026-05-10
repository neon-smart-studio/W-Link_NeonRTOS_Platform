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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LIS3MDL_MAG_REGISTER_DEF_H
#define LIS3MDL_MAG_REGISTER_DEF_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/

//these could change accordingly with the architecture

#ifndef __ARCHDEP__TYPES
#define __ARCHDEP__TYPES

typedef unsigned char u8_t;
typedef unsigned short int u16_t;
typedef unsigned int u32_t;
typedef int i32_t;
typedef short int i16_t;
typedef signed char i8_t;

#endif /*__ARCHDEP__TYPES*/

/************** Who am I  *******************/

#define LIS3MDL_MAG_WHO_AM_I         0x3D

/************** Device Register  *******************/
#define LIS3MDL_MAG_WHO_AM_I_REG  	0X0F
#define LIS3MDL_MAG_CTRL_REG1  	0X20
#define LIS3MDL_MAG_CTRL_REG2  	0X21
#define LIS3MDL_MAG_CTRL_REG3  	0X22
#define LIS3MDL_MAG_CTRL_REG4  	0X23
#define LIS3MDL_MAG_CTRL_REG5  	0X24
#define LIS3MDL_MAG_STATUS_REG  	0X27
#define LIS3MDL_MAG_OUTX_L  	0X28
#define LIS3MDL_MAG_OUTX_H  	0X29
#define LIS3MDL_MAG_OUTY_L  	0X2A
#define LIS3MDL_MAG_OUTY_H  	0X2B
#define LIS3MDL_MAG_OUTZ_L  	0X2C
#define LIS3MDL_MAG_OUTZ_H  	0X2D
#define LIS3MDL_MAG_TEMP_OUT_L  	0X2E
#define LIS3MDL_MAG_TEMP_OUT_H  	0X2F
#define LIS3MDL_MAG_INT_CFG  	0X30
#define LIS3MDL_MAG_INT_SRC  	0X31
#define LIS3MDL_MAG_INT_THS_L  	0X32
#define LIS3MDL_MAG_INT_THS_H  	0X33

#define LIS3MDL_MAG_WHO_AM_I_BIT_MASK  	0xFF
#define LIS3MDL_MAG_WHO_AM_I_BIT_POSITION  	0

#define LIS3MDL_MAG_MD_MASK  	0x03
typedef enum {
  	LIS3MDL_MAG_MD_CONTINUOUS 		 =0x00,
  	LIS3MDL_MAG_MD_SINGLE 		 =0x01,
  	LIS3MDL_MAG_MD_POWER_DOWN 		 =0x02,
  	LIS3MDL_MAG_MD_POWER_DOWN_AUTO 		 =0x03,
} LIS3MDL_MAG_MD_t;

#define LIS3MDL_MAG_BDU_MASK  	0x40
typedef enum {
  	LIS3MDL_MAG_BDU_DISABLE 		 =0x00,
  	LIS3MDL_MAG_BDU_ENABLE 		 =0x40,
} LIS3MDL_MAG_BDU_t;

#define LIS3MDL_MAG_FS_MASK  	0x60
typedef enum {
  	LIS3MDL_MAG_FS_4Ga 		 =0x00,
  	LIS3MDL_MAG_FS_8Ga 		 =0x20,
  	LIS3MDL_MAG_FS_12Ga 		 =0x40,
  	LIS3MDL_MAG_FS_16Ga 		 =0x60,
} LIS3MDL_MAG_FS_t;

#define LIS3MDL_MAG_DO_MASK  	0x1C
typedef enum {
  	LIS3MDL_MAG_DO_0_625Hz 		 =0x00,
  	LIS3MDL_MAG_DO_1_25Hz 		 =0x04,
  	LIS3MDL_MAG_DO_2_5Hz 		 =0x08,
  	LIS3MDL_MAG_DO_5Hz 		 =0x0C,
  	LIS3MDL_MAG_DO_10Hz 		 =0x10,
  	LIS3MDL_MAG_DO_20Hz 		 =0x14,
  	LIS3MDL_MAG_DO_40Hz 		 =0x18,
  	LIS3MDL_MAG_DO_80Hz 		 =0x1C,
} LIS3MDL_MAG_DO_t;

#define LIS3MDL_MAG_ST_MASK  	0x01
typedef enum {
  	LIS3MDL_MAG_ST_DISABLE 		 =0x00,
  	LIS3MDL_MAG_ST_ENABLE 		 =0x01,
} LIS3MDL_MAG_ST_t;

#define LIS3MDL_MAG_OM_MASK  	0x60
typedef enum {
  	LIS3MDL_MAG_OM_LOW_POWER 		 =0x00,
  	LIS3MDL_MAG_OM_MEDIUM 		 =0x20,
  	LIS3MDL_MAG_OM_HIGH 		 =0x40,
  	LIS3MDL_MAG_OM_ULTRA_HIGH 		 =0x60,
} LIS3MDL_MAG_OM_t;

#define LIS3MDL_MAG_TEMP_EN_MASK  	0x80
typedef enum {
  	LIS3MDL_MAG_TEMP_EN_DISABLE 		 =0x00,
  	LIS3MDL_MAG_TEMP_EN_ENABLE 		 =0x80,
} LIS3MDL_MAG_TEMP_EN_t;

#define LIS3MDL_MAG_SOFT_RST_MASK  	0x04
typedef enum {
  	LIS3MDL_MAG_SOFT_RST_NO 		 =0x00,
  	LIS3MDL_MAG_SOFT_RST_YES 		 =0x04,
} LIS3MDL_MAG_SOFT_RST_t;

#define LIS3MDL_MAG_REBOOT_MASK  	0x08
typedef enum {
  	LIS3MDL_MAG_REBOOT_NO 		 =0x00,
  	LIS3MDL_MAG_REBOOT_YES 		 =0x08,
} LIS3MDL_MAG_REBOOT_t;

#define LIS3MDL_MAG_SIM_MASK  	0x04
typedef enum {
  	LIS3MDL_MAG_SIM_4_WIRE 		 =0x00,
  	LIS3MDL_MAG_SIM_3_WIRE 		 =0x04,
} LIS3MDL_MAG_SIM_t;

#define LIS3MDL_MAG_LP_MASK  	0x20
typedef enum {
  	LIS3MDL_MAG_LP_DISABLE 		 =0x00,
  	LIS3MDL_MAG_LP_ENABLE 		 =0x20,
} LIS3MDL_MAG_LP_t;

#define LIS3MDL_MAG_BLE_MASK  	0x02
typedef enum {
  	LIS3MDL_MAG_BLE_INVERT 		 =0x00,
  	LIS3MDL_MAG_BLE_DEFAULT 		 =0x02,
} LIS3MDL_MAG_BLE_t;

#define LIS3MDL_MAG_OMZ_MASK  	0x0C
typedef enum {
  	LIS3MDL_MAG_OMZ_LOW_POWER 		 =0x00,
  	LIS3MDL_MAG_OMZ_MEDIUM 		 =0x04,
  	LIS3MDL_MAG_OMZ_HIGH 		 =0x08,
  	LIS3MDL_MAG_OMZ_ULTRA_HIGH 		 =0x0C,
} LIS3MDL_MAG_OMZ_t;

#define LIS3MDL_MAG_XDA_MASK  	0x01
typedef enum {
  	LIS3MDL_MAG_XDA_NOT_AVAILABLE 		 =0x00,
  	LIS3MDL_MAG_XDA_AVAILABLE 		 =0x01,
} LIS3MDL_MAG_XDA_t;

#define LIS3MDL_MAG_YDA_MASK  	0x02
typedef enum {
  	LIS3MDL_MAG_YDA_NOT_AVAILABLE 		 =0x00,
  	LIS3MDL_MAG_YDA_AVAILABLE 		 =0x02,
} LIS3MDL_MAG_YDA_t;

#define LIS3MDL_MAG_ZDA_MASK  	0x04
typedef enum {
  	LIS3MDL_MAG_ZDA_NOT_AVAILABLE 		 =0x00,
  	LIS3MDL_MAG_ZDA_AVAILABLE 		 =0x04,
} LIS3MDL_MAG_ZDA_t;

#define LIS3MDL_MAG_ZYXDA_MASK  	0x08
typedef enum {
  	LIS3MDL_MAG_ZYXDA_NOT_AVAILABLE 		 =0x00,
  	LIS3MDL_MAG_ZYXDA_AVAILABLE 		 =0x08,
} LIS3MDL_MAG_ZYXDA_t;

#define LIS3MDL_MAG_XOR_MASK  	0x10
typedef enum {
  	LIS3MDL_MAG_XOR_NOT_OVERRUN 		 =0x00,
  	LIS3MDL_MAG_XOR_OVERRUN 		 =0x10,
} LIS3MDL_MAG_XOR_t;

#define LIS3MDL_MAG_YOR_MASK  	0x20
typedef enum {
  	LIS3MDL_MAG_YOR_NOT_OVERRUN 		 =0x00,
  	LIS3MDL_MAG_YOR_OVERRUN 		 =0x20,
} LIS3MDL_MAG_YOR_t;

#define LIS3MDL_MAG_ZOR_MASK  	0x40
typedef enum {
  	LIS3MDL_MAG_ZOR_NOT_OVERRUN 		 =0x00,
  	LIS3MDL_MAG_ZOR_OVERRUN 		 =0x40,
} LIS3MDL_MAG_ZOR_t;

#define LIS3MDL_MAG_ZYXOR_MASK  	0x80
typedef enum {
  	LIS3MDL_MAG_ZYXOR_NOT_OVERRUN 		 =0x00,
  	LIS3MDL_MAG_ZYXOR_OVERRUN 		 =0x80,
} LIS3MDL_MAG_ZYXOR_t;

#define LIS3MDL_MAG_IEN_MASK  	0x01
typedef enum {
  	LIS3MDL_MAG_IEN_DISABLE 		 =0x00,
  	LIS3MDL_MAG_IEN_ENABLE 		 =0x01,
} LIS3MDL_MAG_IEN_t;

#define LIS3MDL_MAG_LIR_MASK  	0x02
typedef enum {
  	LIS3MDL_MAG_LIR_LATCHED 		 =0x00,
  	LIS3MDL_MAG_LIR_NOT_LATCHED 		 =0x02,
} LIS3MDL_MAG_LIR_t;

#define LIS3MDL_MAG_IEA_MASK  	0x04
typedef enum {
  	LIS3MDL_MAG_IEA_LOW 		 =0x00,
  	LIS3MDL_MAG_IEA_HIGH 		 =0x04,
} LIS3MDL_MAG_IEA_t;

#define LIS3MDL_MAG_ZIEN_MASK  	0x20
typedef enum {
  	LIS3MDL_MAG_ZIEN_DISABLE 		 =0x00,
  	LIS3MDL_MAG_ZIEN_ENABLE 		 =0x20,
} LIS3MDL_MAG_ZIEN_t;

#define LIS3MDL_MAG_YIEN_MASK  	0x40
typedef enum {
  	LIS3MDL_MAG_YIEN_DISABLE 		 =0x00,
  	LIS3MDL_MAG_YIEN_ENABLE 		 =0x40,
} LIS3MDL_MAG_YIEN_t;

#define LIS3MDL_MAG_XIEN_MASK  	0x80
typedef enum {
  	LIS3MDL_MAG_XIEN_DISABLE 		 =0x00,
  	LIS3MDL_MAG_XIEN_ENABLE 		 =0x80,
} LIS3MDL_MAG_XIEN_t;

#define LIS3MDL_MAG_INT_MASK  	0x01
typedef enum {
  	LIS3MDL_MAG_INT_DOWN 		 =0x00,
  	LIS3MDL_MAG_INT_UP 		 =0x01,
} LIS3MDL_MAG_INT_t;

#define LIS3MDL_MAG_MROI_MASK  	0x02
typedef enum {
  	LIS3MDL_MAG_MROI_IN_RANGE 		 =0x00,
  	LIS3MDL_MAG_MROI_OVERFLOW 		 =0x02,
} LIS3MDL_MAG_MROI_t;

#define LIS3MDL_MAG_NTH_Z_MASK  	0x04
typedef enum {
  	LIS3MDL_MAG_NTH_Z_DOWN 		 =0x00,
  	LIS3MDL_MAG_NTH_Z_UP 		 =0x04,
} LIS3MDL_MAG_NTH_Z_t;

#define LIS3MDL_MAG_NTH_Y_MASK  	0x08
typedef enum {
  	LIS3MDL_MAG_NTH_Y_DOWN 		 =0x00,
  	LIS3MDL_MAG_NTH_Y_UP 		 =0x08,
} LIS3MDL_MAG_NTH_Y_t;

#define LIS3MDL_MAG_NTH_X_MASK  	0x10
typedef enum {
  	LIS3MDL_MAG_NTH_X_DOWN 		 =0x00,
  	LIS3MDL_MAG_NTH_X_UP 		 =0x10,
} LIS3MDL_MAG_NTH_X_t;

#define LIS3MDL_MAG_PTH_Z_MASK  	0x20
typedef enum {
  	LIS3MDL_MAG_PTH_Z_DOWN 		 =0x00,
  	LIS3MDL_MAG_PTH_Z_UP 		 =0x20,
} LIS3MDL_MAG_PTH_Z_t;

#define LIS3MDL_MAG_PTH_Y_MASK  	0x40
typedef enum {
  	LIS3MDL_MAG_PTH_Y_DOWN 		 =0x00,
  	LIS3MDL_MAG_PTH_Y_UP 		 =0x40,
} LIS3MDL_MAG_PTH_Y_t;

#define LIS3MDL_MAG_PTH_X_MASK  	0x80
typedef enum {
  	LIS3MDL_MAG_PTH_X_DOWN 		 =0x00,
  	LIS3MDL_MAG_PTH_X_UP 		 =0x80,
} LIS3MDL_MAG_PTH_X_t;

#ifdef __cplusplus
}
#endif

#endif // LIS3MDL_MAG_REGISTER_DEF_H
