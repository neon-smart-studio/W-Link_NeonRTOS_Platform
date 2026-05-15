
#ifndef GPIO_DEF_H
#define GPIO_DEF_H

typedef enum hwGPIO_OpResult_t
{
  hwGPIO_OK = 0,
  hwGPIO_InvalidParameter = -1,
  hwGPIO_PinConflict = -2,
  hwGPIO_HW_Error = -3,
  hwGPIO_Unsupport = -4
} hwGPIO_OpResult;

typedef enum hwGPIO_Direction_t
{
  hwGPIO_Direction_Input = 0,
  hwGPIO_Direction_Output = 1,
  hwGPIO_Direction_Output_Only = 2,
  hwGPIO_Direction_MAX = 3
} hwGPIO_Direction;

typedef enum hwGPIO_Pull_Mode_t
{
  hwGPIO_Pull_Mode_None = 0,
  hwGPIO_Pull_Mode_Up = 1,
  hwGPIO_Pull_Mode_Down = 2,
  hwGPIO_Pull_Mode_OpenDrain = 3,
  hwGPIO_Pull_Mode_MAX = 4
} hwGPIO_Pull_Mode;

typedef enum hwGPIO_Interrupt_Mode_t
{
  hwGPIO_Interrupt_Mode_Falling_Edge = 0,
  hwGPIO_Interrupt_Mode_Rising_Edge = 1,
  hwGPIO_Interrupt_Mode_Both_Edge = 2,
  hwGPIO_Interrupt_Mode_MAX = 3
} hwGPIO_Interrupt_Mode;

typedef enum hwGPIO_Interrupt_Action_t
{
  hwGPIO_Interrupt_Action_Disbled = 0,
  hwGPIO_Interrupt_Action_Falling_Edge = 1,
  hwGPIO_Interrupt_Action_Rising_Edge = 2,
  hwGPIO_Interrupt_Action_Toggle = 3,
  hwGPIO_Interrupt_Action_MAX = 4
} hwGPIO_Interrupt_Action;

#endif //GPIO_DEF_H