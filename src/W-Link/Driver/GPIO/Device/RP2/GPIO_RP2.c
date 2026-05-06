
#include "soc.h"

#include "GPIO/Pin/GPIO_Pin.h"

#include "GPIO/GPIO_Def.h"

#include "GPIO/GPIO.h"

#ifdef DEVICE_RP2

#include "GPIO_RP2.h"

bool gpio_pin_init_status[hwGPIO_Pin_MAX] = {false};

static hwGPIO_Direction gpio_current_dir[hwGPIO_Pin_MAX] = {hwGPIO_Direction_Input};
static hwGPIO_Pull_Mode gpio_current_mode[hwGPIO_Pin_MAX] = {hwGPIO_Pull_Mode_None};

static GPIO_Interrupt_Event_Handler gpio_irq_handlers[hwGPIO_Int_Pin_MAX] = {NULL};

static uint32_t gpio_irq_mask[hwGPIO_Int_Pin_MAX] = {0};
static bool gpio_irq_enabled[hwGPIO_Int_Pin_MAX] = {false};

bool GPIO_Pin_is_Init(hwGPIO_Pin pin)
{
    if(pin>=hwGPIO_Pin_MAX)
    {
      return false;
    }
    
    return gpio_pin_init_status[pin];
}

hwGPIO_OpStatus GPIO_Pin_Init(hwGPIO_Pin pin, hwGPIO_Direction dir, hwGPIO_Pull_Mode pull_mode)
{
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
  
    if(pull_mode==hwGPIO_Pull_Mode_OpenDrain)
    {
      return hwGPIO_Unsupport;
    }
  
    if(gpio_pin_init_status[pin]==true)
    {
      return hwGPIO_PinConflict;
    }
    
    gpio_init(pin);

    switch(dir)
    {
        case hwGPIO_Direction_Input:
            gpio_set_dir(pin, GPIO_IN);
            gpio_disable_pulls(pin);
            break;
        case hwGPIO_Direction_Output:
        case hwGPIO_Direction_Output_Only:
            gpio_set_dir(pin, GPIO_OUT);

            switch(pull_mode)
            {
                case hwGPIO_Pull_Mode_None:
                    gpio_disable_pulls(pin);
                    break;
                case hwGPIO_Pull_Mode_Up:
                    gpio_pull_up(pin);
                    break;
                case hwGPIO_Pull_Mode_Down:
                    gpio_pull_down(pin);
                    break;
            }
            break;
    }
    
    gpio_current_dir[pin] = dir;
    gpio_current_mode[pin] = pull_mode;

    gpio_pin_init_status[pin] = true;
    
    return hwGPIO_OK;
}

hwGPIO_OpStatus GPIO_Pin_DeInit(hwGPIO_Pin pin)
{
    if(gpio_pin_init_status[pin]==false)
    {
      return hwGPIO_OK;
    }
    
    gpio_deinit(pin);

    gpio_current_dir[pin] = hwGPIO_Direction_Input;
    gpio_current_mode[pin] = hwGPIO_Pull_Mode_None;
    
    gpio_pin_init_status[pin] = false;
    
    return hwGPIO_OK;
}

hwGPIO_OpStatus GPIO_Pin_Set_Direction(hwGPIO_Pin pin, hwGPIO_Direction dir)
{
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

    switch (dir) {
        case hwGPIO_Direction_Input:
            gpio_set_dir(pin, GPIO_IN);
            break;

        case hwGPIO_Direction_Output:
            gpio_set_dir(pin, GPIO_OUT);
            break;
    }

    switch(dir)
    {
        case hwGPIO_Direction_Input:
            gpio_disable_pulls(pin);
            break;
        case hwGPIO_Direction_Output:
        case hwGPIO_Direction_Output_Only:
            switch(gpio_current_mode[pin])
            {
                case hwGPIO_Pull_Mode_None:
                    gpio_disable_pulls(pin);
                    break;
                case hwGPIO_Pull_Mode_Up:
                    gpio_pull_up(pin);
                    break;
                case hwGPIO_Pull_Mode_Down:
                    gpio_pull_down(pin);
                    break;
            }
            break;
    }
    
    gpio_current_dir[pin] = dir;
    
    return hwGPIO_OK;
}

