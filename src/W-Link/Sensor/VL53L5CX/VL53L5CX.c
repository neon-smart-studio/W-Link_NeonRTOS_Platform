/**
 ******************************************************************************
 * @file    vl53l5cx_api.cpp
 * @author  STMicroelectronics
 * @version V1.0.0
 * @date    11 November 2021
 * @brief   Implementation of the VL53L5CX APIs.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2021 STMicroelectronics</center></h2>
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
 * Based on STMicroelectronics VL53L5CX driver
 * Modified by Neon Smart Studio for W-Link
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "VL53L5CX_FW.h"

#include "VL53L5CX_Def.h"
#include "VL53L5CX_Global.h"
#include "VL53L5CX_Utils.h"
#include "VL53L5CX_IO.h"
#include "VL53L5CX.h"

/**
 * @brief Definitions for Range results block headers
 */

#if VL53L5CX_NB_TARGET_PER_ZONE == 1

  #define VL53L5CX_START_BH     ((uint32_t)0x0000000DU)
  #define VL53L5CX_METADATA_BH      ((uint32_t)0x54B400C0U)
  #define VL53L5CX_COMMONDATA_BH      ((uint32_t)0x54C00040U)
  #define VL53L5CX_AMBIENT_RATE_BH    ((uint32_t)0x54D00104U)
  #define VL53L5CX_SPAD_COUNT_BH      ((uint32_t)0x55D00404U)
  #define VL53L5CX_NB_TARGET_DETECTED_BH          ((uint32_t)0xCF7C0401U)
  #define VL53L5CX_SIGNAL_RATE_BH     ((uint32_t)0xCFBC0404U)
  #define VL53L5CX_RANGE_SIGMA_MM_BH    ((uint32_t)0xD2BC0402U)
  #define VL53L5CX_DISTANCE_BH      ((uint32_t)0xD33C0402U)
  #define VL53L5CX_REFLECTANCE_BH     ((uint32_t)0xD43C0401U)
  #define VL53L5CX_TARGET_STATUS_BH   ((uint32_t)0xD47C0401U)
  #define VL53L5CX_MOTION_DETECT_BH   ((uint32_t)0xCC5008C0U)

  #define VL53L5CX_METADATA_IDX     ((uint16_t)0x54B4U)
  #define VL53L5CX_SPAD_COUNT_IDX     ((uint16_t)0x55D0U)
  #define VL53L5CX_AMBIENT_RATE_IDX   ((uint16_t)0x54D0U)
  #define VL53L5CX_NB_TARGET_DETECTED_IDX   ((uint16_t)0xCF7CU)
  #define VL53L5CX_SIGNAL_RATE_IDX    ((uint16_t)0xCFBCU)
  #define VL53L5CX_RANGE_SIGMA_MM_IDX   ((uint16_t)0xD2BCU)
  #define VL53L5CX_DISTANCE_IDX     ((uint16_t)0xD33CU)
  #define VL53L5CX_REFLECTANCE_EST_PC_IDX   ((uint16_t)0xD43CU)
  #define VL53L5CX_TARGET_STATUS_IDX    ((uint16_t)0xD47CU)
  #define VL53L5CX_MOTION_DETEC_IDX   ((uint16_t)0xCC50U)

#else
  #define VL53L5CX_START_BH     ((uint32_t)0x0000000DU)
  #define VL53L5CX_METADATA_BH      ((uint32_t)0x54B400C0U)
  #define VL53L5CX_COMMONDATA_BH      ((uint32_t)0x54C00040U)
  #define VL53L5CX_AMBIENT_RATE_BH    ((uint32_t)0x54D00104U)
  #define VL53L5CX_NB_TARGET_DETECTED_BH    ((uint32_t)0x57D00401U)
  #define VL53L5CX_SPAD_COUNT_BH      ((uint32_t)0x55D00404U)
  #define VL53L5CX_SIGNAL_RATE_BH     ((uint32_t)0x58900404U)
  #define VL53L5CX_RANGE_SIGMA_MM_BH    ((uint32_t)0x64900402U)
  #define VL53L5CX_DISTANCE_BH      ((uint32_t)0x66900402U)
  #define VL53L5CX_REFLECTANCE_BH     ((uint32_t)0x6A900401U)
  #define VL53L5CX_TARGET_STATUS_BH   ((uint32_t)0x6B900401U)
  #define VL53L5CX_MOTION_DETECT_BH   ((uint32_t)0xCC5008C0U)

  #define VL53L5CX_METADATA_IDX     ((uint16_t)0x54B4U)
  #define VL53L5CX_SPAD_COUNT_IDX     ((uint16_t)0x55D0U)
  #define VL53L5CX_AMBIENT_RATE_IDX   ((uint16_t)0x54D0U)
  #define VL53L5CX_NB_TARGET_DETECTED_IDX   ((uint16_t)0x57D0U)
  #define VL53L5CX_SIGNAL_RATE_IDX    ((uint16_t)0x5890U)
  #define VL53L5CX_RANGE_SIGMA_MM_IDX   ((uint16_t)0x6490U)
  #define VL53L5CX_DISTANCE_IDX     ((uint16_t)0x6690U)
  #define VL53L5CX_REFLECTANCE_EST_PC_IDX   ((uint16_t)0x6A90U)
  #define VL53L5CX_TARGET_STATUS_IDX    ((uint16_t)0x6B90U)
  #define VL53L5CX_MOTION_DETEC_IDX   ((uint16_t)0xCC50U)
#endif

/**
 * @brief This buffer is used to get NVM data.
 */

const uint8_t VL53L5CX_GET_NVM_CMD[] = {
  0x54, 0x00, 0x00, 0x40,
  0x9E, 0x14, 0x00, 0xC0,
  0x9E, 0x20, 0x01, 0x40,
  0x9E, 0x34, 0x00, 0x40,
  0x9E, 0x38, 0x04, 0x04,
  0x9F, 0x38, 0x04, 0x02,
  0x9F, 0xB8, 0x01, 0x00,
  0x9F, 0xC8, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x0F,
  0x02, 0x02, 0x00, 0x24
};

