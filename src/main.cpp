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

//#include "NFC/Device/M24SR/M24SR.h"
//#include "NFC/Devce/ST25R95//RFal_ST25R95.h"

#include "Bluetooth/Bluetooth.h"

#include "NeonRTOS.h"
#include "NeonTCPIP.h"

#include "GPIO/GPIO.h"

#include "NeonService/HTTPd/HTTPd.h"

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

static uint16_t NFC_BuildUriNdef(uint8_t *out, uint16_t out_size, float temp, float hum)
{
    char uri[64];
    int uri_len;
    uint16_t idx = 0;

    uri_len = snprintf(
        uri,
        sizeof(uri),
        "neon-smart?t=%.2f&h=%.2f",
        temp,
        hum
    );

    if (uri_len <= 0 || uri_len > 255)
    {
        return 0;
    }

    /* 2 bytes NLEN + NDEF record */
    if ((uint16_t)(2 + 5 + uri_len) > out_size)
    {
        return 0;
    }

    out[idx++] = 0x00;
    out[idx++] = (uint8_t)(5 + uri_len);

    out[idx++] = 0xD1;
    out[idx++] = 0x01;
    out[idx++] = (uint8_t)(1 + uri_len);
    out[idx++] = 0x55;
    out[idx++] = 0x04;   /* https:// */

    memcpy(&out[idx], uri, uri_len);
    idx += uri_len;

    return idx;
}

/*
static void NFC_Task(void *arg)
{
    M24SR_OpStatus nfc_status;
    HTS221_OpStatus sensor_status;

    uint8_t ndef_buf[128];
    uint16_t ndef_len;

    float temperature = 0.0f;
    float humidity = 0.0f;

    (void)arg;

    I2C_Master_Init(hwI2C_Index_1, hwI2C_Standard_Mode);

    sensor_status = HTS221_Init();
    UART_Printf("HTS221_Init=%d\r\n", sensor_status);

    if (sensor_status == HTS221_OK)
    {
        HTS221_SetODR(1.0f);
        HTS221_Enable();
    }

    nfc_status = M24SR_Init();
    UART_Printf("M24SR_Init=%d\r\n", nfc_status);

    if (nfc_status < M24SR_OK)
    {
        while (1) NeonRTOS_Sleep(1000);
    }

    while (1)
    {
        sensor_status = HTS221_GetTemperature(&temperature);
        UART_Printf("GetTemperature=%d %.2f\r\n", sensor_status, temperature);

        sensor_status = HTS221_GetHumidity(&humidity);
        UART_Printf("GetHumidity=%d %.2f\r\n", sensor_status, humidity);

        memset(ndef_buf, 0, sizeof(ndef_buf));
        ndef_len = NFC_BuildUriNdef(
            ndef_buf,
            sizeof(ndef_buf),
            temperature,
            humidity
        );

        if (ndef_len == 0)
        {
            UART_Printf("Build NDEF failed\r\n");
            NeonRTOS_Sleep(1000);
            continue;
        }

        nfc_status = M24SR_RFConfig(false);
        UART_Printf("RF disable=%d\r\n", nfc_status);

        nfc_status = M24SR_GetSession();
        UART_Printf("GetSession=%d\r\n", nfc_status);

        if (nfc_status == M24SR_OK)
        {
            nfc_status = M24SR_SelectApplication();
            UART_Printf("SelectApplication=%d\r\n", nfc_status);
        }

        if (nfc_status == M24SR_OK)
        {
            nfc_status = M24SR_SelectNDEFfile(NDEF_FILE_ID);
            UART_Printf("SelectNDEF=%d\r\n", nfc_status);
        }

        if (nfc_status == M24SR_OK)
        {
            nfc_status = M24SR_UpdateBinary(0x0000, ndef_len, ndef_buf);
            UART_Printf("WriteNDEF=%d len=%u\r\n", nfc_status, ndef_len);
        }

        nfc_status = M24SR_Deselect();
        UART_Printf("Deselect=%d\r\n", nfc_status);

        nfc_status = M24SR_RFConfig(true);
        UART_Printf("RF enable=%d\r\n", nfc_status);

        UART_Printf("NDEF updated: T=%.2f C, H=%.2f %%RH\r\n",
                    temperature,
                    humidity);

        NeonRTOS_Sleep(5000);
    }
}
*/
/*
void BLE_Task(void* p)
{
    Bluetooth_OpResult bt_ret;
    HTS221_OpStatus sensor_status;

    const char *name = "NeonSmart-IotHub";
    uint8_t SERVER_BDADDR[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x03};

    uint16_t env_service_handle = 0;
    uint16_t temp_char_handle = 0;
    uint16_t humidity_char_handle = 0;

    float temperature = 0.0f;
    float humidity = 0.0f;

    int16_t ble_temp = 0;
    uint16_t ble_humidity = 0;

    I2C_Master_Init(hwI2C_Index_1, hwI2C_Standard_Mode);

    bt_ret = Bluetooth_Init(name, SERVER_BDADDR);
    UART_Printf("Bluetooth_Init=%d\r\n", bt_ret);

    NeonRTOS_Sleep(1000);

    bt_ret = Bluetooth_Add_Service(
        ENV_SERVICE_UUID,
        Bluetooth_Service_Type_Primary,
        10,
        &env_service_handle
    );
    UART_Printf("Bluetooth_Add_Service=%d handle=0x%04X\r\n",
                bt_ret,
                env_service_handle);

    bt_ret = Bluetooth_Service_Add_Char(
        env_service_handle,
        TEMP_CHAR_UUID,
        2,
        (Bluetooth_Char_Property)(Bluetooth_Char_Property_Read | Bluetooth_Char_Property_Notify),
        Bluetooth_Char_Permission_None,
        &temp_char_handle
    );
    UART_Printf("Add Temp Char=%d handle=0x%04X\r\n",
                bt_ret,
                temp_char_handle);

    bt_ret = Bluetooth_Service_Add_Char(
        env_service_handle,
        HUMIDITY_CHAR_UUID,
        2,
        (Bluetooth_Char_Property)(Bluetooth_Char_Property_Read | Bluetooth_Char_Property_Notify),
        Bluetooth_Char_Permission_None,
        &humidity_char_handle
    );
    UART_Printf("Add Humidity Char=%d handle=0x%04X\r\n",
                bt_ret,
                humidity_char_handle);

    bt_ret = Bluetooth_Set_TX_Power(Bluetooth_PA_Level_7);
    UART_Printf("Bluetooth_Set_TX_Power=%d\r\n", bt_ret);

    bt_ret = Bluetooth_Set_Scan_Response_Data(NULL);
    UART_Printf("Bluetooth_Set_Scan_Response_Data=%d\r\n", bt_ret);

    bt_ret = Bluetooth_Set_Discoverable();
    UART_Printf("Bluetooth_Set_Discoverable=%d\r\n", bt_ret);

    while(1)
    {
        sensor_status = HTS221_GetTemperature(&temperature);
        if(sensor_status == HTS221_OK)
        {
            ble_temp = (int16_t)(temperature * 100.0f);

            bt_ret = Bluetooth_Service_Update_Char_Value(
                env_service_handle,
                temp_char_handle,
                0,
                2,
                (const uint8_t *)&ble_temp
            );

            UART_Printf("BLE Temp Update=%d %.2f\r\n", bt_ret, temperature);
        }

        sensor_status = HTS221_GetHumidity(&humidity);
        if(sensor_status == HTS221_OK)
        {
            ble_humidity = (uint16_t)(humidity * 100.0f);

            bt_ret = Bluetooth_Service_Update_Char_Value(
                env_service_handle,
                humidity_char_handle,
                0,
                2,
                (const uint8_t *)&ble_humidity
            );

            UART_Printf("BLE Humidity Update=%d %.2f\r\n", bt_ret, humidity);
        }

        NeonRTOS_Sleep(1000);
    }
}
*/

