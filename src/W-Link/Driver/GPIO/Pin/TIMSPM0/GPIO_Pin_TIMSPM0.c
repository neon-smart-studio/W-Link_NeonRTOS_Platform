#include <stdbool.h>
#include <stdint.h>

#include "GPIO_Pin_TIMSPM0.h"

#include "soc.h"

#ifdef DEVICE_TIMSPM0

GPIO_Regs* GPIO_Map_Soc_Port_Base(hwGPIO_Pin pin)
{
#if defined(GPIOA_BASE)
    if (pin >= hwGPIO_Pin_A0 && pin <= hwGPIO_Pin_A31)
    {
        return GPIOA_BASE;
    }
#endif
#if defined(GPIOB_BASE)
    if (pin >= hwGPIO_Pin_B0 && pin <= hwGPIO_Pin_B31)
    {
        return GPIOB_BASE;
    }
#endif
#if defined(GPIOC_BASE)
    if (pin >= hwGPIO_Pin_C0 && pin <= hwGPIO_Pin_C29)
    {
        return GPIOC_BASE;
    }
#endif

    return NULL;
}

GPIO_Regs* GPIO_Map_Soc_Int_Port_Base(hwGPIO_Int_Pin pin)
{
#if defined(GPIOA_BASE)
    if (pin >= hwGPIO_Int_Pin_A0 && pin <= hwGPIO_Int_Pin_A31)
    {
        return GPIOA_BASE;
    }
#endif
#if defined(GPIOB_BASE)
    if (pin >= hwGPIO_Int_Pin_B0 && pin <= hwGPIO_Int_Pin_B31)
    {
        return GPIOB_BASE;
    }
#endif
#if defined(GPIOC_BASE)
    if (pin >= hwGPIO_Int_Pin_C0 && pin <= hwGPIO_Int_Pin_C29)
    {
        return GPIOC_BASE;
    }
#endif

    return NULL;
}

uint32_t GPIO_Map_Soc_Pin_Mask(hwGPIO_Pin pin)
{
    if (pin < hwGPIO_Pin_A0 || pin >= hwGPIO_Pin_MAX)
        return 0;

    return (uint32_t)1UL << (uint32_t)pin;
}

uint32_t GPIO_Map_Soc_Int_Pin_Mask(hwGPIO_Int_Pin pin)
{
    if (pin < hwGPIO_Int_Pin_A0 || pin >= hwGPIO_Int_Pin_MAX)
        return 0;

    return (uint32_t)1UL << (uint32_t)pin;
}

hwGPIO_Int_Pin GPIO_Map_Int_Pin_By_Mask(GPIO_Regs* portBase, uint32_t intMask)
{
    uint8_t bit = 0;

    if (intMask == 0)
        return hwGPIO_Int_Pin_NC;

    while (((intMask >> bit) & 0x01UL) == 0UL)
    {
        bit++;

        if (bit >= 32)
            return hwGPIO_Int_Pin_NC;
    }

#if defined(GPIOA_BASE)
    if(portBase==GPIOA_BASE)
    {
        return (hwGPIO_Int_Pin)(hwGPIO_Int_Pin_A0 + bit);
    }
#endif
#if defined(GPIOB_BASE)
    if(portBase==GPIOB_BASE)
    {
        return (hwGPIO_Int_Pin)(hwGPIO_Int_Pin_B0 + bit);
    }
#endif
#if defined(GPIOC_BASE)
    if(portBase==GPIOC_BASE)
    {
        return (hwGPIO_Int_Pin)(hwGPIO_Int_Pin_C0 + bit);
    }
#endif

    return hwGPIO_Int_Pin_NC;
}

#endif /* DEVICE_TIMSPM0 */