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

#include "NFC/M24SR/M24SR.h"

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

int main(void) {
    SysCtrl_Init();

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
    NeonRTOS_TaskCreate(
        NFC_Task,
        (const signed char *)"NFC",
        2048,
        NULL,
        2,
        NULL
    );

    // 啟動 NeonRTOS 調度器
    NeonRTOS_start();

    while (1);
}
//