#define LED_G hwGPIO_Pin_B0
#define LED_B hwGPIO_Pin_B7
#define LED_R hwGPIO_Pin_B14

#define APP_THREAD_STACK_SIZE  2048
#define APP_THREAD_PRIORITY    1

void APP_Thread(void* p)
{
    GPIO_Pin_Init(LED_R, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    GPIO_Pin_Init(LED_B, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    GPIO_Pin_Init(LED_G, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);

    NeonTCPIP_init(NULL, NULL, NULL);

    HTTPd_Init();

    while (1)
    {
        if(NeonTCPIP_IF_isLinkUp())
        {
#if LWIP_DHCP
            if(NeonTCPIP_Get_DHCP_State()==DHCP_ADDRESS_ASSIGNED)
            {
                GPIO_Pin_Write(LED_B, 1);
                NeonRTOS_Sleep(500);

                GPIO_Pin_Write(LED_B, 0);
                NeonRTOS_Sleep(500);
            }
            else
            {
                GPIO_Pin_Write(LED_G, 1);
                NeonRTOS_Sleep(500);

                GPIO_Pin_Write(LED_G, 0);
                NeonRTOS_Sleep(500);
            }
#else
                GPIO_Pin_Write(LED_B, 1);
                NeonRTOS_Sleep(500);

                GPIO_Pin_Write(LED_B, 0);
                NeonRTOS_Sleep(500);
#endif
        }
        else
        {
            GPIO_Pin_Write(LED_R, 1);
            NeonRTOS_Sleep(200);

            GPIO_Pin_Write(LED_R, 0);
            NeonRTOS_Sleep(200);  
        }

        size_t freeHeapSize = NeonRTOS_GetFreeHeapSize();
        UART_Printf("Remain Heap Size %d bytes\n", freeHeapSize);
    }
}
int main(void) {
    SysCtrl_Init();

    DMA_Init();
    
    //__HAL_RCC_WWDG_CLK_DISABLE();  // 禁用窗口看門狗
    //__HAL_RCC_IWDG_CLK_DISABLE();  // 禁用獨立看門狗

    UART_Open(LOG_UART_INDEX, 115200, false);
    
/*
    NeonRTOS_TaskCreate(
        Sensor_Task,
        (const signed char *)"Sensor",
        1024,
        NULL,
        2,
        NULL
    );
*/
/*
    NeonRTOS_TaskCreate(
        BLE_Task,
        (const signed char *)"BLE",
        2048,
        NULL,
        2,
        NULL
    );
*/
	NeonRTOS_TaskCreate(
        APP_Thread,
        (signed char *)"APP Thread",
        APP_THREAD_STACK_SIZE,
        NULL,
        APP_THREAD_PRIORITY,
        NULL
    );

    //NFC_Demo_Init();

    // 啟動 NeonRTOS 調度器
    NeonRTOS_start();

    while (1);
}
//