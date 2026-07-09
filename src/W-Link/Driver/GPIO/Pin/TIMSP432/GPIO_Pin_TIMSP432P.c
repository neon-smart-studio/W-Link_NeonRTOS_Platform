
#include <stdbool.h>
#include <stdint.h>

#include "GPIO_Pin_TIMSP432P.h"

#include "soc.h"

#ifdef DEVICE_TIMSP432P

uint8_t GPIO_Map_Soc_Port_Base(hwGPIO_Pin pin)
{
    if (pin >= hwGPIO_Pin_A0 && pin <= hwGPIO_Pin_A15) return GPIO_PORT_PA;
    if (pin >= hwGPIO_Pin_B0 && pin <= hwGPIO_Pin_B15) return GPIO_PORT_PB;
    if (pin >= hwGPIO_Pin_C0 && pin <= hwGPIO_Pin_C15) return GPIO_PORT_PC;
    if (pin >= hwGPIO_Pin_D0 && pin <= hwGPIO_Pin_D15) return GPIO_PORT_PD;
    if (pin >= hwGPIO_Pin_E0 && pin <= hwGPIO_Pin_E15) return GPIO_PORT_PE;
    if (pin >= hwGPIO_Pin_J0 && pin <= hwGPIO_Pin_J3)  return GPIO_PORT_PJ;

    return 0;
}

uint8_t GPIO_Map_Soc_Int_Port_Base(hwGPIO_Int_Pin pin)
{
    if (pin >= hwGPIO_Int_Pin_A0 && pin <= hwGPIO_Int_Pin_A15) return GPIO_PORT_PA;
    if (pin >= hwGPIO_Int_Pin_B0 && pin <= hwGPIO_Int_Pin_B15) return GPIO_PORT_PB;
    if (pin >= hwGPIO_Int_Pin_C0 && pin <= hwGPIO_Int_Pin_C15) return GPIO_PORT_PC;
    if (pin >= hwGPIO_Int_Pin_D0 && pin <= hwGPIO_Int_Pin_D15) return GPIO_PORT_PD;
    if (pin >= hwGPIO_Int_Pin_E0 && pin <= hwGPIO_Int_Pin_E15) return GPIO_PORT_PE;
    if (pin >= hwGPIO_Int_Pin_J0 && pin <= hwGPIO_Int_Pin_J3)  return GPIO_PORT_PJ;

    return 0;
}

uint16_t GPIO_Map_Soc_Pin_Mask(hwGPIO_Pin pin)
{
    if (pin == hwGPIO_Pin_NC)
        return 0;

    if (pin >= hwGPIO_Pin_A0 && pin <= hwGPIO_Pin_E15)
        return (GPIO_PIN0 << ((uint_fast16_t)pin & 0x0F));

    if (pin >= hwGPIO_Pin_J0 && pin <= hwGPIO_Pin_J3)
        return (GPIO_PIN0 << ((uint_fast16_t)(pin - hwGPIO_Pin_J0)));

    return 0;
}

uint16_t GPIO_Map_Soc_Int_Pin_Mask(hwGPIO_Int_Pin pin)
{
    if (pin == hwGPIO_Int_Pin_NC)
        return 0;

    if (pin >= hwGPIO_Int_Pin_A0 && pin <= hwGPIO_Int_Pin_E15)
        return (GPIO_PIN0 << ((uint_fast16_t)pin & 0x0F));

    if (pin >= hwGPIO_Int_Pin_J0 && pin <= hwGPIO_Int_Pin_J3)
        return (GPIO_PIN0 << ((uint_fast16_t)(pin - hwGPIO_Int_Pin_J0)));

    return 0;
}

hwGPIO_Int_Pin GPIO_Map_Int_Pin_By_Mask(uint8_t portBase, uint16_t intMask)
{
    uint8_t bit;

    if (intMask == 0)
        return hwGPIO_Int_Pin_NC;

    for (bit = 0; bit < 16; bit++)
    {
        if (intMask & (GPIO_PIN0 << bit))
            break;
    }

    switch (portBase)
    {
        case GPIO_PORT_PA:
            if (bit <= 15)
                return (hwGPIO_Int_Pin)(hwGPIO_Int_Pin_A0 + bit);
            break;

        case GPIO_PORT_PB:
            if (bit <= 15)
                return (hwGPIO_Int_Pin)(hwGPIO_Int_Pin_B0 + bit);
            break;

        case GPIO_PORT_PC:
            if (bit <= 15)
                return (hwGPIO_Int_Pin)(hwGPIO_Int_Pin_C0 + bit);
            break;

        case GPIO_PORT_PD:
            if (bit <= 15)
                return (hwGPIO_Int_Pin)(hwGPIO_Int_Pin_D0 + bit);
            break;

        case GPIO_PORT_PE:
            if (bit <= 15)
                return (hwGPIO_Int_Pin)(hwGPIO_Int_Pin_E0 + bit);
            break;

        case GPIO_PORT_PJ:
            if (bit <= 3)
                return (hwGPIO_Int_Pin)(hwGPIO_Int_Pin_J0 + bit);
            break;
    }

    return hwGPIO_Int_Pin_NC;
}

#endif //DEVICE_TIMSP432P
