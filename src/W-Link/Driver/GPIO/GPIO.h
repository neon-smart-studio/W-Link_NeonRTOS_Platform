
#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>

#include "soc.h"

#include "Pin/GPIO_Pin.h"

#include "GPIO_Def.h"

bool GPIO_Pin_is_Init(hwGPIO_Pin pin);
hwGPIO_OpStatus GPIO_Pin_Init(hwGPIO_Pin pin, hwGPIO_Direction dir, hwGPIO_Pull_Mode pull_mode);
hwGPIO_OpStatus GPIO_Pin_DeInit(hwGPIO_Pin pin);
hwGPIO_OpStatus GPIO_Pin_Set_Direction(hwGPIO_Pin pin, hwGPIO_Direction dir);
hwGPIO_OpStatus GPIO_Pin_Get_Direction(hwGPIO_Pin pin, hwGPIO_Direction* dir);
hwGPIO_OpStatus GPIO_Pin_Set_PullMode(hwGPIO_Pin pin, hwGPIO_Pull_Mode pull_mode);
hwGPIO_OpStatus GPIO_Pin_Get_PullMode(hwGPIO_Pin pin, hwGPIO_Pull_Mode* pull_mode);
hwGPIO_OpStatus GPIO_Pin_Read(hwGPIO_Pin pin, bool* level);
hwGPIO_OpStatus GPIO_Pin_Write(hwGPIO_Pin pin, bool level);
hwGPIO_OpStatus GPIO_Pin_Toggle(hwGPIO_Pin pin);

typedef void (*GPIO_Interrupt_Event_Handler)(hwGPIO_Int_Pin pin, hwGPIO_Interrupt_Action action);

hwGPIO_OpStatus GPIO_Interrupt_Init(hwGPIO_Int_Pin irq_pin, hwGPIO_Interrupt_Mode mode);
hwGPIO_OpStatus GPIO_Interrupt_DeInit(hwGPIO_Int_Pin irq_pin);
hwGPIO_OpStatus GPIO_Config_Interrupt_Mode(hwGPIO_Int_Pin irq_pin, hwGPIO_Interrupt_Mode mode);
hwGPIO_OpStatus GPIO_Register_Interrupt_Handler(hwGPIO_Int_Pin pin, GPIO_Interrupt_Event_Handler handler);
hwGPIO_OpStatus GPIO_Unregister_Interrupt_Handler(hwGPIO_Int_Pin pin);
bool GPIO_Interrupt_Is_Enabled(hwGPIO_Int_Pin irq_pin);
hwGPIO_OpStatus GPIO_Interrupt_Enable(hwGPIO_Int_Pin pin);
hwGPIO_OpStatus GPIO_Interrupt_Disable(hwGPIO_Int_Pin pin);
hwGPIO_OpStatus GPIO_Interrupt_Pin_Read(hwGPIO_Int_Pin pin, bool* level);

#endif //GPIO_H