#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"

#ifdef DEVICE_TIMSP432E

bool gpio_pin_init_status[hwGPIO_Pin_MAX] = {false};

static hwGPIO_Direction gpio_current_dir[hwGPIO_Int_Pin_MAX] = {hwGPIO_Direction_Input};
static hwGPIO_Pull_Mode gpio_current_mode[hwGPIO_Int_Pin_MAX] = {hwGPIO_Pull_Mode_None};
static hwGPIO_Interrupt_Mode gpio_current_irq_mode[hwGPIO_Int_Pin_MAX] = {hwGPIO_Interrupt_Mode_MAX};
static GPIO_Interrupt_Event_Handler gpio_irq_handlers[hwGPIO_Int_Pin_MAX];

static void GPIO_Enable_Port_Clock(uint32_t base)
{
  switch(base)
  {
    case GPIO_PORTA_BASE:
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
      while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
      break;
    case GPIO_PORTB_BASE:
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
      while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
      break;
    case GPIO_PORTC_BASE:
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
      while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC));
      break;
    case GPIO_PORTD_BASE:
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
      while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
      break;
    case GPIO_PORTE_BASE:
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
      while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
      break;
    case GPIO_PORTF_BASE:
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
      while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
      break;
    case GPIO_PORTG_BASE:
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
      while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG));
      break;
    case GPIO_PORTH_BASE:
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
      while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH));
      break;
    case GPIO_PORTK_BASE:
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
      while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));
      break;
    case GPIO_PORTL_BASE:
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
      while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));
      break;
    case GPIO_PORTM_BASE:
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
      while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));
      break;
    case GPIO_PORTN_BASE:
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
      while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));
      break;
    case GPIO_PORTP_BASE:
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
      while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP));
      break;
    case GPIO_PORTQ_BASE:
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
      while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOQ));
      break;
  }
}

