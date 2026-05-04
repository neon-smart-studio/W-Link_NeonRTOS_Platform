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
#ifdef DEVICE_STM32
    __WFI(); // Wait For Interrupt
#else
    while (1);
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

int main(void) {
#ifdef DEVICE_STM32
    HAL_Init();
#endif
    SystemClock_Config();

    //__HAL_RCC_WWDG_CLK_DISABLE();  // 禁用窗口看門狗
    //__HAL_RCC_IWDG_CLK_DISABLE();  // 禁用獨立看門狗
    
    // 啟動 NeonRTOS 調度器
    NeonRTOS_start();

    while (1);
}
//