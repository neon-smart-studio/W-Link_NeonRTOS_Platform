#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"

#ifdef DEVICE_TIMSP432P

bool gpio_pin_init_status[hwGPIO_Pin_MAX] = {false};

static hwGPIO_Direction gpio_current_dir[hwGPIO_Int_Pin_MAX] = {hwGPIO_Direction_Input};
static hwGPIO_Pull_Mode gpio_current_mode[hwGPIO_Int_Pin_MAX] = {hwGPIO_Pull_Mode_None};
static hwGPIO_Interrupt_Mode gpio_current_irq_mode[hwGPIO_Int_Pin_MAX] = {hwGPIO_Interrupt_Mode_MAX};
static GPIO_Interrupt_Event_Handler gpio_irq_handlers[hwGPIO_Int_Pin_MAX];

hwGPIO_OpResult GPIO_Pin_Init(hwGPIO_Pin pin, hwGPIO_Direction dir, hwGPIO_Pull_Mode pull_mode)
{
    if(pin==hwGPIO_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(pin>=hwGPIO_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(dir>=hwGPIO_Direction_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(pull_mode>=hwGPIO_Pull_Mode_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(gpio_pin_init_status[pin]==true)
    {
      return hwGPIO_PinConflict;
    }
    
    uint8_t portBase = GPIO_Map_Soc_Port_Base(pin);
    uint16_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    switch (dir)
    {
        case hwGPIO_Direction_Input:
            switch (pull_mode)
            {
                case hwGPIO_Pull_Mode_None:
                    GPIO_setAsInputPin(portBase, pinMask);
                    break;

                case hwGPIO_Pull_Mode_Up:
                    GPIO_setAsInputPinWithPullUpResistor(portBase, pinMask);
                    break;

                case hwGPIO_Pull_Mode_Down:
                    GPIO_setAsInputPinWithPullDownResistor(portBase, pinMask);
                    break;

                case hwGPIO_Pull_Mode_OpenDrain:
                    return hwGPIO_InvalidParameter;

                default:
                    return hwGPIO_InvalidParameter;
            }
            break;

        case hwGPIO_Direction_Output:
        case hwGPIO_Direction_Output_Only:
            GPIO_setAsOutputPin(portBase, pinMask);
            break;

        default:
            return hwGPIO_InvalidParameter;
    }

    gpio_current_dir[pin] = dir;
    gpio_current_mode[pin] = pull_mode;

    gpio_pin_init_status[pin] = true;
    
    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_DeInit(hwGPIO_Pin pin)
{
    if(gpio_pin_init_status[pin]==false)
    {
      return hwGPIO_OK;
    }
    
    uint8_t portBase = GPIO_Map_Soc_Port_Base(pin);
    uint16_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    GPIO_setAsInputPin(portBase, pinMask);

    gpio_current_dir[pin] = hwGPIO_Direction_Input;
    gpio_current_mode[pin] = hwGPIO_Pull_Mode_None;
    
    gpio_pin_init_status[pin] = false;
    
    return hwGPIO_OK;
}

bool GPIO_Pin_is_Init(hwGPIO_Pin pin)
{
    if(pin>=hwGPIO_Pin_MAX)
    {
      return false;
    }
    
    return gpio_pin_init_status[pin];
}

hwGPIO_OpResult GPIO_Pin_Set_Direction(hwGPIO_Pin pin, hwGPIO_Direction dir)
{
    if(pin==hwGPIO_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(pin>=hwGPIO_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(dir>=hwGPIO_Direction_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(gpio_current_dir[pin]==hwGPIO_Direction_Output_Only)
    {
      return hwGPIO_Unsupport;
    }

    uint8_t portBase = GPIO_Map_Soc_Port_Base(pin);
    uint16_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    switch (dir)
    {
        case hwGPIO_Direction_Input:
            switch (gpio_current_mode[pin])
            {
                case hwGPIO_Pull_Mode_None:
                    GPIO_setAsInputPin(portBase, pinMask);
                    break;

                case hwGPIO_Pull_Mode_Up:
                    GPIO_setAsInputPinWithPullUpResistor(portBase, pinMask);
                    break;

                case hwGPIO_Pull_Mode_Down:
                    GPIO_setAsInputPinWithPullDownResistor(portBase, pinMask);
                    break;

                case hwGPIO_Pull_Mode_OpenDrain:
                    return hwGPIO_InvalidParameter;

                default:
                    return hwGPIO_InvalidParameter;
            }
            break;

        case hwGPIO_Direction_Output:
        case hwGPIO_Direction_Output_Only:
            GPIO_setAsOutputPin(portBase, pinMask);
            break;

        default:
            return hwGPIO_InvalidParameter;
    }

    gpio_current_dir[pin] = dir;
    
    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Get_Direction(hwGPIO_Pin pin, hwGPIO_Direction* dir)
{
    if(pin==hwGPIO_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(pin>=hwGPIO_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(dir==NULL)
    {
      return hwGPIO_InvalidParameter;
    }

    *dir = gpio_current_dir[pin];
  
    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Set_PullMode(hwGPIO_Pin pin, hwGPIO_Pull_Mode pull_mode)
{
    if(pin==hwGPIO_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(pin>=hwGPIO_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(pull_mode>=hwGPIO_Pull_Mode_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
  
    uint8_t portBase = GPIO_Map_Soc_Port_Base(pin);
    uint16_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }
    
    switch (gpio_current_dir[pin])
    {
        case hwGPIO_Direction_Input:
            switch (pull_mode)
            {
                case hwGPIO_Pull_Mode_None:
                    GPIO_setAsInputPin(portBase, pinMask);
                    break;

                case hwGPIO_Pull_Mode_Up:
                    GPIO_setAsInputPinWithPullUpResistor(portBase, pinMask);
                    break;

                case hwGPIO_Pull_Mode_Down:
                    GPIO_setAsInputPinWithPullDownResistor(portBase, pinMask);
                    break;

                case hwGPIO_Pull_Mode_OpenDrain:
                    return hwGPIO_InvalidParameter;

                default:
                    return hwGPIO_InvalidParameter;
            }
            break;

        case hwGPIO_Direction_Output:
        case hwGPIO_Direction_Output_Only:
            GPIO_setAsOutputPin(portBase, pinMask);
            break;

        default:
            return hwGPIO_InvalidParameter;
    }

    gpio_current_mode[pin] = pull_mode;
    
    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Get_PullMode(hwGPIO_Pin pin, hwGPIO_Pull_Mode* pull_mode)
{
    if(pin==hwGPIO_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(pin>=hwGPIO_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(pull_mode==NULL)
    {
      return hwGPIO_InvalidParameter;
    }
  
    *pull_mode = gpio_current_mode[pin];
  
    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Read(hwGPIO_Pin pin, bool* level)
{
    if(pin==hwGPIO_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(pin>=hwGPIO_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(level==NULL)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(gpio_current_dir[pin]==hwGPIO_Direction_Output_Only)
    {
      return hwGPIO_Unsupport;
    }

    uint8_t portBase = GPIO_Map_Soc_Port_Base(pin);
    uint16_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    *level = GPIO_getInputPinValue(portBase, pinMask) ? 1 : 0;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Write(hwGPIO_Pin pin, bool level)
{
    if(pin==hwGPIO_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(pin>=hwGPIO_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(gpio_current_dir[pin]==hwGPIO_Direction_Input)
    {
      return hwGPIO_Unsupport;
    }

    uint8_t portBase = GPIO_Map_Soc_Port_Base(pin);
    uint16_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    if(level)
    {
        GPIO_setOutputHighOnPin(portBase, pinMask);
    }
    else
    {
        GPIO_setOutputLowOnPin(portBase, pinMask);
    }
    
    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Toggle(hwGPIO_Pin pin)
{
    if(pin==hwGPIO_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(pin>=hwGPIO_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(gpio_current_dir[pin]==hwGPIO_Direction_Input)
    {
      return hwGPIO_Unsupport;
    }

    uint8_t portBase = GPIO_Map_Soc_Port_Base(pin);
    uint16_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    GPIO_toggleOutputOnPin(portBase, pinMask);
    
    return hwGPIO_OK;
}

void GPIO_IRQ_Handler(uint_fast8_t portBase)
{
    uint_fast16_t flags;
    uint_fast16_t bit;

    flags = GPIO_getEnabledInterruptStatus(portBase);
    GPIO_clearInterruptFlag(portBase, flags);

    for (bit = 0; bit < 16; bit++)
    {
        uint_fast16_t intMask = (GPIO_PIN0 << bit);

        if ((flags & intMask) == 0)
            continue;

        hwGPIO_Int_Pin intPin = GPIO_Map_Int_Pin_By_Mask(portBase, intMask);

        if (intPin < 0 || intPin >= hwGPIO_Int_Pin_MAX)
            continue;

        hwGPIO_Interrupt_Action action;

        switch (gpio_current_irq_mode[intPin])
        {
            case hwGPIO_Interrupt_Mode_Rising_Edge:
                action = hwGPIO_Interrupt_Action_Rising_Edge;
                break;

            case hwGPIO_Interrupt_Mode_Falling_Edge:
                action = hwGPIO_Interrupt_Action_Falling_Edge;
                break;

            case hwGPIO_Interrupt_Mode_Both_Edge:
                bool nowHigh = GPIO_getInputPinValue(portBase, intMask) ? 1 : 0;

                if (nowHigh)
                {
                    action = hwGPIO_Interrupt_Action_Rising_Edge;
                    GPIO_interruptEdgeSelect(portBase, intMask, GPIO_HIGH_TO_LOW_TRANSITION);
                }
                else
                {
                    action = hwGPIO_Interrupt_Action_Falling_Edge;
                    GPIO_interruptEdgeSelect(portBase, intMask, GPIO_LOW_TO_HIGH_TRANSITION);
                }
                break;

            default:
                continue;
        }

        if (gpio_irq_handlers[intPin] != NULL)
        {
            gpio_irq_handlers[intPin](intPin, action);
        }
    }
}

void GPIOA_IRQ_Handler(void) { GPIO_IRQ_Handler(GPIO_PORT_PA); }
void GPIOB_IRQ_Handler(void) { GPIO_IRQ_Handler(GPIO_PORT_PB); }
void GPIOC_IRQ_Handler(void) { GPIO_IRQ_Handler(GPIO_PORT_PC); }

hwGPIO_OpResult GPIO_Interrupt_Init(hwGPIO_Int_Pin irq_pin, hwGPIO_Interrupt_Mode mode)
{
    if(irq_pin==hwGPIO_Int_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(irq_pin>=hwGPIO_Int_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(mode>=hwGPIO_Interrupt_Mode_MAX)
    {
        return hwGPIO_InvalidParameter;
    }
  
    if(gpio_pin_init_status[irq_pin]==true)
    {
        return hwGPIO_PinConflict;
    }
    
    uint8_t portBase = GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint16_t pinMask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    if(portBase==GPIO_PORT_PD || portBase==GPIO_PORT_PE || portBase==GPIO_PORT_PJ)
    {
      return hwGPIO_Unsupport;
    }

    GPIO_Enable_Port_Clock(portBase);

    GPIO_setAsInputPin(portBase, pinMask);

    switch (mode)
    {
        case hwGPIO_Interrupt_Mode_Falling_Edge:
            GPIO_interruptEdgeSelect(portBase, pinMask, GPIO_HIGH_TO_LOW_TRANSITION);
            break;

        case hwGPIO_Interrupt_Mode_Rising_Edge:
            GPIO_interruptEdgeSelect(portBase, pinMask, GPIO_LOW_TO_HIGH_TRANSITION);
            break;

        case hwGPIO_Interrupt_Mode_Both_Edge:
            bool nowHigh = GPIO_getInputPinValue(portBase, pinMask) ? 1 : 0;

            if (nowHigh)
            {
                GPIO_interruptEdgeSelect(portBase, pinMask, GPIO_HIGH_TO_LOW_TRANSITION);
            }
            else
            {
                GPIO_interruptEdgeSelect(portBase, pinMask, GPIO_LOW_TO_HIGH_TRANSITION);
            }
            break;
    }

    GPIO_clearInterruptFlag(portBase, pinMask);

    switch (portBase)
    {
        case GPIO_PORT_PA:
            GPIO_registerInterrupt(GPIO_PORT_PA, GPIOA_IRQ_Handler);
            Interrupt_enableInterrupt(INT_PORT1);
            Interrupt_enableInterrupt(INT_PORT2);
            break;

        case GPIO_PORT_PB:
            GPIO_registerInterrupt(GPIO_PORT_PB, GPIOB_IRQ_Handler);
            Interrupt_enableInterrupt(INT_PORT3);
            Interrupt_enableInterrupt(INT_PORT4);
            break;

        case GPIO_PORT_PC:
            GPIO_registerInterrupt(GPIO_PORT_PC, GPIOC_IRQ_Handler);
            Interrupt_enableInterrupt(INT_PORT5);
            Interrupt_enableInterrupt(INT_PORT6);
            break;

        case GPIO_PORT_PD:
        case GPIO_PORT_PE:
        case GPIO_PORT_PJ:
            return hwGPIO_Unsupport;
    }

    GPIO_enableInterrupt(portBase, pinMask);

    gpio_current_irq_mode[irq_pin] = mode;

    gpio_pin_init_status[irq_pin] = true;
    
    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Interrupt_DeInit(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin==hwGPIO_Int_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(irq_pin>=hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }
    
    if(gpio_pin_init_status[irq_pin]==false)
    {
        return hwGPIO_OK;
    }
    
    uint8_t portBase = GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint16_t pinMask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    GPIO_disableInterrupt(portBase, pinMask);
    GPIO_clearInterruptFlag(portBase, pinMask);

    gpio_pin_init_status[irq_pin] = false;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Config_Interrupt_Mode(hwGPIO_Int_Pin irq_pin, hwGPIO_Interrupt_Mode mode)
{
    if(irq_pin==hwGPIO_Int_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(irq_pin>=hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }
  
    if(mode>=hwGPIO_Interrupt_Mode_MAX)
    {
        return hwGPIO_InvalidParameter;
    }
  
    uint8_t portBase = GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint16_t pinMask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    switch (mode)
    {
        case hwGPIO_Interrupt_Mode_Falling_Edge:
            GPIO_interruptEdgeSelect(portBase, pinMask, GPIO_HIGH_TO_LOW_TRANSITION);
            break;

        case hwGPIO_Interrupt_Mode_Rising_Edge:
            GPIO_interruptEdgeSelect(portBase, pinMask, GPIO_LOW_TO_HIGH_TRANSITION);
            break;

        case hwGPIO_Interrupt_Mode_Both_Edge:
        {
            bool nowHigh = GPIO_getInputPinValue(portBase, pinMask) ? true : false;

            if (nowHigh)
            {
                GPIO_interruptEdgeSelect(portBase, pinMask, GPIO_HIGH_TO_LOW_TRANSITION);
            }
            else
            {
                GPIO_interruptEdgeSelect(portBase, pinMask, GPIO_LOW_TO_HIGH_TRANSITION);
            }
            break;
        }

        default:
            return hwGPIO_InvalidParameter;
    }

    GPIO_clearInterruptFlag(portBase, pinMask);

    gpio_current_irq_mode[irq_pin] = mode;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Register_Interrupt_Handler(hwGPIO_Int_Pin irq_pin, GPIO_Interrupt_Event_Handler handler)
{
    if(irq_pin==hwGPIO_Int_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(irq_pin>=hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }
  
    if(handler==NULL)
    {
        return hwGPIO_InvalidParameter;
    }
  
    gpio_irq_handlers[irq_pin] = handler;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Unregister_Interrupt_Handler(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin==hwGPIO_Int_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(irq_pin>=hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }
  
    gpio_irq_handlers[irq_pin] = NULL;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Interrupt_Enable(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin==hwGPIO_Int_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(irq_pin>=hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }
  
    uint8_t portBase = GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint16_t pinMask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    GPIO_clearInterruptFlag(portBase, pinMask);
    GPIO_enableInterrupt(portBase, pinMask);

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Interrupt_Disable(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin==hwGPIO_Int_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(irq_pin>=hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }
  
    uint8_t portBase = GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint16_t pinMask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    GPIO_disableInterrupt(portBase, pinMask);
    GPIO_clearInterruptFlag(portBase, pinMask);

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Interrupt_Pin_Read(hwGPIO_Int_Pin irq_pin, bool* level)
{
    if(irq_pin>=hwGPIO_Int_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(level==NULL)
    {
      return hwGPIO_InvalidParameter;
    }
  
    uint8_t portBase = GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint16_t pinMask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    *level = GPIO_getInputPinValue(portBase, pinMask) ? 1 : 0;

    return hwGPIO_OK;
}

#endif //DEVICE_TIMSP432P