static void GPIO_Disable_Port_Clock(uint32_t base)
{
  switch(base)
  {
    case GPIO_PORTA_BASE:
      MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOA);
      break;
    case GPIO_PORTB_BASE:
      MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOB);
      break;
    case GPIO_PORTC_BASE:
      MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOC);
      break;
    case GPIO_PORTD_BASE:
      MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOD);
      break;
    case GPIO_PORTE_BASE:
      MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOE);
      break;
    case GPIO_PORTF_BASE:
      MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOF);
      break;
    case GPIO_PORTG_BASE:
      MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOG);
      break;
    case GPIO_PORTH_BASE:
      MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOH);
      break;
    case GPIO_PORTK_BASE:
      MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOK);
      break;
    case GPIO_PORTL_BASE:
      MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOL);
      break;
    case GPIO_PORTM_BASE:
      MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOM);
      break;
    case GPIO_PORTN_BASE:
      MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_GPION);
      break;
    case GPIO_PORTP_BASE:
      MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOP);
      break;
    case GPIO_PORTQ_BASE:
      MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOQ);
      break;
  }
}

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
    
    uint32_t portBase =  GPIO_Map_Soc_Port_Base(pin);
    uint32_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    GPIO_Enable_Port_Clock(portBase);

    switch(pull_mode)
    {
        case hwGPIO_Pull_Mode_None:
            MAP_GPIOPadConfigSet(portBase, pinMask, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
            break;
        case hwGPIO_Pull_Mode_Up:
            MAP_GPIOPadConfigSet(portBase, pinMask, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
            break;
        case hwGPIO_Pull_Mode_Down:
            MAP_GPIOPadConfigSet(portBase, pinMask, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
            break;
        case hwGPIO_Pull_Mode_OpenDrain:
            MAP_GPIOPadConfigSet(portBase, pinMask, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);
            break;
    }
    
    switch(dir)
    {
        case hwGPIO_Direction_Input:
            MAP_GPIODirModeSet(portBase, pinMask, GPIO_DIR_MODE_IN);
            break;
        case hwGPIO_Direction_Output:
        case hwGPIO_Direction_Output_Only:
            MAP_GPIODirModeSet(portBase, pinMask, GPIO_DIR_MODE_OUT);
            break;
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
    
    uint32_t portBase =  GPIO_Map_Soc_Port_Base(pin);
    uint32_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    MAP_GPIOPinTypeGPIOInput(portBase, pinMask);
    
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

    uint32_t portBase =  GPIO_Map_Soc_Port_Base(pin);
    uint32_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    switch(dir)
    {
        case hwGPIO_Direction_Input:
            MAP_GPIODirModeSet(portBase, pinMask, GPIO_DIR_MODE_IN);
            break;
        case hwGPIO_Direction_Output:
        case hwGPIO_Direction_Output_Only:
            MAP_GPIODirModeSet(portBase, pinMask, GPIO_DIR_MODE_OUT);
            break;
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
  
    uint32_t portBase =  GPIO_Map_Soc_Port_Base(pin);
    uint32_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }
    
    switch(pull_mode)
    {
        case hwGPIO_Pull_Mode_None:
            MAP_GPIOPadConfigSet(portBase, pinMask, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
            break;
        case hwGPIO_Pull_Mode_Up:
            MAP_GPIOPadConfigSet(portBase, pinMask, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
            break;
        case hwGPIO_Pull_Mode_Down:
            MAP_GPIOPadConfigSet(portBase, pinMask, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
            break;
        case hwGPIO_Pull_Mode_OpenDrain:
            MAP_GPIOPadConfigSet(portBase, pinMask, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);
            break;
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

    uint32_t portBase =  GPIO_Map_Soc_Port_Base(pin);
    uint32_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    *level = (MAP_GPIOPinRead(portBase, pinMask)) ? 1 : 0;
    
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

    uint32_t portBase =  GPIO_Map_Soc_Port_Base(pin);
    uint32_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    if(level)
    {
      MAP_GPIOPinWrite(portBase, pinMask, pinMask);
    }
    else
    {
      MAP_GPIOPinWrite(portBase, pinMask, 0);
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

    uint32_t portBase =  GPIO_Map_Soc_Port_Base(pin);
    uint32_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    int read_mask = MAP_GPIOPinRead(portBase, pinMask);
    
    if(read_mask & pinMask)
    {
        MAP_GPIOPinWrite(portBase, pinMask, 0);
    }
    else
    {
        MAP_GPIOPinWrite(portBase, pinMask, pinMask);
    }
    
    return hwGPIO_OK;
}

static void GPIO_IRQ_Handler(uint32_t portBase)
{
    hwGPIO_Interrupt_Action action;

    uint32_t flags = GPIOIntStatus(portBase, true);
    GPIOIntClear(portBase, flags);

    hwGPIO_Int_Pin intPin = GPIO_Map_Int_Pin_By_Mask(portBase, flags);

    switch(gpio_current_irq_mode[intPin])
    {
        case hwGPIO_Interrupt_Mode_Rising_Edge:
            action = hwGPIO_Interrupt_Action_Rising_Edge;
            break;
        case hwGPIO_Interrupt_Mode_Falling_Edge:
            action = hwGPIO_Interrupt_Action_Falling_Edge;
            break;
        case hwGPIO_Interrupt_Mode_Both_Edge:
            action = hwGPIO_Interrupt_Action_Toggle;
            break;
        default:
            return;
    }
    if(gpio_irq_handlers[intPin]!=NULL)
    {
        gpio_irq_handlers[intPin](intPin, action);
    }
}

static void GPIOA_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTA_BASE); }
static void GPIOB_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTB_BASE); }
static void GPIOC_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTC_BASE); }
static void GPIOD_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTD_BASE); }
static void GPIOE_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTE_BASE); }
static void GPIOF_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTF_BASE); }
static void GPIOG_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTG_BASE); }
static void GPIOH_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTH_BASE); }
static void GPIOJ_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTJ_BASE); }
static void GPIOK_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTK_BASE); }
static void GPIOL_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTL_BASE); }
static void GPIOM_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTM_BASE); }
static void GPION_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTN_BASE); }
static void GPIOP_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTP_BASE); }
static void GPIOQ_IRQ_Handler() { GPIO_IRQ_Handler(GPIO_PORTQ_BASE); }

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
    
    uint32_t portBase =  GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint32_t pinMask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);
    uint32_t intMask = GPIO_Map_Pin_Int_Mask(pinMask);

    if(portBase==0 || pinMask==0 || intMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    GPIO_Enable_Port_Clock(portBase);

    MAP_GPIOPinTypeGPIOInput(portBase, pinMask);
    
    switch(mode)
    {
        case hwGPIO_Interrupt_Mode_Falling_Edge:
            MAP_GPIOIntTypeSet(portBase, pinMask, GPIO_FALLING_EDGE);
            break;
        case hwGPIO_Interrupt_Mode_Rising_Edge:
            MAP_GPIOIntTypeSet(portBase, pinMask, GPIO_RISING_EDGE);
            break;
        case hwGPIO_Interrupt_Mode_Both_Edge:
            MAP_GPIOIntTypeSet(portBase, pinMask, GPIO_BOTH_EDGES);
            break;
        default:
            return hwGPIO_InvalidParameter;
    }
    
    switch(portBase)
    {
        case GPIO_PORTA_BASE:
          MAP_GPIOIntRegister(portBase, GPIOA_IRQ_Handler);
          break;
        case GPIO_PORTB_BASE:
          MAP_GPIOIntRegister(portBase, GPIOB_IRQ_Handler);
          break;
        case GPIO_PORTC_BASE:
          MAP_GPIOIntRegister(portBase, GPIOC_IRQ_Handler);
          break;
        case GPIO_PORTD_BASE:
          MAP_GPIOIntRegister(portBase, GPIOD_IRQ_Handler);
          break;
        case GPIO_PORTE_BASE:
          MAP_GPIOIntRegister(portBase, GPIOE_IRQ_Handler);
          break;
        case GPIO_PORTF_BASE:
          MAP_GPIOIntRegister(portBase, GPIOF_IRQ_Handler);
          break;
        case GPIO_PORTG_BASE:
          MAP_GPIOIntRegister(portBase, GPIOG_IRQ_Handler);
          break;
        case GPIO_PORTH_BASE:
          MAP_GPIOIntRegister(portBase, GPIOH_IRQ_Handler);
          break;
        case GPIO_PORTJ_BASE:
          MAP_GPIOIntRegister(portBase, GPIOJ_IRQ_Handler);
          break;
        case GPIO_PORTK_BASE:
          MAP_GPIOIntRegister(portBase, GPIOK_IRQ_Handler);
          break;
        case GPIO_PORTL_BASE:
          MAP_GPIOIntRegister(portBase, GPIOL_IRQ_Handler);
          break;
        case GPIO_PORTM_BASE:
          MAP_GPIOIntRegister(portBase, GPIOM_IRQ_Handler);
          break;
        case GPIO_PORTN_BASE:
          MAP_GPIOIntRegister(portBase, GPION_IRQ_Handler);
          break;
        case GPIO_PORTP_BASE:
          MAP_GPIOIntRegister(portBase, GPIOP_IRQ_Handler);
          break;
        case GPIO_PORTQ_BASE:
          MAP_GPIOIntRegister(portBase, GPIOQ_IRQ_Handler);
          break;
    }

    MAP_GPIOIntEnable(portBase, intMask);

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
    
    uint32_t portBase =  GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint32_t pinMask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);
    uint32_t intMask = GPIO_Map_Pin_Int_Mask(pinMask);

    if(portBase==0 || pinMask==0 || intMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    MAP_GPIOIntDisable(portBase, intMask);

    MAP_GPIOIntUnregister(portBase);

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
  
    uint32_t portBase =  GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint32_t pinMask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);
    uint32_t intMask = GPIO_Map_Pin_Int_Mask(pinMask);

    if(portBase==0 || pinMask==0 || intMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    switch(mode)
    {
        case hwGPIO_Interrupt_Mode_Falling_Edge:
            MAP_GPIOIntTypeSet(portBase, pinMask, GPIO_FALLING_EDGE);
            break;
        case hwGPIO_Interrupt_Mode_Rising_Edge:
            MAP_GPIOIntTypeSet(portBase, pinMask, GPIO_RISING_EDGE);
            break;
        case hwGPIO_Interrupt_Mode_Both_Edge:
            MAP_GPIOIntTypeSet(portBase, pinMask, GPIO_BOTH_EDGES);
            break;
        default:
            return hwGPIO_InvalidParameter;
    }
    
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
  
    uint32_t portBase =  GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint32_t pinMask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);
    uint32_t intMask = GPIO_Map_Pin_Int_Mask(pinMask);

    if(portBase==0 || pinMask==0 || intMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    MAP_GPIOIntEnable(portBase, intMask);
    
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
  
    uint32_t portBase =  GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint32_t pinMask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);
    uint32_t intMask = GPIO_Map_Pin_Int_Mask(pinMask);

    if(portBase==0 || pinMask==0 || intMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    MAP_GPIOIntDisable(portBase, intMask);
    
    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Interrupt_Pin_Read(hwGPIO_Int_Pin irq_pin, bool* level)
{
    if(irq_pin==hwGPIO_Int_Pin_NC)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(irq_pin>=hwGPIO_Int_Pin_MAX)
    {
      return hwGPIO_InvalidParameter;
    }
    
    if(level==NULL)
    {
      return hwGPIO_InvalidParameter;
    }
  
    uint32_t portBase =  GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint32_t pinMask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);
    uint32_t intMask = GPIO_Map_Pin_Int_Mask(pinMask);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    *level = (MAP_GPIOPinRead(portBase, pinMask)) ? 1 : 0;
    
    return hwGPIO_OK;
}

#endif //DEVICE_TIMSP432E
