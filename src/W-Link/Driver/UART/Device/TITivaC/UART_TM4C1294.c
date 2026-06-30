
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "UART/UART.h"

#if defined(TM4C1294)

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
            if (pin == hwGPIO_Pin_H0) return GPIO_PH0_U0RTS;
            if (pin == hwGPIO_Pin_H1) return GPIO_PH1_U0CTS;
            break;

        case hwUART_Index_1:
            if (pin == hwGPIO_Pin_B1) return GPIO_PB1_U1TX;
            if (pin == hwGPIO_Pin_B0) return GPIO_PB0_U1RX;
            if (pin == hwGPIO_Pin_N0) return GPIO_PN0_U1RTS;
            if (pin == hwGPIO_Pin_N1) return GPIO_PN1_U1CTS;
            break;

        case hwUART_Index_2:
            if (pin == hwGPIO_Pin_D5) return GPIO_PD5_U2TX;
            if (pin == hwGPIO_Pin_D4) return GPIO_PD4_U2RX;
            if (pin == hwGPIO_Pin_N2) return GPIO_PN2_U2RTS;
            if (pin == hwGPIO_Pin_N3) return GPIO_PN3_U2CTS;
            break;

        case hwUART_Index_3:
            if (pin == hwGPIO_Pin_J1) return GPIO_PJ1_U3TX;
            if (pin == hwGPIO_Pin_J0) return GPIO_PJ0_U3RX;
            if (pin == hwGPIO_Pin_N4) return GPIO_PN4_U3RTS;
            if (pin == hwGPIO_Pin_N5) return GPIO_PN5_U3CTS;
            break;

        case hwUART_Index_4:
            if (pin == hwGPIO_Pin_K1) return GPIO_PK1_U4TX;
            if (pin == hwGPIO_Pin_K0) return GPIO_PK0_U4RX;
            if (pin == hwGPIO_Pin_K2) return GPIO_PK2_U4RTS;
            if (pin == hwGPIO_Pin_K3) return GPIO_PK3_U4CTS;
            break;

        case hwUART_Index_5:
            if (pin == hwGPIO_Pin_C7) return GPIO_PC7_U5TX;
            if (pin == hwGPIO_Pin_C6) return GPIO_PC6_U5RX;
            break;

        case hwUART_Index_6:
            if (pin == hwGPIO_Pin_P1) return GPIO_PP1_U6TX;
            if (pin == hwGPIO_Pin_P0) return GPIO_PP0_U6RX;
            break;

        case hwUART_Index_7:
            if (pin == hwGPIO_Pin_C5) return GPIO_PC5_U7TX;
            if (pin == hwGPIO_Pin_C4) return GPIO_PC4_U7RX;
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

#endif //DEVICE_TM4C1294