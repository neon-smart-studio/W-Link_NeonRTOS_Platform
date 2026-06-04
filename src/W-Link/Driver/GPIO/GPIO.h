
#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>

#include "soc.h"

#include "Pin/GPIO_Pin.h"

#include "GPIO_Def.h"

#include "Driver_Config.h"

#ifdef	__cplusplus
extern "C" {
#endif

bool GPIO_Pin_is_Init(hwGPIO_Pin pin);
hwGPIO_OpResult GPIO_Pin_Init(hwGPIO_Pin pin, hwGPIO_Direction dir, hwGPIO_Pull_Mode pull_mode);
hwGPIO_OpResult GPIO_Pin_DeInit(hwGPIO_Pin pin);
hwGPIO_OpResult GPIO_Pin_Set_Direction(hwGPIO_Pin pin, hwGPIO_Direction dir);
hwGPIO_OpResult GPIO_Pin_Get_Direction(hwGPIO_Pin pin, hwGPIO_Direction* dir);
hwGPIO_OpResult GPIO_Pin_Set_PullMode(hwGPIO_Pin pin, hwGPIO_Pull_Mode pull_mode);
hwGPIO_OpResult GPIO_Pin_Get_PullMode(hwGPIO_Pin pin, hwGPIO_Pull_Mode* pull_mode);
hwGPIO_OpResult GPIO_Pin_Read(hwGPIO_Pin pin, bool* level);
hwGPIO_OpResult GPIO_Pin_Write(hwGPIO_Pin pin, bool level);
hwGPIO_OpResult GPIO_Pin_Toggle(hwGPIO_Pin pin);

typedef void (*GPIO_Interrupt_Event_Handler)(hwGPIO_Int_Pin pin, hwGPIO_Interrupt_Action action);

hwGPIO_OpResult GPIO_Interrupt_Init(hwGPIO_Int_Pin irq_pin, hwGPIO_Interrupt_Mode mode);
hwGPIO_OpResult GPIO_Interrupt_DeInit(hwGPIO_Int_Pin irq_pin);
hwGPIO_OpResult GPIO_Config_Interrupt_Mode(hwGPIO_Int_Pin irq_pin, hwGPIO_Interrupt_Mode mode);
hwGPIO_OpResult GPIO_Register_Interrupt_Handler(hwGPIO_Int_Pin pin, GPIO_Interrupt_Event_Handler handler);
hwGPIO_OpResult GPIO_Unregister_Interrupt_Handler(hwGPIO_Int_Pin pin);
bool GPIO_Interrupt_Is_Enabled(hwGPIO_Int_Pin irq_pin);
hwGPIO_OpResult GPIO_Interrupt_Enable(hwGPIO_Int_Pin pin);
hwGPIO_OpResult GPIO_Interrupt_Disable(hwGPIO_Int_Pin pin);
hwGPIO_OpResult GPIO_Interrupt_Pin_Read(hwGPIO_Int_Pin pin, bool* level);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //GPIO_H