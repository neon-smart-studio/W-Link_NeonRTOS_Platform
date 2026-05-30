
#include "soc.h"

#include "GPIO/Pin/GPIO_Pin.h"

#include "GPIO/GPIO_Def.h"

#include "GPIO/GPIO.h"

#ifdef DEVICE_NUVOTON

#include "GPIO_Nuvoton.h"

bool gpio_pin_init_status[hwGPIO_Pin_MAX] = {false};

static hwGPIO_Direction gpio_current_dir[hwGPIO_Pin_MAX] = {hwGPIO_Direction_Input};
static hwGPIO_Pull_Mode gpio_current_mode[hwGPIO_Pin_MAX] = {hwGPIO_Pull_Mode_None};

static GPIO_Interrupt_Event_Handler gpio_irq_handlers[hwGPIO_Int_Pin_MAX] = {NULL};

static uint32_t gpio_irq_mode[hwGPIO_Int_Pin_MAX] = {0};
static bool gpio_irq_enabled[hwGPIO_Int_Pin_MAX] = {false};

void GPIO_Int_Handler(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin < 0 || irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return;
    }
    
    GPIO_Interrupt_Event_Handler handler = gpio_irq_handlers[irq_pin];

    if(handler != NULL)
    {
        hwGPIO_Interrupt_Action action = hwGPIO_Interrupt_Action_Toggle;

        if(gpio_irq_mode[irq_pin] == GPIO_INT_RISING)
        {
            action = hwGPIO_Interrupt_Action_Rising_Edge;
        }
        else if(gpio_irq_mode[irq_pin] == GPIO_INT_FALLING)
        {
            action = hwGPIO_Interrupt_Action_Falling_Edge;
        }

        handler(irq_pin, action);
    }
}

bool GPIO_Pin_is_Init(hwGPIO_Pin pin)
{
    if(pin < 0 || pin >= hwGPIO_Pin_MAX)
    {
        return false;
    }

    return gpio_pin_init_status[pin];
}

