
#ifndef SOC_H
#define SOC_H

#if defined (NUC442) || defined (NUC472)
#include "NUC472_442.h"
#include "sys.h"
#include "clk.h"
#include "gpio.h"
#include "uart.h"
#include "spi.h"
#include "i2c.h"
#include "rtc.h"
#include "adc.h"
#include "pwm.h"
#include "rtc.h"
#define DEVICE_NUVOTON
#endif //NUC442 || NUC472

#if defined (RP2040) || defined (RP2350)
#define DEVICE_RP2
#endif

#if defined (STM32C011xx) || defined (STM32C031xx) || \
    defined (STM32C051xx) || defined (STM32C071xx) || \
    defined (STM32C091xx) || defined (STM32C092xx)
#include "stm32c0xx.h"
#include "stm32c0xx_hal_gpio.h"
#include "stm32c0xx_hal_exti.h"
#include "stm32c0xx_hal_uart.h"
#include "stm32c0xx_hal_spi.h"
#include "stm32c0xx_hal_i2c.h"
#include "stm32c0xx_hal_dma.h"
#include "stm32c0xx_hal_rtc.h"
#include "stm32c0xx_hal_adc.h"
#endif //STM32C0

#if defined (STM32F030x6) || defined (STM32F030x8) ||                           \
    defined (STM32F031x6) || defined (STM32F038xx) ||                           \
    defined (STM32F042x6) || defined (STM32F048xx) || defined (STM32F070x6) || \
    defined (STM32F051x8) || defined (STM32F058xx) ||                           \
    defined (STM32F071xB) || defined (STM32F072xB) || defined (STM32F078xx) || defined (STM32F070xB) || \
    defined (STM32F091xC) || defined (STM32F098xx) || defined (STM32F030xC)
#include "stm32f0xx.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f0xx_hal_exti.h"
#include "stm32f0xx_hal_uart.h"
#include "stm32f0xx_hal_spi.h"
#include "stm32f0xx_hal_i2c.h"
#include "stm32f0xx_hal_dma.h"
#include "stm32f0xx_hal_rtc.h"
#include "stm32f0xx_hal_adc.h"
#include "stm32f0xx_hal_can.h"
#include "stm32f0xx_hal_dac.h"
#endif //STM32F0

#if defined (STM32F100xB) || defined (STM32F100xE) || defined (STM32F101x6) || \
    defined (STM32F101xB) || defined (STM32F101xE) || defined (STM32F101xG) || defined (STM32F102x6) || defined (STM32F102xB) || defined (STM32F103x6) || \
    defined (STM32F103xB) || defined (STM32F103xE) || defined (STM32F103xG) || defined (STM32F105xC) || defined (STM32F107xC)
#include "stm32f1xx.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_exti.h"
#include "stm32f1xx_hal_uart.h"
#include "stm32f1xx_hal_spi.h"
#include "stm32f1xx_hal_i2c.h"
#include "stm32f1xx_hal_dma.h"
#include "stm32f1xx_hal_rtc.h"
#include "stm32f1xx_hal_adc.h"
#include "stm32f1xx_hal_can.h"
#include "stm32f1xx_hal_dac.h"
#endif //STM32F1

#if defined (STM32F205xx) || defined (STM32F215xx) || defined (STM32F207xx) || defined (STM32F217xx)
#include "stm32f2xx.h"
#include "stm32f2xx_hal_gpio.h"
#include "stm32f2xx_hal_exti.h"
#include "stm32f2xx_hal_uart.h"
#include "stm32f2xx_hal_spi.h"
#include "stm32f2xx_hal_i2c.h"
#include "stm32f2xx_hal_dma.h"
#include "stm32f2xx_hal_rtc.h"
#include "stm32f2xx_hal_adc.h"
#include "stm32f2xx_hal_can.h"
#include "stm32f2xx_hal_dac.h"
#endif //STM32F2

#if defined (STM32F301x8) || defined (STM32F302x8) || defined (STM32F318xx) || \
    defined (STM32F302xC) || defined (STM32F303xC) || defined (STM32F358xx) || \
    defined (STM32F303x8) || defined (STM32F334x8) || defined (STM32F328xx) || \
    defined (STM32F302xE) || defined (STM32F303xE) || defined (STM32F398xx) || \
    defined (STM32F373xC) || defined (STM32F378xx)
