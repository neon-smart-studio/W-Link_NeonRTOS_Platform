#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "soc.h"

#include "FreeRTOS.h"
#include "task.h"

#include "SysCtrl/SysCtrl.h"
#include "UART/UART.h"
#include "I2C/I2C_Master.h"

#include "Sensor/HTS221/HTS221.h"

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"

void HardFault_Handler()
{
    while (1);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    // 堆疊溢出處理
    while (1);
}

void vApplicationIdleHook(void) {
    // MCU 進入低功耗模式，等待中斷
#if defined(DEVICE_STM32)
    __WFI();
#elif defined(RP2040) || defined(RP2350)
    __wfi();
#else
    __asm volatile ("wfi");
#endif
}

void vApplicationTickHook(void) {
#ifdef DEVICE_STM32
    HAL_IncTick(); // 增加 HAL 的滴答計數
#endif
}

void Sensor_Task(void* p)
{
    uint8_t id = 0;
    float temperature = 0.0f;
    float humidity = 0.0f;
    HTS221_OpStatus status;

    I2C_Master_Init(hwI2C_Index_1, hwI2C_Standard_Mode);

    status = HTS221_Init();
    if (status != HTS221_OK) {
        UART_Printf("HTS221_Init failed: %d\r\n", status);
        while (1) {
            NeonRTOS_Sleep(1000);
        }
    }

    status = HTS221_ReadID(&id);
    if (status != HTS221_OK) {
        UART_Printf("HTS221_ReadID failed: %d\r\n", status);
    } else {
        UART_Printf("HTS221 ID: 0x%02X\r\n", id);
    }

    status = HTS221_SetODR(1.0f);
    if (status != HTS221_OK) {
        UART_Printf("HTS221_SetODR failed: %d\r\n", status);
    }

    status = HTS221_Enable();
    if (status != HTS221_OK) {
        UART_Printf("HTS221_Enable failed: %d\r\n", status);
        while (1) {
            NeonRTOS_Sleep(1000);
        }
    }

    UART_Printf("HTS221 demo started\r\n");

    while (1) {
        status = HTS221_GetTemperature(&temperature);
        if (status != HTS221_OK) {
            UART_Printf("Read temperature failed: %d\r\n", status);
        }

        status = HTS221_GetHumidity(&humidity);
        if (status != HTS221_OK) {
            UART_Printf("Read humidity failed: %d\r\n", status);
        }

        UART_Printf("Temperature: %.2f C, Humidity: %.2f %%RH\r\n",
               temperature,
               humidity);

        NeonRTOS_Sleep(1000);
    }
}

int main(void) {
    SysCtrl_Init();

    //__HAL_RCC_WWDG_CLK_DISABLE();  // 禁用窗口看門狗
    //__HAL_RCC_IWDG_CLK_DISABLE();  // 禁用獨立看門狗

    UART_Open(LOG_UART_INDEX, 115200, false);

    NeonRTOS_TaskCreate(
        Sensor_Task,
        (const signed char *)"Sensor",
        512,
        NULL,
        2,
        NULL
    );

    // 啟動 NeonRTOS 調度器
    NeonRTOS_start();

    while (1);
}
//