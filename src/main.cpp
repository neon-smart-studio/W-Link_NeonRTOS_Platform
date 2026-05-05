#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "soc.h"

#include "FreeRTOS.h"
#include "task.h"

#include "SysCtrl/SysCtrl.h"

#include "NeonRTOS.h"

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

void SystemClock_Config(void) {
    SysCtrl_Init();
}

void LED_Task(void* p)
{
    while (1) {
        NeonRTOS_Sleep(500);
        NeonRTOS_Sleep(500);
    }
}

int main(void) {
#if defined(RP2040) || defined(RP2350)
    stdio_init_all();
#endif

#ifdef DEVICE_STM32
    HAL_Init();
#endif
    SystemClock_Config();

    //__HAL_RCC_WWDG_CLK_DISABLE();  // 禁用窗口看門狗
    //__HAL_RCC_IWDG_CLK_DISABLE();  // 禁用獨立看門狗

    NeonRTOS_TaskCreate(
        LED_Task,
        (const signed char *)"LED",
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