#include "stm32f3xx.h"
#include "stm32f3xx_hal_gpio.h"
#include "stm32f3xx_hal_exti.h"
#include "stm32f3xx_hal_uart.h"
#include "stm32f3xx_hal_spi.h"
#include "stm32f3xx_hal_i2c.h"
#include "stm32f3xx_hal_dma.h"
#include "stm32f3xx_hal_rtc.h"
#include "stm32f3xx_hal_adc.h"
#include "stm32f3xx_hal_can.h"
#include "stm32f3xx_hal_dac.h"
#endif //STM32F3

#if defined (STM32F405xx) || defined (STM32F415xx) || defined (STM32F407xx) || defined (STM32F417xx) || \
    defined (STM32F427xx) || defined (STM32F437xx) || defined (STM32F429xx) || defined (STM32F439xx) || \
    defined (STM32F401xC) || defined (STM32F401xE) || defined (STM32F410Tx) || defined (STM32F410Cx) || \
    defined (STM32F410Rx) || defined (STM32F411xE) || defined (STM32F446xx) || defined (STM32F469xx) || \
    defined (STM32F479xx) || defined (STM32F412Cx) || defined (STM32F412Rx) || defined (STM32F412Vx) || \
    defined (STM32F412Zx) || defined (STM32F413xx) || defined (STM32F423xx)
#include "stm32f4xx.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_exti.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_i2c.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_rtc.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_can.h"
#include "stm32f4xx_hal_dac.h"
#endif //STM32F4

#if defined (STM32F756xx) || defined (STM32F746xx) || defined (STM32F745xx) || defined (STM32F765xx) || \
    defined (STM32F767xx) || defined (STM32F769xx) || defined (STM32F777xx) || defined (STM32F779xx) || \
    defined (STM32F722xx) || defined (STM32F723xx) || defined (STM32F732xx) || defined (STM32F733xx) || \
    defined (STM32F730xx) || defined (STM32F750xx)
#include "stm32f7xx.h"
#include "stm32f7xx_hal_gpio.h"
#include "stm32f7xx_hal_exti.h"
#include "stm32f7xx_hal_uart.h"
#include "stm32f7xx_hal_spi.h"
#include "stm32f7xx_hal_i2c.h"
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_rtc.h"
#include "stm32f7xx_hal_adc.h"
#include "stm32f7xx_hal_can.h"
#include "stm32f7xx_hal_dac.h"
#endif //STM32F7

#if defined (STM32G071xx) || defined (STM32G081xx) || defined (STM32G070xx) || \
    defined (STM32G030xx) || defined (STM32G031xx) || defined (STM32G041xx) || \
    defined (STM32G0B0xx) || defined (STM32G0B1xx) || defined (STM32G0C1xx) || \
    defined (STM32G050xx) || defined (STM32G051xx) || defined (STM32G061xx)
#include "stm32g0xx.h"
#include "stm32g0xx_hal_gpio.h"
#include "stm32g0xx_hal_exti.h"
#include "stm32g0xx_hal_uart.h"
#include "stm32g0xx_hal_spi.h"
#include "stm32g0xx_hal_i2c.h"
#include "stm32g0xx_hal_dma.h"
#include "stm32g0xx_hal_rtc.h"
#include "stm32g0xx_hal_adc.h"
#include "stm32g0xx_hal_dac.h"
#endif //STM32G0

#if defined (STM32G431xx) || defined (STM32G441xx) || defined (STM32G471xx) || \
    defined (STM32G473xx) || defined (STM32G474xx) || defined (STM32G484xx) || \
    defined (STM32GBK1CB) || defined (STM32G491xx) || defined (STM32G4A1xx)
