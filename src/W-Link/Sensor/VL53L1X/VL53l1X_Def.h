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

/**
 * @file vl53l1_error_codes.h
 *
 * @brief Error Code definitions for VL53L1X API.
 *
 */

#ifndef VL53l1X_DEF_H
#define VL53l1X_DEF_H

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
