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

//#include "NFC/Device/M24SR/M24SR.h"
//#include "NFC/Devce/ST25R95//RFal_ST25R95.h"

#include "Bluetooth/Bluetooth.h"

#include "NeonRTOS.h"
#include "NeonTCPIP.h"
#include "NeonAppInterface.h"

#include "GPIO/GPIO.h"

#include "NeonServices/HTTPd/HTTPd.h"

#include "NFC_Demo.h"

/*
#define ENV_SERVICE_UUID      "42821a40-e477-11e2-82d0-0002a5d5c51b"
#define TEMP_CHAR_UUID        "a32e5520-e477-11e2-a9e3-0002a5d5c51b"
#define HUMIDITY_CHAR_UUID    "01c50b60-e48c-11e2-a073-0002a5d5c51b"
*/
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

void vSensor_Task(void* p)
{
    uint16_t sensorID;

    I2C_Master_Init(hwI2C_Index_1, hwI2C_Standard_Mode);

    //VL53L1X_Init();
    VL53L4CD_Init();
    VL53L5CX_Init();

    //VL53L1X_Power_Off();
    VL53L4CD_Power_Off();
    VL53L5CX_Power_Off();
    NeonRTOS_Sleep(10);

    //VL53L1X_Power_On();
    //NeonRTOS_Sleep(10);
    //VL53L1X_SetI2CAddress(0x54);

    VL53L4CD_Power_On();
    NeonRTOS_Sleep(10);
    VL53L4CD_SetI2CAddress(0x56);

    VL53L5CX_Power_On();
    NeonRTOS_Sleep(10);
    //VL53L5CX_Set_I2C_Address(0x58);

    //VL53L1X_GetSensorId(&sensorID);
    //UART_Printf("sensorID = 0x%4x\n", sensorID);

    VL53L4CD_GetSensorId(&sensorID);
    UART_Printf("sensorID = 0x%4x\n", sensorID);
/*
    if(VL53L1X_SensorInit() < VL53L1X_OK)
    {
        UART_Printf("VL53L1X init failed\n");
        while(1);
    }
*/
    if(VL53L4CD_SensorInit() < VL53L4CD_OK)
    {
        UART_Printf("VL53L4CD init failed\n");
        while(1);
    }

    if(VL53L5CX_SensorInit() < VL53L5CX_OK)
    {
        UART_Printf("VL53L5CX init failed\n");
        while(1);
    }

    //VL53L1X_SetDistanceMode(2);
    //VL53L1X_SetTimingBudgetInMs(50);
    //VL53L1X_SetInterMeasurementInMs(100);
    //VL53L1X_StartRanging();

    VL53L4CD_SetRangeTiming(200, 0);
    VL53L4CD_StartRanging();

    VL53L5CX_Start_Ranging();

    while(1)
    {
        uint8_t ready = 0;
        VL53L4CD_Result_t result;
        VL53L5CX_ResultsData resultData;
        uint16_t dist = 0;
/*
        if(VL53L1X_CheckForDataReady(&ready) == VL53L1X_OK && ready)
        {
            VL53L1X_GetDistance(&dist);
            VL53L1X_ClearInterrupt();

            UART_Printf("distance = %d\n", dist);
        }
*/
        if(VL53L4CD_CheckForDataReady(&ready) == VL53L4CD_OK && ready)
        {
            VL53L4CD_ClearInterrupt();
            VL53L4CD_GetResult(&result);

            UART_Printf("distance = %d\n", result.distance_mm);
        }

        if(VL53L5CX_Check_Data_Ready(&ready) == VL53L4CD_OK && ready)
        {
            VL53L5CX_Get_Ranging_Data(&resultData);

            UART_Printf("distance = %d\n", resultData.distance_mm);
        }

        NeonRTOS_Sleep(10);
    }
}