hwGPIO_OpResult GPIO_Pin_Init(hwGPIO_Pin pin, hwGPIO_Direction dir, hwGPIO_Pull_Mode pull_mode)
{
    if(pin < 0 || pin >= hwGPIO_Pin_MAX)
        return hwGPIO_InvalidParameter;

    if(dir >= hwGPIO_Direction_MAX)
        return hwGPIO_InvalidParameter;

    if(pull_mode >= hwGPIO_Pull_Mode_MAX)
        return hwGPIO_InvalidParameter;

    if(gpio_pin_init_status[pin])
        return hwGPIO_PinConflict;

    GPIO_T *port = GPIO_Map_Soc_Base(pin);
    uint16_t pin_mask = GPIO_Map_Soc_Pin(pin);

    if(port == NULL || pin_mask == 0)
    {
        return hwGPIO_InvalidParameter;
    }

    switch(dir)
    {
        case hwGPIO_Direction_Input:
            GPIO_SetMode(port, pin_mask, GPIO_MODE_INPUT);
            break;

        case hwGPIO_Direction_Output:
        case hwGPIO_Direction_Output_Only:
            if(pull_mode == hwGPIO_Pull_Mode_OpenDrain)
            {
                GPIO_SetMode(port, pin_mask, GPIO_MODE_OPEN_DRAIN);
            }
            else
            {
                GPIO_SetMode(port, pin_mask, GPIO_MODE_OUTPUT);
            }
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
    if(pin < 0 || pin >= hwGPIO_Pin_MAX)
        return hwGPIO_InvalidParameter;

    if(!gpio_pin_init_status[pin])
        return hwGPIO_OK;

    GPIO_T *port = GPIO_Map_Soc_Base(pin);
    uint16_t pin_mask = GPIO_Map_Soc_Pin(pin);

    if(port == NULL || pin_mask == 0)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_SetMode(port, pin_mask, GPIO_MODE_INPUT);

    gpio_current_dir[pin] = hwGPIO_Direction_Input;
    gpio_current_mode[pin] = hwGPIO_Pull_Mode_None;
    gpio_pin_init_status[pin] = false;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Set_Direction(hwGPIO_Pin pin, hwGPIO_Direction dir)
{
    if(pin < 0 || pin >= hwGPIO_Pin_MAX)
        return hwGPIO_InvalidParameter;

    if(dir >= hwGPIO_Direction_MAX)
        return hwGPIO_InvalidParameter;

    if(gpio_current_dir[pin] == hwGPIO_Direction_Output_Only)
        return hwGPIO_Unsupport;

    GPIO_T *port = GPIO_Map_Soc_Base(pin);
    uint16_t pin_mask = GPIO_Map_Soc_Pin(pin);

    if(port == NULL || pin_mask == 0)
    {
        return hwGPIO_InvalidParameter;
    }

    switch(dir)
    {
        case hwGPIO_Direction_Input:
            GPIO_SetMode(port, pin_mask, GPIO_MODE_INPUT);
            break;

        case hwGPIO_Direction_Output:
            if(gpio_current_mode[pin] == hwGPIO_Pull_Mode_OpenDrain)
            {
                GPIO_SetMode(port, pin_mask, GPIO_MODE_OPEN_DRAIN);
            }
            else
            {
                GPIO_SetMode(port, pin_mask, GPIO_MODE_OUTPUT);
            }
            break;

        default:
            return hwGPIO_InvalidParameter;
    }

    gpio_current_dir[pin] = dir;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Get_Direction(hwGPIO_Pin pin, hwGPIO_Direction *dir)
{
    if(pin < 0 || pin >= hwGPIO_Pin_MAX)
        return hwGPIO_InvalidParameter;

    if(dir == NULL)
        return hwGPIO_InvalidParameter;

    *dir = gpio_current_dir[pin];

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Set_PullMode(hwGPIO_Pin pin, hwGPIO_Pull_Mode pull_mode)
{
    if(pin < 0 || pin >= hwGPIO_Pin_MAX)
        return hwGPIO_InvalidParameter;

    if(pull_mode >= hwGPIO_Pull_Mode_MAX)
        return hwGPIO_InvalidParameter;

    GPIO_T *port = GPIO_Map_Soc_Base(pin);
    uint16_t pin_mask = GPIO_Map_Soc_Pin(pin);

    if(port == NULL || pin_mask == 0)
        return hwGPIO_InvalidParameter;

    if(gpio_current_dir[pin] == hwGPIO_Direction_Input)
    {
        /*
         * NUC4x2 GPIO pull-up/down 沒有通用 gpio_pull_up/down API。
         * 先維持 input mode。若之後要支援 PUEN，可再補 port->PUEN 設定。
         */
        GPIO_SetMode(port, pin_mask, GPIO_MODE_INPUT);
    }
    else
    {
        if(pull_mode == hwGPIO_Pull_Mode_OpenDrain)
        {
            GPIO_SetMode(port, pin_mask, GPIO_MODE_OPEN_DRAIN);
        }
        else
        {
            GPIO_SetMode(port, pin_mask, GPIO_MODE_OUTPUT);
        }
    }

    gpio_current_mode[pin] = pull_mode;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Get_PullMode(hwGPIO_Pin pin, hwGPIO_Pull_Mode *pull_mode)
{
    if(pin < 0 || pin >= hwGPIO_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if(pull_mode == NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    *pull_mode = gpio_current_mode[pin];

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Read(hwGPIO_Pin pin, bool *level)
{
    if(pin < 0 || pin >= hwGPIO_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if(level == NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    if(gpio_current_dir[pin] == hwGPIO_Direction_Output_Only)
    {
        return hwGPIO_Unsupport;
    }

    GPIO_T *port = GPIO_Map_Soc_Base(pin);
    uint16_t pin_mask = GPIO_Map_Soc_Pin(pin);

    if(port == NULL || pin_mask == 0)
    {
        return hwGPIO_InvalidParameter;
    }

    return GPIO_Bit_Read(pin, level);
}

hwGPIO_OpResult GPIO_Pin_Write(hwGPIO_Pin pin, bool level)
{
    if(pin < 0 || pin >= hwGPIO_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if(gpio_current_dir[pin] == hwGPIO_Direction_Input)
    {
        return hwGPIO_Unsupport;
    }

    GPIO_T *port = GPIO_Map_Soc_Base(pin);
    uint16_t pin_mask = GPIO_Map_Soc_Pin(pin);

    if(port == NULL || pin_mask == 0)
    {
        return hwGPIO_InvalidParameter;
    }

    return GPIO_Bit_Write(pin, level);
}

hwGPIO_OpResult GPIO_Pin_Toggle(hwGPIO_Pin pin)
{
    if(pin < 0 || pin >= hwGPIO_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if(gpio_current_dir[pin] == hwGPIO_Direction_Input)
    {
        return hwGPIO_Unsupport;
    }

    GPIO_T *port = GPIO_Map_Soc_Base(pin);
    uint16_t pin_mask = GPIO_Map_Soc_Pin(pin);

    if(port == NULL || pin_mask == 0)
    {
        return hwGPIO_InvalidParameter;
    }

    return GPIO_Bit_Toggle(pin);
}

hwGPIO_OpResult GPIO_Interrupt_Init(hwGPIO_Int_Pin irq_pin, hwGPIO_Interrupt_Mode mode)
{
    if(irq_pin < 0 || irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if(mode >= hwGPIO_Interrupt_Mode_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if(gpio_pin_init_status[irq_pin])
    {
        return hwGPIO_PinConflict;
    }

    GPIO_T *port = GPIO_Int_Map_Soc_Base(irq_pin);
    uint16_t pin_mask = GPIO_Int_Map_Soc_Pin(irq_pin);
    uint32_t pin_index = GPIO_Int_Pin_To_Index(irq_pin);

    if(port == NULL || pin_mask == 0)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_SetMode(port, pin_mask, GPIO_MODE_INPUT);

    GPIO_DisableInt(port, pin_index);
    switch(mode)
    {
        case hwGPIO_Interrupt_Mode_Falling_Edge:
            GPIO_EnableInt(port, pin_index, GPIO_INT_FALLING);
            gpio_irq_mode[irq_pin] = GPIO_INT_FALLING;
            break;
        case hwGPIO_Interrupt_Mode_Rising_Edge:
            GPIO_EnableInt(port, pin_index, GPIO_INT_RISING);
            gpio_irq_mode[irq_pin] = GPIO_INT_RISING;
            break;
        case hwGPIO_Interrupt_Mode_Both_Edge:
            GPIO_EnableInt(port, pin_index, GPIO_INT_BOTH_EDGE);
            gpio_irq_mode[irq_pin] = GPIO_INT_BOTH_EDGE;
            break;
    }
    GPIO_DisableInt(port, pin_index);
    
    GPIO_NVIC_Init(irq_pin);

    gpio_pin_init_status[irq_pin] = true;
    gpio_current_dir[irq_pin] = hwGPIO_Direction_Input;
    gpio_current_mode[irq_pin] = hwGPIO_Pull_Mode_None;
    gpio_irq_enabled[irq_pin] = false;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Interrupt_DeInit(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin < 0 || irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if(!gpio_pin_init_status[irq_pin])
    {
        return hwGPIO_OK;
    }

    GPIO_T *port = GPIO_Int_Map_Soc_Base(irq_pin);
    uint32_t pin_index = GPIO_Int_Pin_To_Index(irq_pin);

    if(port == NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_NVIC_DeInit(irq_pin);

    GPIO_DisableInt(port, pin_index);

    gpio_pin_init_status[irq_pin] = false;
    gpio_irq_mode[irq_pin] = 0;
    gpio_irq_enabled[irq_pin] = false;
    gpio_irq_handlers[irq_pin] = NULL;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Config_Interrupt_Mode(hwGPIO_Int_Pin irq_pin, hwGPIO_Interrupt_Mode mode)
{
    if(irq_pin < 0 || irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if(mode >= hwGPIO_Interrupt_Mode_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_T *port = GPIO_Int_Map_Soc_Base(irq_pin);
    uint32_t pin_index = GPIO_Int_Pin_To_Index(irq_pin);

    if(port == NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_DisableInt(port, pin_index);

    if(gpio_irq_enabled[irq_pin])
    {
        switch(mode)
        {
            case hwGPIO_Interrupt_Mode_Falling_Edge:
                GPIO_EnableInt(port, pin_index, GPIO_INT_FALLING);
                gpio_irq_mode[irq_pin] = GPIO_INT_FALLING;
                break;
            case hwGPIO_Interrupt_Mode_Rising_Edge:
                GPIO_EnableInt(port, pin_index, GPIO_INT_RISING);
                gpio_irq_mode[irq_pin] = GPIO_INT_RISING;
                break;
            case hwGPIO_Interrupt_Mode_Both_Edge:
                GPIO_EnableInt(port, pin_index, GPIO_INT_BOTH_EDGE);
                gpio_irq_mode[irq_pin] = GPIO_INT_BOTH_EDGE;
                break;
        }
    }

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Register_Interrupt_Handler(hwGPIO_Int_Pin irq_pin, GPIO_Interrupt_Event_Handler handler)
{
    if(irq_pin < 0 || irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if(handler == NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    gpio_irq_handlers[irq_pin] = handler;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Unregister_Interrupt_Handler(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin < 0 || irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    gpio_irq_handlers[irq_pin] = NULL;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Interrupt_Enable(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin < 0 || irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if(gpio_irq_mode[irq_pin] == 0)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_T *port = GPIO_Int_Map_Soc_Base(irq_pin);
    uint32_t pin_index = GPIO_Int_Pin_To_Index(irq_pin);

    if(port == NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_EnableInt(port, pin_index, gpio_irq_mode[irq_pin]);

    gpio_irq_enabled[irq_pin] = true;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Interrupt_Disable(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin < 0 || irq_pin >= hwGPIO_Int_Pin_MAX)
        return hwGPIO_InvalidParameter;

    GPIO_T *port = GPIO_Int_Map_Soc_Base(irq_pin);
    uint32_t pin_index = GPIO_Int_Pin_To_Index(irq_pin);

    if(port == NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_DisableInt(port, pin_index);

    gpio_irq_enabled[irq_pin] = false;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Interrupt_Pin_Read(hwGPIO_Int_Pin pin, bool *level)
{
    if(pin < 0 || pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    return GPIO_Pin_Read((hwGPIO_Pin)pin, level);
}

#endif //DEVICE_NUVOTON