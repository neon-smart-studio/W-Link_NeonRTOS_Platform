/*
* Copyright (c) 2017, STMicroelectronics - All Rights Reserved
*
* This file is part of VL53L1X Core and is dual licensed,
* either 'STMicroelectronics
* Proprietary license'
* or 'BSD 3-clause "New" or "Revised" License' , at your option.
*
********************************************************************************
*
* 'STMicroelectronics Proprietary license'
*
********************************************************************************
*
* License terms: STMicroelectronics Proprietary in accordance with licensing
* terms at www.st.com/sla0081
*
* STMicroelectronics confidential
* Reproduction and Communication of this document is strictly prohibited unless
* specifically authorized in writing by STMicroelectronics.
*
*
********************************************************************************
*
* Alternatively, VL53L1X Core may be distributed under the terms of
* 'BSD 3-clause "New" or "Revised" License', in which case the following
* provisions apply instead of the ones mentioned above :
*
********************************************************************************
*
* License terms: BSD 3-clause "New" or "Revised" License.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
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
*
********************************************************************************
*
*/
/*
 * Based on STMicroelectronics VL53L1X driver
 * Modified by Neon Smart Studio for W-Link
 */

#ifndef VL53l1X_DEF_H
#define VL53l1X_DEF_H

#define ALGO__PART_TO_PART_RANGE_OFFSET_MM	0x001E
#define MM_CONFIG__INNER_OFFSET_MM			0x0020
#define MM_CONFIG__OUTER_OFFSET_MM 			0x0022

#define VL53L1X_I2C_SLAVE__DEVICE_ADDRESS					0x0001
#define VL53L1X_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND        0x0008
#define ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS 		0x0016
#define ALGO__CROSSTALK_COMPENSATION_X_PLANE_GRADIENT_KCPS 	0x0018
#define ALGO__CROSSTALK_COMPENSATION_Y_PLANE_GRADIENT_KCPS 	0x001A
#define ALGO__PART_TO_PART_RANGE_OFFSET_MM					0x001E
#define MM_CONFIG__INNER_OFFSET_MM							0x0020
#define MM_CONFIG__OUTER_OFFSET_MM 							0x0022
#define GPIO_HV_MUX__CTRL									0x0030
#define GPIO__TIO_HV_STATUS       							0x0031
#define SYSTEM__INTERRUPT_CONFIG_GPIO 						0x0046
#define PHASECAL_CONFIG__TIMEOUT_MACROP     				0x004B
#define RANGE_CONFIG__TIMEOUT_MACROP_A_HI   				0x005E
#define RANGE_CONFIG__VCSEL_PERIOD_A        				0x0060
#define RANGE_CONFIG__VCSEL_PERIOD_B						0x0063
#define RANGE_CONFIG__TIMEOUT_MACROP_B_HI  					0x0061
#define RANGE_CONFIG__TIMEOUT_MACROP_B_LO  					0x0062
#define RANGE_CONFIG__SIGMA_THRESH 							0x0064
#define RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS			0x0066
#define RANGE_CONFIG__VALID_PHASE_HIGH      				0x0069
#define VL53L1X_SYSTEM__INTERMEASUREMENT_PERIOD				0x006C
#define SYSTEM__THRESH_HIGH 								0x0072
#define SYSTEM__THRESH_LOW 									0x0074
#define SD_CONFIG__WOI_SD0                  				0x0078
#define SD_CONFIG__INITIAL_PHASE_SD0        				0x007A
#define ROI_CONFIG__USER_ROI_CENTRE_SPAD					0x007F
#define ROI_CONFIG__USER_ROI_REQUESTED_GLOBAL_XY_SIZE		0x0080
#define SYSTEM__SEQUENCE_CONFIG								0x0081
#define VL53L1X_SYSTEM__GROUPED_PARAMETER_HOLD 				0x0082
#define SYSTEM__INTERRUPT_CLEAR       						0x0086
#define SYSTEM__MODE_START                 					0x0087
#define VL53L1X_RESULT__RANGE_STATUS							0x0089
#define VL53L1X_RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0		0x008C
#define RESULT__AMBIENT_COUNT_RATE_MCPS_SD					0x0090
#define VL53L1X_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0				0x0096
#define VL53L1X_RESULT__PEAK_SIGNAL_COUNT_RATE_CROSSTALK_CORRECTED_MCPS_SD0 	0x0098
#define VL53L1X_RESULT__OSC_CALIBRATE_VAL					0x00DE
#define VL53L1X_FIRMWARE__SYSTEM_STATUS                      0x00E5
#define VL53L1X_IDENTIFICATION__MODEL_ID                     0x010F
#define VL53L1X_ROI_CONFIG__MODE_ROI_CENTRE_SPAD				0x013E