static VL53L5CX_OpResult VL53L5CX_Poll_For_Answer(uint8_t size, uint8_t pos, uint16_t address, uint8_t mask, uint8_t expected_value)
{
  VL53L5CX_OpResult status;
  uint8_t timeout = 0;

  do {
    status = VL53L5CX_IO_Read_Bytes(address, temp_buffer, size);
    if(status < VL53L5CX_OK)
    {
      return status;
    }

    NeonRTOS_Sleep(10);

    if (timeout >= (uint8_t)200)
    { /* 2s timeout */
      return VL53L5CX_SlaveTimeout;
    }
    else if ((size >= (uint8_t)4) && (temp_buffer[2] >= (uint8_t)0x7f))
    {
      return VL53L5CX_MCU_Error;
    }
    else
    {
      timeout++;
    }

  } while ((temp_buffer[pos] & mask) != expected_value);

  return VL53L5CX_OK;
}

static VL53L5CX_OpResult VL53L5CX_Send_Offset_Data(uint8_t resolution)
{
  uint8_t status;
  uint32_t signal_grid[64];
  int16_t range_grid[64];
  uint8_t dss_4x4[] = {0x0F, 0x04, 0x04, 0x00, 0x08, 0x10, 0x10, 0x07};
  uint8_t footer[] = {0x00, 0x00, 0x00, 0x0F, 0x03, 0x01, 0x01, 0xE4};
  int8_t i, j;
  uint16_t k;

  (void)memcpy(temp_buffer, offset_data, VL53L5CX_OFFSET_BUFFER_SIZE);

  /* Data extrapolation is required for 4X4 offset */
  if (resolution == (uint8_t)VL53L5CX_RESOLUTION_4X4)
  {
    (void)memcpy(&(temp_buffer[0x10]), dss_4x4, sizeof(dss_4x4));

    VL53L5CX_SwapBuffer(temp_buffer, VL53L5CX_OFFSET_BUFFER_SIZE);

    (void)memcpy(signal_grid, &(temp_buffer[0x3C]), sizeof(signal_grid));
    (void)memcpy(range_grid, &(temp_buffer[0x140]), sizeof(range_grid));

    for (j = 0; j < (int8_t)4; j++)
    {
      for (i = 0; i < (int8_t)4 ; i++)
      {
        signal_grid[i + (4 * j)] =
          (signal_grid[(2 * i) + (16 * j) + (int8_t)0]
           + signal_grid[(2 * i) + (16 * j) + (int8_t)1]
           + signal_grid[(2 * i) + (16 * j) + (int8_t)8]
           + signal_grid[(2 * i) + (16 * j) + (int8_t)9])
          / (uint32_t)4;
        range_grid[i + (4 * j)] =
          (range_grid[(2 * i) + (16 * j)]
           + range_grid[(2 * i) + (16 * j) + 1]
           + range_grid[(2 * i) + (16 * j) + 8]
           + range_grid[(2 * i) + (16 * j) + 9])
          / (int16_t)4;
      }
    }

    (void)memset(&range_grid[0x10], 0, (uint16_t)96);
    (void)memset(&signal_grid[0x10], 0, (uint16_t)192);

    (void)memcpy(&(temp_buffer[0x3C]), signal_grid, sizeof(signal_grid));
    (void)memcpy(&(temp_buffer[0x140]), range_grid, sizeof(range_grid));

    VL53L5CX_SwapBuffer(temp_buffer, VL53L5CX_OFFSET_BUFFER_SIZE);
  }

  for (k = 0; k < (VL53L5CX_OFFSET_BUFFER_SIZE - (uint16_t)4); k++) {
    temp_buffer[k] = temp_buffer[k + (uint16_t)8];
  }

  (void)memcpy(&(temp_buffer[0x1E0]), footer, 8);

  status = VL53L5CX_IO_Write_Bytes(0x2e18, temp_buffer, VL53L5CX_OFFSET_BUFFER_SIZE);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  status = VL53L5CX_Poll_For_Answer(4, 1, VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}

static VL53L5CX_OpResult VL53L5CX_Send_Xtalk_Data(uint8_t resolution)
{
  VL53L5CX_OpResult status;
  uint8_t res4x4[] = {0x0F, 0x04, 0x04, 0x17, 0x08, 0x10, 0x10, 0x07};
  uint8_t dss_4x4[] = {0x00, 0x78, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08};
  uint8_t profile_4x4[] = {0xA0, 0xFC, 0x01, 0x00};
  uint32_t signal_grid[64];
  int8_t i, j;

  (void)memcpy(temp_buffer, (uint8_t *)xtalk_data, VL53L5CX_XTALK_BUFFER_SIZE);

  /* Data extrapolation is required for 4X4 Xtalk */
  if (resolution == (uint8_t)VL53L5CX_RESOLUTION_4X4)
  {
    (void)memcpy(&(temp_buffer[0x8]), res4x4, sizeof(res4x4));
    (void)memcpy(&(temp_buffer[0x020]), dss_4x4, sizeof(dss_4x4));

    VL53L5CX_SwapBuffer(temp_buffer, VL53L5CX_XTALK_BUFFER_SIZE);

    (void)memcpy(signal_grid, &(temp_buffer[0x34]), sizeof(signal_grid));

    for (j = 0; j < (int8_t)4; j++) {
      for (i = 0; i < (int8_t)4 ; i++) {
        signal_grid[i + (4 * j)] =
          (signal_grid[(2 * i) + (16 * j) + 0]
           + signal_grid[(2 * i) + (16 * j) + 1]
           + signal_grid[(2 * i) + (16 * j) + 8]
           + signal_grid[(2 * i) + (16 * j) + 9]) / (uint32_t)4;
      }
    }
    (void)memset(&signal_grid[0x10], 0, (uint32_t)192);
    (void)memcpy(&(temp_buffer[0x34]), signal_grid, sizeof(signal_grid));

    VL53L5CX_SwapBuffer(temp_buffer, VL53L5CX_XTALK_BUFFER_SIZE);

    (void)memcpy(&(temp_buffer[0x134]), profile_4x4, sizeof(profile_4x4));
    (void)memset(&(temp_buffer[0x078]), 0, (uint32_t)4 * sizeof(uint8_t));
  }

  status = VL53L5CX_IO_Write_Bytes(0x2cf8, temp_buffer, VL53L5CX_XTALK_BUFFER_SIZE);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Poll_For_Answer(4, 1, VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Init()
{
   return VL53L5CX_IO_Init();
}

VL53L5CX_OpResult VL53L5CX_DeInit()
{
   return VL53L5CX_IO_DeInit();
}

VL53L5CX_OpResult VL53L5CX_Power_Off()
{
   return VL53L5CX_IO_Power_Off();
}

VL53L5CX_OpResult VL53L5CX_Power_On()
{
   return VL53L5CX_IO_Power_On();
}

VL53L5CX_OpResult VL53L5CX_Set_I2C_Address(uint8_t new_address)
{
   return VL53L5CX_IO_Set_I2C_Address(new_address);
}

VL53L5CX_OpResult VL53L5CX_SensorInit()
{
  VL53L5CX_OpResult status;
  uint8_t tmp;
  uint8_t pipe_ctrl[] = {VL53L5CX_NB_TARGET_PER_ZONE, 0x00, 0x01, 0x00};
  uint32_t single_range = 0x01;

  /* SW reboot sequence */
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x0009, 0x04);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x000F, 0x40);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x000A, 0x03);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Read_Byte(0x7FFF, &tmp);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x000C, 0x01);
  if(status < VL53L5CX_OK) { return status; }

  status = VL53L5CX_IO_Write_Byte(0x0101, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x0102, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x010A, 0x01);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x4002, 0x01);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x4002, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x010A, 0x03);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x0103, 0x01);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x000C, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x000F, 0x43);
  if(status < VL53L5CX_OK) { return status; }

  NeonRTOS_Sleep(1);

  status = VL53L5CX_IO_Write_Byte(0x000F, 0x40);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x000A, 0x01);
  if(status < VL53L5CX_OK) { return status; }

  NeonRTOS_Sleep(100);

  /* Wait for sensor booted (several ms required to get sensor ready ) */
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x00);
  if(status < VL53L5CX_OK) { return status; }

  status = VL53L5CX_Poll_For_Answer(1, 0, 0x06, 0xff, 1);
  if(status < VL53L5CX_OK) { return status; }

  status = VL53L5CX_IO_Write_Byte(0x000E, 0x01);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x02);
  if(status < VL53L5CX_OK) { return status; }

  /* Enable FW access */
  status = VL53L5CX_IO_Write_Byte(0x03, 0x0D);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x01);
  if(status < VL53L5CX_OK) { return status; }

  status = VL53L5CX_Poll_For_Answer(1, 0, 0x21, 0x10, 0x10);
  if(status < VL53L5CX_OK) { return status; }

  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x00);
  if(status < VL53L5CX_OK) { return status; }

  /* Enable host access to GO1 */
  status = VL53L5CX_IO_Write_Byte(0x0C, 0x01);
  if(status < VL53L5CX_OK) { return status; }

  /* Power ON status */
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x101, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x102, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x010A, 0x01);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x4002, 0x01);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x4002, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x010A, 0x03);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x103, 0x01);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x400F, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x21A, 0x43);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x21A, 0x03);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x21A, 0x01);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x21A, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x219, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x21B, 0x00);
  if(status < VL53L5CX_OK) { return status; }

  /* Wake up MCU */
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x0C, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x01);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x20, 0x07);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x20, 0x06);
  if(status < VL53L5CX_OK) { return status; }

  /* Download FW into VL53L5 */
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x09);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Bytes(0, (uint8_t *)&VL53L5CX_FIRMWARE[0], 0x8000);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x0a);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Bytes(0, (uint8_t *)&VL53L5CX_FIRMWARE[0x8000], 0x8000);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x0b);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Bytes(0, (uint8_t *)&VL53L5CX_FIRMWARE[0x10000], 0x5000);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x01);
  if(status < VL53L5CX_OK) { return status; }

  /* Check if FW correctly downloaded */
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x02);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x03, 0x0D);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x01);
  if(status < VL53L5CX_OK) { return status; }

  status = VL53L5CX_Poll_For_Answer(1, 0, 0x21, 0x10, 0x10);
  if(status < VL53L5CX_OK) { return status; }

  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x0C, 0x01);
  if(status < VL53L5CX_OK) { return status; }

  /* Reset MCU and wait boot */
  status = VL53L5CX_IO_Write_Byte(0x7FFF, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x114, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x115, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x116, 0x42);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x117, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x0B, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x0C, 0x00);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_IO_Write_Byte(0x0B, 0x01);
  if(status < VL53L5CX_OK) { return status; }

  status = VL53L5CX_Poll_For_Answer(1, 0, 0x06, 0xff, 0x00);
  if(status < VL53L5CX_OK) { return status; }

  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x02);
  if(status < VL53L5CX_OK) { return status; }

  /* Get offset NVM data and store them into the offset buffer */
  status = VL53L5CX_IO_Write_Bytes(0x2fd8, (uint8_t *)VL53L5CX_GET_NVM_CMD, sizeof(VL53L5CX_GET_NVM_CMD));
  if(status < VL53L5CX_OK) { return status; }

  status = VL53L5CX_Poll_For_Answer(4, 0, VL53L5CX_UI_CMD_STATUS, 0xff, 2);
  if(status < VL53L5CX_OK) { return status; }

  status = VL53L5CX_IO_Read_Bytes(VL53L5CX_UI_CMD_START, temp_buffer, VL53L5CX_NVM_DATA_SIZE);
  if(status < VL53L5CX_OK) { return status; }

  (void)memcpy(offset_data, temp_buffer, VL53L5CX_OFFSET_BUFFER_SIZE);

  status = VL53L5CX_Send_Offset_Data(VL53L5CX_RESOLUTION_4X4);
  if(status < VL53L5CX_OK) { return status; }

  /* Set default Xtalk shape. Send Xtalk to sensor */
  (void)memcpy(xtalk_data, (uint8_t *)VL53L5CX_DEFAULT_XTALK, VL53L5CX_XTALK_BUFFER_SIZE);

  status = VL53L5CX_Send_Xtalk_Data(VL53L5CX_RESOLUTION_4X4);
  if(status < VL53L5CX_OK) { return status; }

  /* Send default configuration to VL53L5CX firmware */
  status = VL53L5CX_IO_Write_Bytes(0x2c34, VL53L5CX_DEFAULT_CONFIGURATION, VL53L5CX_CONFIGURATION_SIZE);
  if(status < VL53L5CX_OK) { return status; }
  status = VL53L5CX_Poll_For_Answer(4, 1, VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);
  if(status < VL53L5CX_OK) { return status; }

  status = VL53L5CX_DCI_Write_Data((uint8_t *)&pipe_ctrl, VL53L5CX_DCI_PIPE_CONTROL, (uint16_t)sizeof(pipe_ctrl));
  if(status < VL53L5CX_OK) { return status; }

