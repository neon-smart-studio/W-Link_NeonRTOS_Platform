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

NeonRTOS_SyncObj_t VL53L5CX_Sync = NULL;
NeonRTOS_SyncObj_t VL53L8CX_Sync = NULL;

static void VL53L1X_Event_Handler(uint8_t sensor_index)
{
    UART_Printf("VL53L1X_Event_Handler(%d)\n", sensor_index);
    
    uint16_t dist = 0;
    
    VL53L1X_GetDistance(0, &dist);
    VL53L1X_ClearInterrupt(0);

    UART_Printf("VL53L1X[%d] distance = %d\n", sensor_index, dist);
}

static void VL53L4CD_Event_Handler(uint8_t sensor_index)
{
    UART_Printf("VL53L4CD_Event_Handler(%d)\n", sensor_index);
    
    VL53L4CD_Result_t result;

    VL53L4CD_ClearInterrupt(0);
    VL53L4CD_GetResult(0, &result);

    UART_Printf("VL53L4CD[%d] distance = %d\n", sensor_index, result.distance_mm);
}

static void VL53L5CX_Event_Handler()
{
    UART_Printf("VL53L5CX_Event_Handler()\n");
    
    NeonRTOS_SyncObjSignal(&VL53L5CX_Sync);
}

static void VL53L8CX_Event_Handler()
{
    UART_Printf("VL53L8CX_Event_Handler()\n");
    
    NeonRTOS_SyncObjSignal(&VL53L8CX_Sync);
}

void vSensor_Task(void* p)
{
    uint16_t sensorID;

    uint8_t VL53L1X_I2C_Addr_List[] = {0x54};
    uint8_t VL53L4CD_I2C_Addr_List[] = {0x56};

    hwGPIO_Pin VL53L1X_Power_Pin_List[] = {hwGPIO_Pin_E14};
    hwGPIO_Pin VL53L4CD_Power_Pin_List[] = {hwGPIO_Pin_E15};
    hwGPIO_Pin VL53L5CX_Power_Pin = hwGPIO_Pin_E7;
    hwGPIO_Pin VL53L8CX_Power_Pin = hwGPIO_Pin_F15;

    hwGPIO_Int_Pin VL53L1X_Interrupt_Pin_List[] = {hwGPIO_Int_Pin_E10};
    hwGPIO_Int_Pin VL53L4CD_Interrupt_Pin_List[] = {hwGPIO_Int_Pin_E12};
    hwGPIO_Int_Pin VL53L5CX_Interrupt_Pin = hwGPIO_Int_Pin_E8;
    hwGPIO_Int_Pin VL53L8CX_Interrupt_Pin = hwGPIO_Int_Pin_F13;

    I2C_Master_Init(hwI2C_Index_1, hwI2C_Standard_Mode);

    VL53L1X_Init(1, VL53L1X_Power_Pin_List, VL53L1X_Interrupt_Pin_List, VL53L1X_Event_Handler);
    VL53L4CD_Init(1, VL53L4CD_Power_Pin_List, VL53L4CD_Interrupt_Pin_List, VL53L4CD_Event_Handler);
    VL53L5CX_Init(VL53L5CX_Power_Pin, VL53L5CX_Interrupt_Pin, VL53L5CX_Event_Handler);
    VL53L8CX_Init(VL53L8CX_Power_Pin, VL53L8CX_Interrupt_Pin, VL53L8CX_Event_Handler);

    VL53L1X_Power_Off(0);
    VL53L4CD_Power_Off(0);
    VL53L5CX_Power_Off();
    VL53L8CX_Power_Off();
/*
    VL53L1X_Power_On(0);
    VL53L1X_Set_I2C_Address(0, VL53L1X_I2C_Addr_List[0]);
    
    VL53L4CD_Power_On(0);
    VL53L4CD_Set_I2C_Address(0, VL53L4CD_I2C_Addr_List[0]);
   
    VL53L5CX_Power_On();
    VL53L5CX_Set_I2C_Address(0x58);
 */
    VL53L8CX_Power_On();
    VL53L8CX_Set_I2C_Address(0x60);
/*
    VL53L1X_GetSensorId(0, &sensorID);
    UART_Printf("VL53L1X sensorID = 0x%4x\n", sensorID);

    VL53L4CD_GetSensorId(0, &sensorID);
    UART_Printf("VL53L4CD sensorID = 0x%4x\n", sensorID);

    if(VL53L1X_SensorInit(0) < VL53L1X_OK)
    {
        UART_Printf("VL53L1X init failed\n");
        while(1);
    }

    if(VL53L4CD_SensorInit(0) < VL53L4CD_OK)
    {
        UART_Printf("VL53L4CD init failed\n");
        while(1);
    }

    if(VL53L5CX_SensorInit() < VL53L5CX_OK)
    {
        UART_Printf("VL53L5CX init failed\n");
        while(1);
    }
*/
    if(VL53L8CX_SensorInit() < VL53L8CX_OK)
    {
        UART_Printf("VL53L8CX init failed\n");
        while(1);
    }
/*
    VL53L1X_SetDistanceMode(0, 2);
    VL53L1X_SetTimingBudgetInMs(0, 50);
    VL53L1X_SetInterMeasurementInMs(0, 100);
    VL53L1X_StartRanging(0);

    VL53L4CD_SetRangeTiming(0, 200, 0);
    VL53L4CD_StartRanging(0);

    VL53L5CX_Set_Resolution(VL53L5CX_RESOLUTION_8X8);
    VL53L5CX_Start_Ranging();
*/
    VL53L8CX_Set_Resolution(VL53L8CX_RESOLUTION_8X8);
    VL53L8CX_Start_Ranging();

    while(1)
    {
        /*
        if(NeonRTOS_SyncObjWait(&VL53L5CX_Sync, 100)==NeonRTOS_OK)
        {
            VL53L5CX_ResultsData resultData;

            VL53L5CX_Get_Ranging_Data(&resultData);

            UART_Printf("VL53L5CX matrix :\n");
            for(int i = 0; i < 64; i++)
            {
                UART_Printf("%d ", resultData.distance_mm[i]);
                if(i%8==7)
                {
                    UART_Printf("\r\n");
                }
            }
            UART_Printf("\r\n");
        }
*/
        if(NeonRTOS_SyncObjWait(&VL53L8CX_Sync, 100)==NeonRTOS_OK)
        {
            VL53L8CX_ResultsData resultData;

            VL53L8CX_Get_Ranging_Data(&resultData);

            UART_Printf("VL53L8CX matrix :\n");
            for(int i = 0; i < 64; i++)
            {
                UART_Printf("%d ", resultData.distance_mm[i]);
                if(i%8==7)
                {
                    UART_Printf("\r\n");
                }
            }
            UART_Printf("\r\n");
        }
        NeonRTOS_Sleep(100);
    }
}

int main(void) {
    SysCtrl_Init();

    DMA_Init();
    
    //__HAL_RCC_WWDG_CLK_DISABLE();  // 禁用窗口看門狗
    //__HAL_RCC_IWDG_CLK_DISABLE();  // 禁用獨立看門狗
    
    NeonRTOS_SyncObjCreate(&VL53L5CX_Sync);
    NeonRTOS_SyncObjCreate(&VL53L8CX_Sync);

    NeonRTOS_TaskCreate(
        vSensor_Task,
        (const signed char *)"Sensor",
        4096,
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