#include "stm32g4xx.h"
#include "stm32g4xx_hal_gpio.h"
#include "stm32g4xx_hal_exti.h"
#include "stm32g4xx_hal_uart.h"
#include "stm32g4xx_hal_spi.h"
#include "stm32g4xx_hal_i2c.h"
#include "stm32g4xx_hal_dma.h"
#include "stm32g4xx_hal_rtc.h"
#include "stm32g4xx_hal_adc.h"
#include "stm32g4xx_hal_dac.h"
#endif

#if defined (STM32H573xx) || defined (STM32H563xx) || \
    defined (STM32H562xx) || defined (STM32H503xx) || \
    defined (STM32H533xx) || defined (STM32H523xx)
#include "stm32h5xx.h"
#include "stm32h5xx_hal_gpio.h"
#include "stm32h5xx_hal_exti.h"
#include "stm32h5xx_hal_uart.h"
#include "stm32h5xx_hal_spi.h"
#include "stm32h5xx_hal_i2c.h"
#include "stm32h5xx_hal_dma.h"
#include "stm32h5xx_hal_rtc.h"
#include "stm32h5xx_hal_adc.h"
#include "stm32h5xx_hal_dac.h"
#endif

#if defined (STM32L010x4) || defined (STM32L010x6) || defined (STM32L010x8) || defined (STM32L010xB) || \
    defined (STM32L011xx) || defined (STM32L021xx) || \
    defined (STM32L031xx) || defined (STM32L041xx) || \
    defined (STM32L051xx) || defined (STM32L052xx) || defined (STM32L053xx) || \
    defined (STM32L062xx) || defined (STM32L063xx) || \
    defined (STM32L071xx) || defined (STM32L072xx) || defined (STM32L073xx) || \
    defined (STM32L081xx) || defined (STM32L082xx) || defined (STM32L083xx)
#include "stm32l0xx.h"
#include "stm32l0xx_hal_gpio.h"
#include "stm32l0xx_hal_exti.h"
#include "stm32l0xx_hal_uart.h"
#include "stm32l0xx_hal_spi.h"
#include "stm32l0xx_hal_i2c.h"
#include "stm32l0xx_hal_dma.h"
#include "stm32l0xx_hal_rtc.h"
#include "stm32l0xx_hal_adc.h"
#include "stm32l0xx_hal_dac.h"
#endif

#if defined (STM32L010x4) || defined (STM32L010x6) || defined (STM32L010x8) || defined (STM32L010xB) || \
    defined (STM32L011xx) || defined (STM32L021xx) || \
    defined (STM32L031xx) || defined (STM32L041xx) || \
    defined (STM32L051xx) || defined (STM32L052xx) || defined (STM32L053xx) || \
    defined (STM32L062xx) || defined (STM32L063xx) || \
    defined (STM32L071xx) || defined (STM32L072xx) || defined (STM32L073xx) || \
    defined (STM32L081xx) || defined (STM32L082xx) || defined (STM32L083xx)
#include "stm32l0xx.h"
#include "stm32l0xx_hal_gpio.h"
#include "stm32l0xx_hal_exti.h"
#include "stm32l0xx_hal_uart.h"
#include "stm32l0xx_hal_spi.h"
#include "stm32l0xx_hal_i2c.h"
#include "stm32l0xx_hal_dma.h"
#include "stm32l0xx_hal_rtc.h"
#include "stm32l0xx_hal_adc.h"
#include "stm32l0xx_hal_dac.h"
#endif

#if defined (STM32L100xB) || defined (STM32L100xBA) || defined (STM32L100xC) || \
    defined (STM32L151xB) || defined (STM32L151xBA) || defined (STM32L151xC) || defined (STM32L151xCA) || defined (STM32L151xD) || defined (STM32L151xDX) || defined (STM32L151xE) || \
    defined (STM32L152xB) || defined (STM32L152xBA) || defined (STM32L152xC) || defined (STM32L152xCA) || defined (STM32L152xD) || defined (STM32L152xDX) || defined (STM32L152xE) || \
    defined (STM32L162xC) || defined (STM32L162xCA) || defined (STM32L162xD) || defined (STM32L162xDX) || defined (STM32L162xE)
