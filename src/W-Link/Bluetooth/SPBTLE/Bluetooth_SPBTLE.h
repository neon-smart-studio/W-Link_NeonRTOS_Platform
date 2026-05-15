/*
 *******************************************************************************
 * Copyright (c) 2017, STMicroelectronics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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
 *******************************************************************************
 */
/*
 * Based on STMicroelectronics SPBTLE driver
 * Modified by Neon Smart Studio for W-Link
 */

#ifndef __SPBTLE_H
#define __SPBTLE_H

#include <stdint.h>

#include "Bluetooth/Bluetooth_Def.h"

#include "Bluetooth_Config.h"

#define IDB04A1 0
#define IDB05A1 1

#if defined(CONFIG_BNRG_EXPANSION_BOARD_IDB04A1)
    #define BNRG_EXPANSION_BOARD    IDB04A1
#elif defined(CONFIG_BNRG_EXPANSION_BOARD_IDB05A1)
    #define BNRG_EXPANSION_BOARD    IDB05A1
#else
    /*
     * Default board
     * B-L475E-IOT01A 使用 SPBTLE-RF / BlueNRG-MS
     */
    #define BNRG_EXPANSION_BOARD    IDB05A1
#endif

#ifndef CONFIG_SPBTLE_SPI_INDEX
#define SPBTLE_SPI_INDEX      hwSPI_Index_2
#else
#define SPBTLE_SPI_INDEX      CONFIG_SPBTLE_SPI_INDEX
#endif

#ifndef CONFIG_SPBTLE_SPI_CLOCK_HZ
#define SPBTLE_SPI_CLOCK_HZ   1000000
#else
#define SPBTLE_SPI_CLOCK_HZ   CONFIG_SPBTLE_SPI_CLOCK_HZ
#endif

#ifndef CONFIG_SPBTLE_SPI_MODE
#define SPBTLE_SPI_MODE       hwSPI_OpMode_Polarity0_Phase0
#else
#define SPBTLE_SPI_MODE       CONFIG_SPBTLE_SPI_MODE
#endif

#ifndef CONFIG_SPBTLE_CS_PIN
#define SPBTLE_CS_PIN         hwGPIO_Pin_D13
#else
#define SPBTLE_CS_PIN         CONFIG_SPBTLE_CS_PIN
#endif

#ifndef CONFIG_SPBTLE_IRQ_PIN
#define SPBTLE_IRQ_PIN        hwGPIO_Int_Pin_E6
#else
#define SPBTLE_IRQ_PIN        CONFIG_SPBTLE_IRQ_PIN
#endif

#ifndef CONFIG_SPBTLE_RESET_PIN
#define SPBTLE_RESET_PIN      hwGPIO_Pin_A8
#else
#define SPBTLE_RESET_PIN      CONFIG_SPBTLE_RESET_PIN
#endif

typedef enum {
  DISABLE_LOW_POWER_MODE = 0,
  ENABLE_LOW_POWER_MODE
} lowPowerMode_t;

#endif //__SPBTLE_H
