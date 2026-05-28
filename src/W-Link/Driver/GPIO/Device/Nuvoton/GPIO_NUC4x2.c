
#include "soc.h"

#include "GPIO/Pin/GPIO_Pin.h"

#include "GPIO/GPIO_Def.h"

#include "GPIO/GPIO.h"

#if defined(NUC442) || defined(NUC472)

static void GPIO_Nuvoton_DispatchIRQ(GPIO_T *port, hwGPIO_Int_Pin int_pin_start)
{
    uint32_t status = port->INTSRC;

    for(uint32_t i = 0; i < 16; i++)
    {
        uint32_t mask = (1UL << i);

        if(status & mask)
        {
            hwGPIO_Int_Pin irq_pin = int_pin_start + i;

            GPIO_Int_Handler(irq_pin);

            port->INTSRC = mask;
        }
    }
}

void GPA_IRQHandler(void)
{
    GPIO_Nuvoton_DispatchIRQ(GPA, hwGPIO_Int_Pin_A0);
}

void GPB_IRQHandler(void)
{
    GPIO_Nuvoton_DispatchIRQ(GPB, hwGPIO_Int_Pin_B0);
}

void GPC_IRQHandler(void)
{
    GPIO_Nuvoton_DispatchIRQ(GPC, hwGPIO_Int_Pin_C0);
}

void GPD_IRQHandler(void)
{
    GPIO_Nuvoton_DispatchIRQ(GPD, hwGPIO_Int_Pin_D0);
}

void GPE_IRQHandler(void)
{
    GPIO_Nuvoton_DispatchIRQ(GPE, hwGPIO_Int_Pin_E0);
}

void GPF_IRQHandler(void)
{
    GPIO_Nuvoton_DispatchIRQ(GPF, hwGPIO_Int_Pin_F0);
}

void GPG_IRQHandler(void)
{
    GPIO_Nuvoton_DispatchIRQ(GPG, hwGPIO_Int_Pin_G0);
}

void GPH_IRQHandler(void)
{
    GPIO_Nuvoton_DispatchIRQ(GPH, hwGPIO_Int_Pin_H0);
}

void GPI_IRQHandler(void)
{
    GPIO_Nuvoton_DispatchIRQ(GPI, hwGPIO_Int_Pin_I0);
}

hwGPIO_OpResult GPIO_Bit_Read(hwGPIO_Pin pin, bool *level)
{
    if(pin < 0 || pin >= hwGPIO_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if(level == NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_T *port = GPIO_Map_Soc_Base(pin);
    uint16_t pin_mask = GPIO_Map_Soc_Pin(pin);

    if(port == NULL || pin_mask == 0)
    {
        return hwGPIO_InvalidParameter;
    }

    *level = ((port->PIN & pin_mask) != 0);

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Bit_Write(hwGPIO_Pin pin, bool level)
{
    if(pin < 0 || pin >= hwGPIO_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_T *port = GPIO_Map_Soc_Base(pin);
    uint16_t pin_mask = GPIO_Map_Soc_Pin(pin);

    if(port == NULL || pin_mask == 0)
    {
        return hwGPIO_InvalidParameter;
    }

    if(level)
    {
        port->DOUT |= pin_mask;
    }
    else
    {
        port->DOUT &= ~pin_mask;
    }

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Bit_Toggle(hwGPIO_Pin pin)
{
    if(pin < 0 || pin >= hwGPIO_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_T *port = GPIO_Map_Soc_Base(pin);
    uint16_t pin_mask = GPIO_Map_Soc_Pin(pin);

    if(port == NULL || pin_mask == 0)
    {
        return hwGPIO_InvalidParameter;
    }

    port->DOUT ^= pin_mask;

    return hwGPIO_OK;
}

void GPIO_NVIC_Init(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin < 0 || irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return;
    }

    GPIO_T *port = GPIO_Int_Map_Soc_Base(irq_pin);
    if(port == NULL)
    {
        return;
    }
    
    if(port == GPA) NVIC_EnableIRQ(GPA_IRQn);
    if(port == GPB) NVIC_EnableIRQ(GPB_IRQn);
    if(port == GPC) NVIC_EnableIRQ(GPC_IRQn);
    if(port == GPD) NVIC_EnableIRQ(GPD_IRQn);
    if(port == GPE) NVIC_EnableIRQ(GPE_IRQn);
    if(port == GPF) NVIC_EnableIRQ(GPF_IRQn);
    if(port == GPG) NVIC_EnableIRQ(GPG_IRQn);
    if(port == GPH) NVIC_EnableIRQ(GPH_IRQn);
    if(port == GPI) NVIC_EnableIRQ(GPI_IRQn);
}

void GPIO_NVIC_DeInit(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin < 0 || irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return;
    }

    GPIO_T *port = GPIO_Int_Map_Soc_Base(irq_pin);

    if(port == NULL)
    {
        return;
    }

    if(port == GPA) NVIC_DisableIRQ(GPA_IRQn);
    if(port == GPB) NVIC_DisableIRQ(GPB_IRQn);
    if(port == GPC) NVIC_DisableIRQ(GPC_IRQn);
    if(port == GPD) NVIC_DisableIRQ(GPD_IRQn);
    if(port == GPE) NVIC_DisableIRQ(GPE_IRQn);
    if(port == GPF) NVIC_DisableIRQ(GPF_IRQn);
    if(port == GPG) NVIC_DisableIRQ(GPG_IRQn);
    if(port == GPH) NVIC_DisableIRQ(GPH_IRQn);
    if(port == GPI) NVIC_DisableIRQ(GPI_IRQn);
}

#endif