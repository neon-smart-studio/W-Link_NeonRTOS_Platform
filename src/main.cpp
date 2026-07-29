#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "soc.h"

#include "FreeRTOS.h"
#include "task.h"

#include "SysCtrl/SysCtrl.h"
#include "UART/UART.h"
#include "I2C/I2C_Master.h"
#include "DMA/DMA.h"

#include "Sensor/HTS221/HTS221.h"

#include "Sensor/VL53L0X/VL53L0X.h"
#include "Sensor/VL53L1X/VL53L1X.h"
#include "Sensor/VL53L4CD/VL53L4CD.h"
#include "Sensor/VL53L5CX/VL53L5CX.h"
#include "Sensor/VL53L8CX/VL53L8CX.h"

//#include "NFC/Device/M24SR/M24SR.h"
//#include "NFC/Devce/ST25R95//RFal_ST25R95.h"

#include "Bluetooth/Bluetooth.h"

#include "NeonRTOS.h"
#include "NeonTCPIP.h"
#include "NeonAppInterface.h"

#include "GPIO/GPIO.h"

#include "NeonServices/HTTPd/HTTPd.h"

//#include "NFC_Demo.h"

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

int main(void) {
    SysCtrl_Init();

    DMA_Init();
    
    Neon_App_Init();

    // 啟動 NeonRTOS 調度器
    NeonRTOS_start();

    while (1);
}
//