typedef enum VL53L1X_OpResult_t
{
    VL53L1X_OK = 0,

    /* Generic Errors */
    VL53L1X_NotInit            = -1,
    VL53L1X_InvalidParameter   = -2,
    VL53L1X_MemoryError        = -3,
    VL53L1X_MutexTimeout       = -4,
    VL53L1X_SlaveTimeout       = -5,
    VL53L1X_IO_Error           = -6,
    VL53L1X_Unsupport          = -7,

    /* Generic Warnings */
    VL53L1X_CalibrationWarning = -8,
    VL53L1X_MinClipped         = -9,

    /* VL53L1X Specific Errors */
    VL53L1X_Undefined                  = -10,
    VL53L1X_RangeError                 = -11,
    VL53L1X_ModeNotSupported           = -12,
    VL53L1X_BufferTooSmall             = -13,
    VL53L1X_CommsBufferTooSmall        = -14,
    VL53L1X_GpioNotExisting            = -15,
    VL53L1X_GpioFunctionNotSupported   = -16,
    VL53L1X_InvalidCommand             = -17,
    VL53L1X_DivisionByZero             = -18,

    VL53L1X_RefSpadInitFail            = -19,
    VL53L1X_GphSyncCheckFail           = -20,
    VL53L1X_StreamCountCheckFail       = -21,
    VL53L1X_GphIdCheckFail             = -22,
    VL53L1X_ZoneStreamCountCheckFail   = -23,
    VL53L1X_ZoneGphIdCheckFail         = -24,

    VL53L1X_XTalkExtractionNoSampleFail       = -25,
    VL53L1X_XTalkExtractionSigmaLimitFail     = -26,

    VL53L1X_OffsetCalNoSampleFail             = -27,
    VL53L1X_OffsetCalNoSpadsEnabledFail       = -28,
    VL53L1X_ZoneCalNoSampleFail               = -29,

    VL53L1X_TuningParmKeyMismatch             = -30,

    /* VL53L1X Warnings */
    VL53L1X_WarningRefSpadCharNotEnoughSpads  = -31,
    VL53L1X_WarningRefSpadCharRateTooHigh     = -32,
    VL53L1X_WarningRefSpadCharRateTooLow      = -33,

    VL53L1X_WarningOffsetCalMissingSamples    = -34,
    VL53L1X_WarningOffsetCalSigmaTooHigh      = -35,
    VL53L1X_WarningOffsetCalRateTooHigh       = -36,
    VL53L1X_WarningOffsetCalSpadCountTooLow   = -37,

    VL53L1X_WarningZoneCalMissingSamples      = -38,
    VL53L1X_WarningZoneCalSigmaTooHigh        = -39,
    VL53L1X_WarningZoneCalRateTooHigh         = -40,

    VL53L1X_WarningXTalkMissingSamples        = -41,
    VL53L1X_WarningXTalkNoSamplesForGradient  = -42,
    VL53L1X_WarningXTalkSigmaLimitForGradient = -43,

    VL53L1X_NotImplemented                    = -44,

    VL53L1X_PlatformSpecificStart                = -60,

} VL53L1X_OpResult;

#endif // VL53l1X_DEF_H