#if VL53L5CX_NB_TARGET_PER_ZONE != 1
  tmp = VL53L5CX_NB_TARGET_PER_ZONE;
  status = VL53L5CX_DCI_Replace_Data(temp_buffer, VL53L5CX_DCI_FW_NB_TARGET, 16, (uint8_t *)&tmp, 1, 0x0C);
  if(status < VL53L5CX_OK) { return status; }
#endif

  status = VL53L5CX_DCI_Write_Data((uint8_t *)&single_range, VL53L5CX_DCI_SINGLE_RANGE, (uint16_t)sizeof(single_range));
  if(status < VL53L5CX_OK) { return status; }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Is_Alive(int8_t *p_is_alive)
{
  VL53L5CX_OpResult status;
  uint8_t device_id, revision_id;

  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x00);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_IO_Read_Byte(0, &device_id);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_IO_Read_Byte(1, &revision_id);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x02);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  if ((device_id == (uint8_t)0xF0) && (revision_id == (uint8_t)0x02)) {
    *p_is_alive = 1;
  } else {
    *p_is_alive = 0;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Get_Power_Mode(uint8_t *p_power_mode)
{
  VL53L5CX_OpResult status;
  uint8_t tmp;

  status = VL53L5CX_IO_Write_Byte(0x7FFF, 0x00);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_IO_Read_Byte(0x009, &tmp);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  switch (tmp) {
    case 0x4:
      *p_power_mode = VL53L5CX_POWER_MODE_WAKEUP;
      break;
    case 0x2:
      *p_power_mode = VL53L5CX_POWER_MODE_SLEEP;

      break;
    default:
      *p_power_mode = 0;
      return VL53L5CX_Status_Error;
  }

  return VL53L5CX_IO_Write_Byte(0x7FFF, 0x02);
}

VL53L5CX_OpResult VL53L5CX_Set_Power_Mode(uint8_t power_mode)
{
  VL53L5CX_OpResult status;
  uint8_t current_power_mode;

  status = VL53L5CX_Get_Power_Mode(&current_power_mode);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  if (power_mode != current_power_mode) {
    switch (power_mode) {
      case VL53L5CX_POWER_MODE_WAKEUP:
        status = VL53L5CX_IO_Write_Byte(0x7FFF, 0x00);
        if(status < VL53L5CX_OK) { return status; }
        status = VL53L5CX_IO_Write_Byte(0x09, 0x04);
        if(status < VL53L5CX_OK) { return status; }
        status = VL53L5CX_Poll_For_Answer(1, 0, 0x06, 0x01, 1);
        if(status < VL53L5CX_OK) { return status; }
        break;

      case VL53L5CX_POWER_MODE_SLEEP:
        status = VL53L5CX_IO_Write_Byte(0x7FFF, 0x00);
        if(status < VL53L5CX_OK) { return status; }
        status = VL53L5CX_IO_Write_Byte(0x09, 0x02);
        if(status < VL53L5CX_OK) { return status; }
        status = VL53L5CX_Poll_For_Answer(1, 0, 0x06, 0x01, 0);
        if(status < VL53L5CX_OK) { return status; }
        break;

      default:
        return VL53L5CX_Status_Error;
    }

    status = VL53L5CX_IO_Write_Byte(0x7FFF, 0x02);
    if(status < VL53L5CX_OK)
    {
      return status;
    }
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Start_Ranging()
{
  VL53L5CX_OpResult status;
  uint8_t resolution;
  uint32_t i;
  uint32_t header_config[2] = {0, 0};

  union Block_header *bh_ptr;
  uint8_t cmd[] = {0x00, 0x03, 0x00, 0x00};

  status = vl53l5cx_get_resolution(&resolution);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  data_read_size = 0;
  streamcount = 255;

  /* Enable mandatory output (meta and common data) */
  uint32_t output_bh_enable[] = {
    0x00000007U,
    0x00000000U,
    0x00000000U,
    0xC0000000U
  };

  /* Send addresses of possible output */
  uint32_t output[] = {VL53L5CX_START_BH,
                       VL53L5CX_METADATA_BH,
                       VL53L5CX_COMMONDATA_BH,
                       VL53L5CX_AMBIENT_RATE_BH,
                       VL53L5CX_SPAD_COUNT_BH,
                       VL53L5CX_NB_TARGET_DETECTED_BH,
                       VL53L5CX_SIGNAL_RATE_BH,
                       VL53L5CX_RANGE_SIGMA_MM_BH,
                       VL53L5CX_DISTANCE_BH,
                       VL53L5CX_REFLECTANCE_BH,
                       VL53L5CX_TARGET_STATUS_BH,
                       VL53L5CX_MOTION_DETECT_BH
                      };

  /* Enable selected outputs in the 'platform.h' file */
#ifndef VL53L5CX_DISABLE_AMBIENT_PER_SPAD
  output_bh_enable[0] += (uint32_t)8;
#endif
#ifndef VL53L5CX_DISABLE_NB_SPADS_ENABLED
  output_bh_enable[0] += (uint32_t)16;
#endif
#ifndef VL53L5CX_DISABLE_NB_TARGET_DETECTED
  output_bh_enable[0] += (uint32_t)32;
#endif
#ifndef VL53L5CX_DISABLE_SIGNAL_PER_SPAD
  output_bh_enable[0] += (uint32_t)64;
#endif
#ifndef VL53L5CX_DISABLE_RANGE_SIGMA_MM
  output_bh_enable[0] += (uint32_t)128;
#endif
#ifndef VL53L5CX_DISABLE_DISTANCE_MM
  output_bh_enable[0] += (uint32_t)256;
#endif
#ifndef VL53L5CX_DISABLE_REFLECTANCE_PERCENT
  output_bh_enable[0] += (uint32_t)512;
#endif
#ifndef VL53L5CX_DISABLE_TARGET_STATUS
  output_bh_enable[0] += (uint32_t)1024;
#endif
#ifndef VL53L5CX_DISABLE_MOTION_INDICATOR
  output_bh_enable[0] += (uint32_t)2048;
#endif

  /* Update data size */
  for (i = 0; i < (uint32_t)(sizeof(output) / sizeof(uint32_t)); i++) {
    if ((output[i] == (uint8_t)0)
        || ((output_bh_enable[i / (uint32_t)32]
             & ((uint32_t)1 << (i % (uint32_t)32))) == (uint32_t)0)) {
      continue;
    }

    bh_ptr = (union Block_header *) & (output[i]);
    if (((uint8_t)bh_ptr->type >= (uint8_t)0x1)
        && ((uint8_t)bh_ptr->type < (uint8_t)0x0d)) {
      if ((bh_ptr->idx >= (uint16_t)0x54d0)
          && (bh_ptr->idx < (uint16_t)(0x54d0 + 960))) {
        bh_ptr->size = resolution;
      } else {
        bh_ptr->size = (uint8_t)(resolution
                                 * (uint8_t)VL53L5CX_NB_TARGET_PER_ZONE);
      }
      data_read_size += bh_ptr->type * bh_ptr->size;
    } else {
      data_read_size += bh_ptr->size;
    }
    data_read_size += (uint32_t)4;
  }
  data_read_size += (uint32_t)20;

  status = VL53L5CX_DCI_Write_Data((uint8_t *) & (output), VL53L5CX_DCI_OUTPUT_LIST, (uint16_t)sizeof(output));
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  header_config[0] = data_read_size;
  header_config[1] = i + (uint32_t)1;

  status = VL53L5CX_DCI_Write_Data((uint8_t *) & (header_config), VL53L5CX_DCI_OUTPUT_CONFIG, (uint16_t)sizeof(header_config));
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  status = VL53L5CX_DCI_Write_Data((uint8_t *) & (output_bh_enable), VL53L5CX_DCI_OUTPUT_ENABLES, (uint16_t)sizeof(output_bh_enable));
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  /* Start xshut bypass (interrupt mode) */
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x00);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_IO_Write_Byte(0x09, 0x05);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x02);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  /* Start ranging session */
  status = VL53L5CX_IO_Write_Bytes(VL53L5CX_UI_CMD_END - (uint16_t)(4 - 1), (uint8_t *)cmd, sizeof(cmd));
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Poll_For_Answer(4, 1, VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Stop_Ranging()
{
  uint8_t tmp = 0, status;
  uint16_t timeout = 0;
  uint32_t auto_stop_flag = 0;

  status = VL53L5CX_IO_Read_Bytes(0x2FFC, (uint8_t *)&auto_stop_flag, 4);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  if (auto_stop_flag != (uint32_t)0x4FF) {
    status = VL53L5CX_IO_Write_Byte(0x7fff, 0x00);
    if(status < VL53L5CX_OK)
    {
      return status;
    }

    /* Provoke MCU stop */
    status = VL53L5CX_IO_Write_Byte(0x15, 0x16);
    if(status < VL53L5CX_OK)
    {
      return status;
    }
    status = VL53L5CX_IO_Write_Byte(0x14, 0x01);
    if(status < VL53L5CX_OK)
    {
      return status;
    }

    /* Poll for G02 status 0 MCU stop */
    while (((tmp & (uint8_t)0x80) >> 7) == (uint8_t)0x00)
    {
      status = VL53L5CX_IO_Read_Byte(0x6, &tmp);
      if(status < VL53L5CX_OK)
      {
        return status;
      }

      NeonRTOS_Sleep(10);

      timeout++;
      /* Timeout reached after 5 seconds */
      if (timeout > (uint16_t)500) {
        return VL53L5CX_Status_Error;
      }
    }
  }

  /* Undo MCU stop */
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x00);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_IO_Write_Byte(0x14, 0x00);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_IO_Write_Byte(0x15, 0x00);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  /* Stop xshut bypass */
  status = VL53L5CX_IO_Write_Byte(0x09, 0x04);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x02);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Check_Data_Ready(uint8_t *p_isReady)
{
  VL53L5CX_OpResult status;

  status = VL53L5CX_IO_Read_Bytes(0x0, temp_buffer, 4);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  if ((temp_buffer[0] != streamcount)
      && (temp_buffer[0] != (uint8_t)255)
      && (temp_buffer[1] == (uint8_t)0x5)
      && ((temp_buffer[2] & (uint8_t)0x5) == (uint8_t)0x5)
      && ((temp_buffer[3] & (uint8_t)0x10) == (uint8_t)0x10)
     ) {
    *p_isReady = (uint8_t)1;
    streamcount = temp_buffer[0];
  } else {
    *p_isReady = 0;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Get_Ranging_Data(VL53L5CX_ResultsData *p_results)
{
  VL53L5CX_OpResult status;
  union Block_header *bh_ptr;
  uint32_t i, j, msize;

  status = VL53L5CX_IO_Read_Bytes(0x0, temp_buffer, data_read_size);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  streamcount = temp_buffer[0];

  VL53L5CX_SwapBuffer(temp_buffer, (uint16_t)data_read_size);

  /* Start conversion at position 16 to avoid headers */
  for (i = (uint32_t)16; i
       < (uint32_t)data_read_size; i += (uint32_t)4) {
    bh_ptr = (union Block_header *) & (temp_buffer[i]);
    if ((bh_ptr->type > (uint32_t)0x1)
        && (bh_ptr->type < (uint32_t)0xd)) {
      msize = bh_ptr->type * bh_ptr->size;
    } else {
      msize = bh_ptr->size;
    }

    switch (bh_ptr->idx) {
#ifndef VL53L5CX_DISABLE_AMBIENT_PER_SPAD
      case VL53L5CX_AMBIENT_RATE_IDX:
        (void)memcpy(p_results->ambient_per_spad, &(temp_buffer[i + (uint32_t)4]), msize);
        break;
#endif
#ifndef VL53L5CX_DISABLE_NB_SPADS_ENABLED
      case VL53L5CX_SPAD_COUNT_IDX:
        (void)memcpy(p_results->nb_spads_enabled, &(temp_buffer[i + (uint32_t)4]), msize);
        break;
#endif
#ifndef VL53L5CX_DISABLE_NB_TARGET_DETECTED
      case VL53L5CX_NB_TARGET_DETECTED_IDX:
        (void)memcpy(p_results->nb_target_detected, &(temp_buffer[i + (uint32_t)4]), msize);
        break;
#endif
#ifndef VL53L5CX_DISABLE_SIGNAL_PER_SPAD
      case VL53L5CX_SIGNAL_RATE_IDX:
        (void)memcpy(p_results->signal_per_spad, &(temp_buffer[i + (uint32_t)4]), msize);
        break;
#endif
#ifndef VL53L5CX_DISABLE_RANGE_SIGMA_MM
      case VL53L5CX_RANGE_SIGMA_MM_IDX:
        (void)memcpy(p_results->range_sigma_mm, &(temp_buffer[i + (uint32_t)4]), msize);
        break;
#endif
#ifndef VL53L5CX_DISABLE_DISTANCE_MM
      case VL53L5CX_DISTANCE_IDX:
        (void)memcpy(p_results->distance_mm, &(temp_buffer[i + (uint32_t)4]), msize);
        break;
#endif
#ifndef VL53L5CX_DISABLE_REFLECTANCE_PERCENT
      case VL53L5CX_REFLECTANCE_EST_PC_IDX:
        (void)memcpy(p_results->reflectance, &(temp_buffer[i + (uint32_t)4]), msize);
        break;
#endif
#ifndef VL53L5CX_DISABLE_TARGET_STATUS
      case VL53L5CX_TARGET_STATUS_IDX:
        (void)memcpy(p_results->target_status, &(temp_buffer[i + (uint32_t)4]), msize);
        break;
#endif
#ifndef VL53L5CX_DISABLE_MOTION_INDICATOR
      case VL53L5CX_MOTION_DETEC_IDX:
        (void)memcpy(&p_results->motion_indicator, &(temp_buffer[i + (uint32_t)4]), msize);
        break;
#endif
      default:
        break;
    }
    i += msize;
  }

#ifndef VL53L5CX_USE_RAW_FORMAT

  /* Convert data into their real format */
#ifndef VL53L5CX_DISABLE_AMBIENT_PER_SPAD
  for (i = 0; i < (uint32_t)VL53L5CX_RESOLUTION_8X8; i++) {
    p_results->ambient_per_spad[i] /= (uint32_t)2048;
  }
#endif

  for (i = 0; i < (uint32_t)(VL53L5CX_RESOLUTION_8X8
                             *VL53L5CX_NB_TARGET_PER_ZONE); i++) {
#ifndef VL53L5CX_DISABLE_DISTANCE_MM
    p_results->distance_mm[i] /= 4;
    if (p_results->distance_mm[i] < 0) {
      p_results->distance_mm[i] = 0;
    }
#endif
#ifndef VL53L5CX_DISABLE_RANGE_SIGMA_MM
    p_results->range_sigma_mm[i] /= (uint16_t)128;
#endif
#ifndef VL53L5CX_DISABLE_SIGNAL_PER_SPAD
    p_results->signal_per_spad[i] /= (uint32_t)2048;
#endif
  }

  /* Set target status to 255 if no target is detected for this zone */
#ifndef VL53L5CX_DISABLE_NB_TARGET_DETECTED
  for (i = 0; i < (uint32_t)VL53L5CX_RESOLUTION_8X8; i++) {
    if (p_results->nb_target_detected[i] == (uint8_t)0) {
      for (j = 0; j < (uint32_t)
           VL53L5CX_NB_TARGET_PER_ZONE; j++) {
#ifndef VL53L5CX_DISABLE_TARGET_STATUS
        p_results->target_status
        [((uint32_t)VL53L5CX_NB_TARGET_PER_ZONE
          * (uint32_t)i) + j] = (uint8_t)255;
#endif
      }
    }
  }
#endif

#ifndef VL53L5CX_DISABLE_MOTION_INDICATOR
  for (i = 0; i < (uint32_t)32; i++) {
    p_results->motion_indicator.motion[i] /= (uint32_t)65535;
  }
#endif

#endif
  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Get_Resolution(uint8_t *p_resolution)
{
  VL53L5CX_OpResult status;

  status = VL53L5CX_DCI_Read_Data(temp_buffer, VL53L5CX_DCI_ZONE_CONFIG, 8);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  *p_resolution = temp_buffer[0x00] * temp_buffer[0x01];

  return status;
}

VL53L5CX_OpResult VL53L5CX_Set_Resolution(uint8_t resolution)
{
  VL53L5CX_OpResult status;

  switch (resolution) {
    case VL53L5CX_RESOLUTION_4X4:
      status = VL53L5CX_DCI_Read_Data(temp_buffer, VL53L5CX_DCI_DSS_CONFIG, 16);
      if(status < VL53L5CX_OK)
      {
        return status;
      }

      temp_buffer[0x04] = 64;
      temp_buffer[0x06] = 64;
      temp_buffer[0x09] = 4;

      status = VL53L5CX_DCI_Write_Data(temp_buffer, VL53L5CX_DCI_DSS_CONFIG, 16);
      if(status < VL53L5CX_OK)
      {
        return status;
      }

      status = VL53L5CX_DCI_Read_Data(temp_buffer, VL53L5CX_DCI_ZONE_CONFIG, 8);
      if(status < VL53L5CX_OK)
      {
        return status;
      }
      
      temp_buffer[0x00] = 4;
      temp_buffer[0x01] = 4;
      temp_buffer[0x04] = 8;
      temp_buffer[0x05] = 8;

      status = VL53L5CX_DCI_Write_Data(temp_buffer, VL53L5CX_DCI_ZONE_CONFIG, 8);
      if(status < VL53L5CX_OK)
      {
        return status;
      }
      
      break;

    case VL53L5CX_RESOLUTION_8X8:
      status = VL53L5CX_DCI_Read_Data(temp_buffer, VL53L5CX_DCI_DSS_CONFIG, 16);
      if(status < VL53L5CX_OK)
      {
        return status;
      }
      
      temp_buffer[0x04] = 16;
      temp_buffer[0x06] = 16;
      temp_buffer[0x09] = 1;

      status = VL53L5CX_DCI_Write_Data(temp_buffer, VL53L5CX_DCI_DSS_CONFIG, 16);
      if(status < VL53L5CX_OK)
      {
        return status;
      }
      
      status = VL53L5CX_DCI_Read_Data(temp_buffer, VL53L5CX_DCI_ZONE_CONFIG, 8);
      if(status < VL53L5CX_OK)
      {
        return status;
      }

      temp_buffer[0x00] = 8;
      temp_buffer[0x01] = 8;
      temp_buffer[0x04] = 4;
      temp_buffer[0x05] = 4;

      status = VL53L5CX_DCI_Write_Data(temp_buffer, VL53L5CX_DCI_ZONE_CONFIG, 8);
      if(status < VL53L5CX_OK)
      {
        return status;
      }
      
      break;

    default:
      return VL53L5CX_InvalidParameter;
  }

  status = VL53L5CX_Send_Offset_Data(resolution);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
      
  status = VL53L5CX_Send_Xtalk_Data(resolution);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Get_Ranging_Frequency_Hz(uint8_t *p_frequency_hz)
{
  VL53L5CX_OpResult status;

  status = VL53L5CX_DCI_Read_Data((uint8_t *)temp_buffer, VL53L5CX_DCI_FREQ_HZ, 4);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  *p_frequency_hz = temp_buffer[0x01];

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Set_Ranging_Frequency_Hz(uint8_t frequency_hz)
{
  VL53L5CX_OpResult status;

  status = VL53L5CX_DCI_Replace_Data(temp_buffer, VL53L5CX_DCI_FREQ_HZ, 4, (uint8_t *)&frequency_hz, 1, 0x01);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Get_Integration_Time_mS(uint32_t *p_time_ms)
{
  VL53L5CX_OpResult status;

  status = VL53L5CX_DCI_Read_Data((uint8_t *)temp_buffer, VL53L5CX_DCI_INT_TIME, 20);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  (void)memcpy(p_time_ms, &(temp_buffer[0x0]), 4);

  *p_time_ms /= (uint32_t)1000;

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Set_Integration_Time_mS(uint32_t integration_time_ms)
{
  VL53L5CX_OpResult status;
  uint32_t integration = integration_time_ms;

  /* Integration time must be between 2ms and 1000ms */
  if ((integration < (uint32_t)2) || (integration > (uint32_t)1000)) {
    return VL53L5CX_InvalidParameter;
  }

  integration *= (uint32_t)1000;

  status = VL53L5CX_DCI_Replace_Data(temp_buffer, VL53L5CX_DCI_INT_TIME, 20, (uint8_t *)&integration, 4, 0x00);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Get_Sharpener_Percent(uint8_t *p_sharpener_percent)
{
  VL53L5CX_OpResult status;

  status = VL53L5CX_DCI_Read_Data(temp_buffer, VL53L5CX_DCI_SHARPENER, 16);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  *p_sharpener_percent = (temp_buffer[0xD] * (uint8_t)100) / (uint8_t)255;

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Set_Sharpener_Percent(uint8_t sharpener_percent)
{
  VL53L5CX_OpResult status;
  uint8_t sharpener;

  if (sharpener_percent >= (uint8_t)100) {
    return VL53L5CX_InvalidParameter;
  }

  sharpener = (sharpener_percent * (uint8_t)255) / (uint8_t)100;

  status = VL53L5CX_DCI_Replace_Data(temp_buffer, VL53L5CX_DCI_SHARPENER, 16, (uint8_t *)&sharpener, 1, 0xD);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Get_Target_Order(uint8_t *p_target_order)
{
  VL53L5CX_OpResult status;

  status = VL53L5CX_DCI_Read_Data((uint8_t *)temp_buffer, VL53L5CX_DCI_TARGET_ORDER, 4);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  *p_target_order = (uint8_t)temp_buffer[0x0];

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Set_Target_Order(uint8_t target_order)
{
  VL53L5CX_OpResult status;

  if ((target_order != (uint8_t)VL53L5CX_TARGET_ORDER_CLOSEST) && (target_order != (uint8_t)VL53L5CX_TARGET_ORDER_STRONGEST))
  {
    return VL53L5CX_InvalidParameter;
  }

  status = VL53L5CX_DCI_Replace_Data(temp_buffer, VL53L5CX_DCI_TARGET_ORDER, 4, (uint8_t *)&target_order, 1, 0x0);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Get_Ranging_Mode(uint8_t *p_ranging_mode)
{
  VL53L5CX_OpResult status;

  status = VL53L5CX_DCI_Read_Data(temp_buffer, VL53L5CX_DCI_RANGING_MODE, 8);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  if (temp_buffer[0x01] == (uint8_t)0x1) {
    *p_ranging_mode = VL53L5CX_RANGING_MODE_CONTINUOUS;
  } else {
    *p_ranging_mode = VL53L5CX_RANGING_MODE_AUTONOMOUS;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_Set_Ranging_Mode(uint8_t ranging_mode)
{
  VL53L5CX_OpResult status;
  uint32_t single_range = 0x00;

  status = VL53L5CX_DCI_Read_Data(temp_buffer, VL53L5CX_DCI_RANGING_MODE, 8);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  switch (ranging_mode) {
    case VL53L5CX_RANGING_MODE_CONTINUOUS:
      temp_buffer[0x01] = 0x1;
      temp_buffer[0x03] = 0x3;
      single_range = 0x00;
      break;

    case VL53L5CX_RANGING_MODE_AUTONOMOUS:
      temp_buffer[0x01] = 0x3;
      temp_buffer[0x03] = 0x2;
      single_range = 0x01;
      break;

    default:
      return VL53L5CX_InvalidParameter;
  }

  status = VL53L5CX_DCI_Write_Data(temp_buffer, VL53L5CX_DCI_RANGING_MODE, (uint16_t)8);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  status = VL53L5CX_DCI_Write_Data((uint8_t *)&single_range, VL53L5CX_DCI_SINGLE_RANGE, (uint16_t)sizeof(single_range));
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return status;
}

VL53L5CX_OpResult VL53L5CX_DCI_Read_Data(uint8_t *data, uint32_t index, uint16_t data_size)
{
  int16_t i;
  VL53L5CX_OpResult status;
  uint32_t rd_size = (uint32_t) data_size + (uint32_t)12;
  uint8_t cmd[] = {0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x0f,
                   0x00, 0x02, 0x00, 0x08
                  };

  /* Check if tmp buffer is large enough */
  if (rd_size > (uint16_t)VL53L5CX_TEMPORARY_BUFFER_SIZE)
  {
    return VL53L5CX_MemoryError;
  }

  cmd[0] = (uint8_t)(index >> 8);
  cmd[1] = (uint8_t)(index & (uint32_t)0xff);
  cmd[2] = (uint8_t)((data_size & (uint16_t)0xff0) >> 4);
  cmd[3] = (uint8_t)((data_size & (uint16_t)0xf) << 4);

  /* Request data reading from FW */
  status = VL53L5CX_IO_Write_Bytes((VL53L5CX_UI_CMD_END - (uint16_t)11), cmd, sizeof(cmd));
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Poll_For_Answer(4, 1, VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  /* Read new data sent (4 bytes header + data_size + 8 bytes footer) */
  status = VL53L5CX_IO_Read_Bytes(VL53L5CX_UI_CMD_START, temp_buffer, rd_size);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  VL53L5CX_SwapBuffer(temp_buffer, data_size + (uint16_t)12);

  /* Copy data from FW into input structure (-4 bytes to remove header) */
  for (i = 0 ; i < (int16_t)data_size; i++) {
    data[i] = temp_buffer[i + 4];
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_DCI_Write_Data(uint8_t *data, uint32_t index, uint16_t data_size)
{
  VL53L5CX_OpResult status;
  int16_t i;
  uint32_t wr_size = (uint32_t) data_size + (uint32_t)12;

  uint8_t headers[] = {0x00, 0x00, 0x00, 0x00};
  uint8_t footer[] = {0x00, 0x00, 0x00, 0x0f, 0x05, 0x01,
                      (uint8_t)((data_size + (uint16_t)8) >> 8),
                      (uint8_t)((data_size + (uint16_t)8) & (uint8_t)0xFF)
                     };

  uint16_t address = (uint16_t)VL53L5CX_UI_CMD_END -
                     (data_size + (uint16_t)12) + (uint16_t)1;

  /* Check if cmd buffer is large enough */
  if (wr_size > (uint16_t)VL53L5CX_TEMPORARY_BUFFER_SIZE)
  {
    return VL53L5CX_MemoryError;
  }
  
  headers[0] = (uint8_t)(index >> 8);
  headers[1] = (uint8_t)(index & (uint32_t)0xff);
  headers[2] = (uint8_t)(((data_size & (uint16_t)0xff0) >> 4));
  headers[3] = (uint8_t)((data_size & (uint16_t)0xf) << 4);

  /* Copy data from structure to FW format (+4 bytes to add header) */
  VL53L5CX_SwapBuffer(data, data_size);
  for (i = (int16_t)data_size - (int16_t)1 ; i >= 0; i--) {
    temp_buffer[i + 4] = data[i];
  }

  /* Add headers and footer */
  (void)memcpy(&temp_buffer[0], headers, sizeof(headers));
  (void)memcpy(&temp_buffer[data_size + (uint16_t)4], footer, sizeof(footer));

  /* Send data to FW */
  status = VL53L5CX_IO_Write_Bytes(address, temp_buffer, (uint32_t)wr_size);
  if(status < VL53L5CX_OK)
  {
    return status;
  }
  status = VL53L5CX_Poll_For_Answer(4, 1, VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  VL53L5CX_SwapBuffer(data, data_size);

  return status;
}

VL53L5CX_OpResult VL53L5CX_DCI_Replace_Data(uint8_t *data, uint32_t index, uint16_t data_size, uint8_t *new_data, uint16_t new_data_size, uint16_t new_data_pos)
{
  VL53L5CX_OpResult status;

  status = VL53L5CX_DCI_Read_Data(data, index, data_size);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  (void)memcpy(&(data[new_data_pos]), new_data, new_data_size);

  status = VL53L5CX_DCI_Write_Data(data, index, data_size);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}
