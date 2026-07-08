
#include <stdbool.h>
#include <stdint.h>

#include "GPIO_Pin_TIMSP432P.h"

#include "soc.h"

#ifdef DEVICE_TIMSP432P

uint8_t GPIO_Map_Soc_Port_Base(hwGPIO_Pin pin)
{
    if (pin >= hwGPIO_Pin_PA0 && pin <= hwGPIO_Pin_PA15) return GPIO_PORT_PA;
    if (pin >= hwGPIO_Pin_PB0 && pin <= hwGPIO_Pin_PB15) return GPIO_PORT_PB;
    if (pin >= hwGPIO_Pin_PC0 && pin <= hwGPIO_Pin_PC15) return GPIO_PORT_PC;
    if (pin >= hwGPIO_Pin_PD0 && pin <= hwGPIO_Pin_PD15) return GPIO_PORT_PD;
    if (pin >= hwGPIO_Pin_PE0 && pin <= hwGPIO_Pin_PE15) return GPIO_PORT_PE;
    if (pin >= hwGPIO_Pin_PJ0 && pin <= hwGPIO_Pin_PJ3)  return GPIO_PORT_PJ;

    return 0;
}

uint8_t GPIO_Map_Soc_Int_Port_Base(hwGPIO_Int_Pin pin)
{
    if (pin >= hwGPIO_Int_Pin_PA0 && pin <= hwGPIO_Int_Pin_PA15) return GPIO_PORT_PA;
    if (pin >= hwGPIO_Int_Pin_PB0 && pin <= hwGPIO_Int_Pin_PB15) return GPIO_PORT_PB;
    if (pin >= hwGPIO_Int_Pin_PC0 && pin <= hwGPIO_Int_Pin_PC15) return GPIO_PORT_PC;
    if (pin >= hwGPIO_Int_Pin_PD0 && pin <= hwGPIO_Int_Pin_PD15) return GPIO_PORT_PD;
    if (pin >= hwGPIO_Int_Pin_PE0 && pin <= hwGPIO_Int_Pin_PE15) return GPIO_PORT_PE;
    if (pin >= hwGPIO_Int_Pin_PJ0 && pin <= hwGPIO_Int_Pin_PJ3)  return GPIO_PORT_PJ;

    return 0;
}

uint16_t GPIO_Map_Soc_Pin_Mask(hwGPIO_Pin pin)
{
    if (pin == hwGPIO_Pin_NC)
        return 0;

    if (pin >= hwGPIO_Pin_PA0 && pin <= hwGPIO_Pin_PE15)
        return (GPIO_PIN0 << ((uint_fast16_t)pin & 0x0F));

    if (pin >= hwGPIO_Pin_PJ0 && pin <= hwGPIO_Pin_PJ3)
        return (GPIO_PIN0 << ((uint_fast16_t)(pin - hwGPIO_Pin_PJ0)));

    return 0;
}

uint16_t GPIO_Map_Soc_Pin_Mask(hwGPIO_Int_Pin pin)
{
    if (pin == hwGPIO_Int_Pin_NC)
        return 0;

    if (pin >= hwGPIO_Int_Pin_PA0 && pin <= hwGPIO_Int_Pin_PE15)
        return (GPIO_PIN0 << ((uint_fast16_t)pin & 0x0F));

    if (pin >= hwGPIO_Int_Pin_PJ0 && pin <= hwGPIO_Int_Pin_PJ3)
        return (GPIO_PIN0 << ((uint_fast16_t)(pin - hwGPIO_Int_Pin_PJ0)));

    return 0;
}

#endif //DEVICE_TIMSP432P