#include "stm32l1xx.h"
#include "stm32l1xx_hal_gpio.h"
#include "stm32l1xx_hal_exti.h"
#include "stm32l1xx_hal_uart.h"
#include "stm32l1xx_hal_spi.h"
#include "stm32l1xx_hal_i2c.h"
#include "stm32l1xx_hal_dma.h"
#include "stm32l1xx_hal_rtc.h"
#include "stm32l1xx_hal_adc.h"
#include "stm32l1xx_hal_dac.h"
#endif

#if defined (STM32L412xx) || defined (STM32L422xx) || \
    defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx) || \
    defined (STM32L451xx) || defined (STM32L452xx) || defined (STM32L462xx) || \
    defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4P5xx) || defined (STM32L4Q5xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#include "stm32l4xx.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_exti.h"
#include "stm32l4xx_hal_uart.h"
#include "stm32l4xx_hal_spi.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_dma.h"
#include "stm32l4xx_hal_rtc.h"
#include "stm32l4xx_hal_adc.h"
#include "stm32l4xx_hal_can.h"
#include "stm32l4xx_hal_dac.h"
#endif

#if defined (STM32L552xx) || defined (STM32L562xx)
#include "stm32l5xx.h"
#include "stm32l5xx_hal_gpio.h"
#include "stm32l5xx_hal_exti.h"
#include "stm32l5xx_hal_uart.h"
#include "stm32l5xx_hal_spi.h"
#include "stm32l5xx_hal_i2c.h"
#include "stm32l5xx_hal_dma.h"
#include "stm32l5xx_hal_rtc.h"
#include "stm32l5xx_hal_adc.h"
#include "stm32l5xx_hal_dac.h"
#endif

#if defined (STM32U073xx) || defined (STM32U083xx) || defined (STM32U031xx)
#include "stm32u0xx.h"
#include "stm32u0xx_hal_gpio.h"
#include "stm32u0xx_hal_exti.h"
#include "stm32u0xx_hal_uart.h"
#include "stm32u0xx_hal_spi.h"
#include "stm32u0xx_hal_i2c.h"
#include "stm32u0xx_hal_dma.h"
#include "stm32u0xx_hal_rtc.h"
#include "stm32u0xx_hal_adc.h"
#include "stm32u0xx_hal_dac.h"
#endif

#if defined (STM32U575xx) || defined (STM32U585xx) || \
    defined (STM32U595xx) || defined (STM32U599xx) || \
    defined (STM32U5A5xx) || defined (STM32U5A9xx) || \
    defined (STM32U5F7xx) || defined (STM32U5G7xx) || \
    defined (STM32U5F9xx) || defined (STM32U5G9xx) || \
    defined (STM32U535xx) || defined (STM32U545xx)
#include "stm32u5xx.h"
#include "stm32u5xx_hal_gpio.h"
#include "stm32u5xx_hal_exti.h"
#include "stm32u5xx_hal_uart.h"
#include "stm32u5xx_hal_spi.h"
#include "stm32u5xx_hal_i2c.h"
#include "stm32u5xx_hal_dma.h"
#include "stm32u5xx_hal_rtc.h"
#include "stm32u5xx_hal_adc.h"
#include "stm32u5xx_hal_dac.h"
#endif

#if defined (STM32H743xx) || defined (STM32H753xx)  || defined (STM32H750xx) || defined (STM32H742xx)  || \
    defined (STM32H745xx) || defined (STM32H745xG)  || defined (STM32H755xx) || defined (STM32H747xx)  || defined (STM32H747xG) || defined (STM32H757xx) || \
    defined (STM32H7A3xx) || defined (STM32H7A3xxQ) || defined (STM32H7B3xx) || defined (STM32H7B3xxQ) || defined (STM32H7B0xx) || defined (STM32H7B0xxQ) || \
    defined (STM32H735xx) || defined (STM32H733xx)  || defined (STM32H730xx) || defined (STM32H730xxQ) || defined (STM32H725xx) || defined (STM32H723xx)
#include "stm32h7xx.h"
#include "stm32h7xx_hal_gpio.h"
#include "stm32h7xx_hal_exti.h"
#include "stm32h7xx_hal_uart.h"
#include "stm32h7xx_hal_spi.h"
#include "stm32h7xx_hal_i2c.h"
#include "stm32h7xx_hal_dma.h"
#include "stm32h7xx_hal_rtc.h"
#include "stm32h7xx_hal_adc.h"
#include "stm32h7xx_hal_dac.h"
#endif //STM32H7

#if defined (STM32H7R7xx) || defined (STM32H7R3xx) || defined (STM32H7S3xx) || defined (STM32H7S7xx)
#include "stm32h7rsxx.h"
#include "stm32h7rsxx_hal_gpio.h"
#include "stm32h7rsxx_hal_exti.h"
#include "stm32h7rsxx_hal_uart.h"
#include "stm32h7rsxx_hal_spi.h"
#include "stm32h7rsxx_hal_i2c.h"
#include "stm32h7rsxx_hal_dma.h"
#include "stm32h7rsxx_hal_rtc.h"
#include "stm32h7rsxx_hal_adc.h"
#endif //STM32H7RS

#if defined (STM32WB55xx) || defined (STM32WB5Mxx) || defined (STM32WB50xx) || defined (STM32WB35xx) || \
    defined(STM32WB30xx) || defined(STM32WB15xx) || defined(STM32WB10xx) || defined(STM32WB1Mxx)
#include "stm32wbxx.h"
#include "stm32wbxx_hal_gpio.h"
#include "stm32wbxx_hal_exti.h"
#include "stm32wbxx_hal_uart.h"
#include "stm32wbxx_hal_spi.h"
#include "stm32wbxx_hal_i2c.h"
#include "stm32wbxx_hal_dma.h"
#include "stm32wbxx_hal_rtc.h"
#include "stm32wbxx_hal_adc.h"
#endif //STM32WB

#if defined(STM32WBA50xx) || defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx)
#include "stm32wbaxx.h"
#include "stm32wbaxx_hal_gpio.h"
#include "stm32wbaxx_hal_exti.h"
#include "stm32wbaxx_hal_uart.h"
#include "stm32wbaxx_hal_spi.h"
#include "stm32wbaxx_hal_i2c.h"
#include "stm32wbaxx_hal_dma.h"
#include "stm32wbaxx_hal_rtc.h"
#include "stm32wbaxx_hal_adc.h"
#endif //STM32WBA

#if defined (STM32WL55xx) || defined (STM32WL54xx) || defined (STM32WLE5xx) || defined (STM32WLE4xx) || defined(STM32WL5Mxx)
#include "stm32wlxx.h"
#include "stm32wlxx_hal_gpio.h"
#include "stm32wlxx_hal_exti.h"
#include "stm32wlxx_hal_uart.h"
#include "stm32wlxx_hal_spi.h"
#include "stm32wlxx_hal_i2c.h"
#include "stm32wlxx_hal_dma.h"
#include "stm32wlxx_hal_rtc.h"
#include "stm32wlxx_hal_adc.h"
#include "stm32wlxx_hal_dac.h"
#endif //STM32WL

#if defined (RP2040) || defined (RP2350)
#include "pico/types.h"
#include "pico/time.h"
#include "hardware/sync.h"
#include "hardware/irq.h"
#include "hardware/resets.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/rtc.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "hardware/structs/adc.h"
#include "hardware/structs/resets.h"
#include "hardware/structs/clocks.h"
#endif

#if defined (STM32F0) || defined (STM32F1) || defined (STM32F2) || defined (STM32F3) || defined (STM32F4) || defined (STM32F7) || \
    defined (STM32L0) || defined (STM32L1) || defined (STM32L4) || defined (STM32L5) || \
    defined (STM32C0) || \
    defined (STM32G0) || defined (STM32G4) || \
    defined (STM32H5) || defined (STM32H7) || defined (STM32H7RS) || \
    defined (STM32U0) || defined (STM32U5) || \
    defined (STM32WBA) || defined (STM32WB) || defined (STM32WL)
#define DEVICE_STM32
#endif

#ifdef DEVICE_TM4C1294
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_emac.h"
#include "inc/hw_types.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/debug.h"
#include "driverlib/emac.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/i2c.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "driverlib/flash.h"
#endif //DEVICE_TM4C1294

#endif //SOC_H