void VL53L0X_Sensor_Task(void* p)
{
    uint16_t sensorID;

    I2C_Master_Init(hwI2C_Index_1, hwI2C_Standard_Mode);

    VL53L0X_GetSensorId(&sensorID);
    UART_Printf("sensorID = 0x%4x\n", sensorID);

    if(VL53L0X_SensorInit() < VL53L0X_OK)
    {
        UART_Printf("VL53L1X init failed\n");
        while(1);
    }

    while(1)
    {
        uint32_t dist = 0;

        VL53L0X_GetDistance(&dist);

        UART_Printf("distance = %d\n", dist);

        NeonRTOS_Sleep(20);
    }
}

void VL53L1X_Sensor_Task(void* p)
{
    uint16_t sensorID;

    I2C_Master_Init(hwI2C_Index_1, hwI2C_Standard_Mode);

    VL53L1X_GetSensorId(&sensorID);
    UART_Printf("sensorID = 0x%4x\n", sensorID);

    if(VL53L1X_SensorInit() < VL53L1X_OK)
    {
        UART_Printf("VL53L1X init failed\n");
        while(1);
    }

    VL53L1X_SetDistanceMode(2);
    VL53L1X_SetTimingBudgetInMs(50);
    VL53L1X_SetInterMeasurementInMs(100);
    VL53L1X_StartRanging();

    while(1)
    {
        uint8_t ready = 0;
        uint16_t dist = 0;

        if(VL53L1X_CheckForDataReady(&ready) == VL53L1X_OK && ready)
        {
            VL53L1X_GetDistance(&dist);
            VL53L1X_ClearInterrupt();

            UART_Printf("distance = %d\n", dist);
        }

        NeonRTOS_Sleep(10);
    }
}

void VL53L4CD_Sensor_Task(void* p)
{
    uint16_t sensorID;
    
    I2C_Master_Init(hwI2C_Index_1, hwI2C_Fast_Mode);

    VL53L4CD_Init();

    VL53L4CD_Power_Off();

    NeonRTOS_Sleep(100);

    VL53L4CD_Power_On();

    VL53L4CD_GetSensorId(&sensorID);
    UART_Printf("sensorID = 0x%4x\n", sensorID);

    if(VL53L4CD_SensorInit() < VL53L4CD_OK)
    {
        UART_Printf("VL53L1X init failed\n");
        while(1);
    }

    VL53L4CD_SetRangeTiming(200, 0);
    VL53L4CD_StartRanging();

    while(1)
    {
        uint8_t ready = 0;
        VL53L4CD_Result_t result;

        if(VL53L4CD_CheckForDataReady(&ready) == VL53L4CD_OK && ready)
        {
            VL53L4CD_ClearInterrupt();
            VL53L4CD_GetResult(&result);

            UART_Printf("distance = %d\n", result.distance_mm);
        }

        NeonRTOS_Sleep(10);
    }
}

int main(void) {
    SysCtrl_Init();

    DMA_Init();
    
    //__HAL_RCC_WWDG_CLK_DISABLE();  // 禁用窗口看門狗
    //__HAL_RCC_IWDG_CLK_DISABLE();  // 禁用獨立看門狗

    NeonRTOS_TaskCreate(
        vSensor_Task,
        (const signed char *)"Sensor",
        2048,
        NULL,
        2,
        NULL
    );
/*
    NeonRTOS_TaskCreate(
        VL53L0X_Sensor_Task,
        (const signed char *)"Sensor",
        1024,
        NULL,
        2,
        NULL
    );
    NeonRTOS_TaskCreate(
        VL53L1X_Sensor_Task,
        (const signed char *)"Sensor",
        1024,
        NULL,
        2,
        NULL
    );
    NeonRTOS_TaskCreate(
        VL53L4CD_Sensor_Task,
        (const signed char *)"Sensor",
        1024,
        NULL,
        2,
        NULL
    );
*/
    //NFC_Demo_Init();

    Neon_App_Init();

    // 啟動 NeonRTOS 調度器
    NeonRTOS_start();

    while (1);
}
//