/**
 ******************************************************************************
 * @file    vl53l4cd_api.h
 * @author  STMicroelectronics
 * @version V1.0.0
 * @date    29 November 2021
 * @brief   Header file for the VL53L4CD main structures.
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
 * Based on STMicroelectronics VL53L4CD driver
 * Modified by Neon Smart Studio for W-Link
 */

#ifndef VL53L4CD_DEF_H
#define VL53L4CD_DEF_H

typedef enum VL53L4CD_OpResult_t
{
    VL53L4CD_OK = 0,

    /* Generic Errors */
    VL53L4CD_NotInit            = -1,
    VL53L4CD_InvalidParameter   = -2,
    VL53L4CD_MemoryError        = -3,
    VL53L4CD_MutexTimeout       = -4,
    VL53L4CD_SlaveTimeout       = -5,
    VL53L4CD_IO_Error           = -6,
    VL53L4CD_Unsupport          = -7,
}VL53L4CD_OpResult;

typedef struct {

  /* Status of measurements. If the status is equal to 0, the data are valid*/
  uint8_t range_status;
  /* Measured distance in millimeters */
  uint16_t distance_mm;
  /* Ambient noise in kcps */
  uint16_t ambient_rate_kcps;
  /* Ambient noise in kcps/SPAD */
  uint16_t ambient_per_spad_kcps;
  /* Measured signal of the target in kcps */
  uint16_t signal_rate_kcps;
  /* Measured signal of the target in kcps/SPAD */
  uint16_t signal_per_spad_kcps;
  /* Number of SPADs enabled */
  uint16_t number_of_spad;
  /* Estimated measurements std deviation in mm */
  uint16_t sigma_mm;
} VL53L4CD_Result_t;

#endif  //VL53L4CD_DEF_H
