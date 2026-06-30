
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "UART/UART.h"

#ifdef DEVICE_TM4C123

#include "UART_TITivaC.h"

uint32_t UART_Map_Soc_Base(hwUART_Index index)
{
    switch(index)
    {
        case hwUART_Index_0: return UART0_BASE;
        case hwUART_Index_1: return UART1_BASE;
        case hwUART_Index_2: return UART2_BASE;
        case hwUART_Index_3: return UART3_BASE;
        case hwUART_Index_4: return UART4_BASE;
        case hwUART_Index_5: return UART5_BASE;
        case hwUART_Index_6: return UART6_BASE;
        case hwUART_Index_7: return UART7_BASE;
        default: return 0;
    }
}

uint32_t UART_Map_Soc_Periph(hwUART_Index index)
{
    switch(index)
    {
        case hwUART_Index_0: return SYSCTL_PERIPH_UART0;
        case hwUART_Index_1: return SYSCTL_PERIPH_UART1;
        case hwUART_Index_2: return SYSCTL_PERIPH_UART2;
        case hwUART_Index_3: return SYSCTL_PERIPH_UART3;
        case hwUART_Index_4: return SYSCTL_PERIPH_UART4;
        case hwUART_Index_5: return SYSCTL_PERIPH_UART5;
        case hwUART_Index_6: return SYSCTL_PERIPH_UART6;
        case hwUART_Index_7: return SYSCTL_PERIPH_UART7;
        default: return 0;
    }
}

uint32_t UART_Map_IRQ(hwUART_Index index)
{
    switch(index)
    {
        case hwUART_Index_0: return INT_UART0;
        case hwUART_Index_1: return INT_UART1;
        case hwUART_Index_2: return INT_UART2;
        case hwUART_Index_3: return INT_UART3;
        case hwUART_Index_4: return INT_UART4;
        case hwUART_Index_5: return INT_UART5;
        case hwUART_Index_6: return INT_UART6;
        case hwUART_Index_7: return INT_UART7;
        default: return 0;
    }
}

uint32_t UART_Map_PinConfig(hwUART_Index index, hwGPIO_Pin pin)
{
    switch (index)
    {
        case hwUART_Index_0:
            if (pin == hwGPIO_Pin_A1) return GPIO_PA1_U0TX;
            if (pin == hwGPIO_Pin_A0) return GPIO_PA0_U0RX;
            break;

        case hwUART_Index_1:
            if (pin == hwGPIO_Pin_B1) return GPIO_PB1_U1TX;
            if (pin == hwGPIO_Pin_B0) return GPIO_PB0_U1RX;
            break;

        case hwUART_Index_2:
            if (pin == hwGPIO_Pin_D7) return GPIO_PD7_U2TX;
            if (pin == hwGPIO_Pin_D6) return GPIO_PD6_U2RX;
            break;

        case hwUART_Index_3:
            if (pin == hwGPIO_Pin_C7) return GPIO_PC7_U3TX;
            if (pin == hwGPIO_Pin_C6) return GPIO_PC6_U3RX;
            break;

        case hwUART_Index_4:
            if (pin == hwGPIO_Pin_C5) return GPIO_PC5_U4TX;
            if (pin == hwGPIO_Pin_C4) return GPIO_PC4_U4RX;
            break;

        case hwUART_Index_5:
            if (pin == hwGPIO_Pin_E5) return GPIO_PE5_U5TX;
            if (pin == hwGPIO_Pin_E4) return GPIO_PE4_U5RX;
            break;

        case hwUART_Index_6:
            if (pin == hwGPIO_Pin_D5) return GPIO_PD5_U6TX;
            if (pin == hwGPIO_Pin_D4) return GPIO_PD4_U6RX;
            break;

        case hwUART_Index_7:
            if (pin == hwGPIO_Pin_E1) return GPIO_PE1_U7TX;
            if (pin == hwGPIO_Pin_E0) return GPIO_PE0_U7RX;
            break;

        default:
            break;
    }

    return 0;
}

void UART0_IRQ_Handler(void) { UART_IRQ_Process(hwUART_Index_0); }
void UART1_IRQ_Handler(void) { UART_IRQ_Process(hwUART_Index_1); }
void UART2_IRQ_Handler(void) { UART_IRQ_Process(hwUART_Index_2); }
void UART3_IRQ_Handler(void) { UART_IRQ_Process(hwUART_Index_3); }
void UART4_IRQ_Handler(void) { UART_IRQ_Process(hwUART_Index_4); }
void UART5_IRQ_Handler(void) { UART_IRQ_Process(hwUART_Index_5); }
void UART6_IRQ_Handler(void) { UART_IRQ_Process(hwUART_Index_6); }
void UART7_IRQ_Handler(void) { UART_IRQ_Process(hwUART_Index_7); }

void UART_NVIC_Enable(hwUART_Index index)
{
    uint32_t base = UART_Map_Soc_Base(index);
    uint32_t irq = UART_Map_IRQ(index);

    if (base == 0)
    {
        return;
    }

    switch (index)
    {
        case hwUART_Index_0:
            UARTIntRegister(base, UART0_IRQ_Handler);
            break;

        case hwUART_Index_1:
            UARTIntRegister(base, UART1_IRQ_Handler);
            break;

        case hwUART_Index_2:
            UARTIntRegister(base, UART2_IRQ_Handler);
            break;

        case hwUART_Index_3:
            UARTIntRegister(base, UART3_IRQ_Handler);
            break;

        case hwUART_Index_4:
            UARTIntRegister(base, UART4_IRQ_Handler);
            break;

        case hwUART_Index_5:
            UARTIntRegister(base, UART5_IRQ_Handler);
            break;

        case hwUART_Index_6:
            UARTIntRegister(base, UART6_IRQ_Handler);
            break;

        case hwUART_Index_7:
            UARTIntRegister(base, UART7_IRQ_Handler);
            break;

        default:
            return;
    }

    MAP_UARTIntDisable(base, UART_INT_RX | UART_INT_RT | UART_INT_TX);
    MAP_UARTIntClear(base, 0xFFFFFFFF);

    if (irq != 0)
    {
        MAP_IntEnable(irq);
    }
}

void UART_NVIC_Disable(hwUART_Index index)
{
    uint32_t base = UART_Map_Soc_Base(index);
    uint32_t irq = UART_Map_IRQ(index);

    if (base != 0)
    {
        MAP_UARTIntDisable(base, UART_INT_RX | UART_INT_RT | UART_INT_TX);
        MAP_UARTIntClear(base, 0xFFFFFFFF);

        UARTIntUnregister(base);
    }

    if (irq != 0)
    {
        MAP_IntDisable(irq);
    }
}

#endif //DEVICE_TM4C123