hwGPIO_OpStatus GPIO_Pin_Get_Direction(hwGPIO_Pin pin, hwGPIO_Direction* dir)
{
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

hwGPIO_OpStatus GPIO_Pin_Set_PullMode(hwGPIO_Pin pin, hwGPIO_Pull_Mode pull_mode)
{
    if(pin>=hwGPIO_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(pull_mode>=hwGPIO_Pull_Mode_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(pull_mode==hwGPIO_Pull_Mode_OpenDrain)
    {
      return hwGPIO_Unsupport;
    }
  
    switch(gpio_current_dir[pin])
    {
        case hwGPIO_Direction_Input:
            gpio_set_dir(pin, GPIO_IN);
            gpio_disable_pulls(pin);
            break;
        case hwGPIO_Direction_Output:
        case hwGPIO_Direction_Output_Only:
            gpio_set_dir(pin, GPIO_OUT);

            switch(pull_mode)
            {
                case hwGPIO_Pull_Mode_None:
                    gpio_disable_pulls(pin);
                    break;
                case hwGPIO_Pull_Mode_Up:
                    gpio_pull_up(pin);
                    break;
                case hwGPIO_Pull_Mode_Down:
                    gpio_pull_down(pin);
                    break;
            }
            break;
    }
    
    gpio_current_mode[pin] = pull_mode;
    
    return hwGPIO_OK;
}

hwGPIO_OpStatus GPIO_Pin_Get_PullMode(hwGPIO_Pin pin, hwGPIO_Pull_Mode* pull_mode)
{
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

hwGPIO_OpStatus GPIO_Pin_Read(hwGPIO_Pin pin, bool* level)
{
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

    *level = gpio_get(pin);
    
    return hwGPIO_OK;
}

hwGPIO_OpStatus GPIO_Pin_Write(hwGPIO_Pin pin, bool level)
{
    if(pin>=hwGPIO_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(gpio_current_dir[pin]==hwGPIO_Direction_Input)
    {
      return hwGPIO_Unsupport;
    }

    gpio_put(pin, level);

    return hwGPIO_OK;
}

hwGPIO_OpStatus GPIO_Pin_Toggle(hwGPIO_Pin pin)
{
    if(pin>=hwGPIO_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
  
    if(gpio_current_dir[pin]==hwGPIO_Direction_Input)
    {
      return hwGPIO_Unsupport;
    }

    bool level = gpio_get(pin);
    
    gpio_put(pin, !level);
    
    return hwGPIO_OK;
}

static void rpi_gpio_irq_callback(uint irq_pin, uint32_t events)
{
    if (irq_pin >= hwGPIO_Int_Pin_MAX) return;

    if(gpio_irq_handlers[irq_pin]!=NULL)
    {
        if ((events & GPIO_IRQ_EDGE_RISE) && !(events & GPIO_IRQ_EDGE_FALL)) {
            gpio_irq_handlers[irq_pin]((hwGPIO_Int_Pin)irq_pin, hwGPIO_Interrupt_Action_Rising_Edge);
        }

        if (!(events & GPIO_IRQ_EDGE_RISE) && (events & GPIO_IRQ_EDGE_FALL)) {
            gpio_irq_handlers[irq_pin]((hwGPIO_Int_Pin)irq_pin, hwGPIO_Interrupt_Action_Falling_Edge);
        }
        
        if ((events & GPIO_IRQ_EDGE_RISE) && (events & GPIO_IRQ_EDGE_FALL)) {
            gpio_irq_handlers[irq_pin]((hwGPIO_Int_Pin)irq_pin, hwGPIO_Interrupt_Action_Toggle);
        }
    }
}

hwGPIO_OpStatus GPIO_Interrupt_Init(hwGPIO_Int_Pin irq_pin, hwGPIO_Interrupt_Mode mode)
{
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
    
    uint32_t event_mask = 0;

    switch (mode) {
        case hwGPIO_Interrupt_Mode_Rising_Edge:
            event_mask = GPIO_IRQ_EDGE_RISE;
            break;

        case hwGPIO_Interrupt_Mode_Falling_Edge:
            event_mask = GPIO_IRQ_EDGE_FALL;
            break;

        case hwGPIO_Interrupt_Mode_Both_Edge:
            event_mask = GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL;
            break;
    }

    gpio_init(irq_pin);
    gpio_set_dir(irq_pin, GPIO_IN);

    gpio_set_irq_enabled_with_callback(irq_pin, event_mask, false, rpi_gpio_irq_callback);

    gpio_pin_init_status[irq_pin] = true;
    gpio_irq_mask[irq_pin] = event_mask;
    gpio_irq_enabled[irq_pin] = false;
    
    return hwGPIO_OK;
}

hwGPIO_OpStatus GPIO_Interrupt_DeInit(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin>=hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }
    
    if(gpio_pin_init_status[irq_pin]==false)
    {
        return hwGPIO_OK;
    }
    
    gpio_set_irq_enabled(irq_pin, GPIO_IRQ_LEVEL_LOW | GPIO_IRQ_LEVEL_HIGH | GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, false);

    gpio_deinit(irq_pin);

    gpio_pin_init_status[irq_pin] = false;
    gpio_irq_mask[irq_pin] = 0;
    gpio_irq_enabled[irq_pin] = false;

    return hwGPIO_OK;
}

hwGPIO_OpStatus GPIO_Config_Interrupt_Mode(hwGPIO_Int_Pin irq_pin, hwGPIO_Interrupt_Mode mode)
{
    if(irq_pin>=hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }
  
    if(mode>=hwGPIO_Interrupt_Mode_MAX)
    {
        return hwGPIO_InvalidParameter;
    }
    
    uint32_t event_mask = 0;

    switch (mode) {
        case hwGPIO_Interrupt_Mode_Rising_Edge:
            event_mask = GPIO_IRQ_EDGE_RISE;
            break;

        case hwGPIO_Interrupt_Mode_Falling_Edge:
            event_mask = GPIO_IRQ_EDGE_FALL;
            break;

        case hwGPIO_Interrupt_Mode_Both_Edge:
            event_mask = GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL;
            break;
    }

    if(gpio_irq_enabled[irq_pin])
    {
        gpio_set_irq_enabled(irq_pin, GPIO_IRQ_LEVEL_LOW | GPIO_IRQ_LEVEL_HIGH | GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, false);

        gpio_set_irq_enabled(irq_pin, event_mask, true);
    }

    gpio_irq_mask[irq_pin] = event_mask;

    return hwGPIO_OK;
}

hwGPIO_OpStatus GPIO_Register_Interrupt_Handler(hwGPIO_Int_Pin irq_pin, GPIO_Interrupt_Event_Handler handler)
{
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

hwGPIO_OpStatus GPIO_Unregister_Interrupt_Handler(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin>=hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }
  
    gpio_irq_handlers[irq_pin] = NULL;

    return hwGPIO_OK;
}

hwGPIO_OpStatus GPIO_Interrupt_Enable(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin>=hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }
  
    if (gpio_irq_mask[irq_pin] == 0)
    {
        return hwGPIO_InvalidParameter;
    }
    
    gpio_set_irq_enabled(irq_pin, gpio_irq_mask[irq_pin], true);

    gpio_irq_enabled[irq_pin] = true;

    return hwGPIO_OK;
}

hwGPIO_OpStatus GPIO_Interrupt_Disable(hwGPIO_Int_Pin irq_pin)
{
    if(irq_pin>=hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }
  
    if (gpio_irq_mask[irq_pin] == 0)
    {
        return hwGPIO_InvalidParameter;
    }
    
    gpio_set_irq_enabled(irq_pin, gpio_irq_mask[irq_pin], false);

    gpio_irq_enabled[irq_pin] = false;

    return hwGPIO_OK;
}

hwGPIO_OpStatus GPIO_Interrupt_Pin_Read(hwGPIO_Int_Pin pin, bool* level)
{
    if(pin>=hwGPIO_Int_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(level==NULL)
    {
      return hwGPIO_InvalidParameter;
    }
  
    *level = gpio_get((uint)pin);
    
    return hwGPIO_OK;
}

#endif