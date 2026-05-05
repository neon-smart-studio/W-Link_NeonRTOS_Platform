#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "soc.h"

#include "FreeRTOS.h"
#include "task.h"

#include "SysCtrl/SysCtrl.h"

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

void LED_Task(void* p)
{
    GPIO_Pin_Init(hwGPIO_Pin_25, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_None);

    while (1) {
        GPIO_Pin_Write(hwGPIO_Pin_25, true);
        NeonRTOS_Sleep(500);
        GPIO_Pin_Write(hwGPIO_Pin_25, false);
        NeonRTOS_Sleep(500);
    }
}

int main(void) {
    SysCtrl_Init();

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