
#ifndef DMA_DEF_H
#define DMA_DEF_H

typedef enum hwDMA_Peripheral_Direction_t
{
  hwDMA_Peripheral_Direction_RX = 0,
  hwDMA_Peripheral_Direction_TX,
  hwDMA_Peripheral_Direction_MAX
}hwDMA_Peripheral_Direction;

typedef enum hwDMA_OpMode_t
{
  hwDMA_Mode_NotInit = -1,
  hwDMA_Mode_Idle = 0,
  hwDMA_Mode_OneShout = 1,
  hwDMA_Mode_Period = 2,
}hwDMA_OpMode_t;

typedef enum hwDMA_OpResult_t
{
  hwDMA_OK = 0,
  hwDMA_NotInit = -1,
  hwDMA_InvalidParameter = -2,
  hwDMA_HwError = -4,
  hwDMA_MemoryError = -5,
  hwDMA_MutexTimeout = -6,
  hwDMA_XferTimeout = -7,
  hwDMA_Unsupport = -8,
}hwDMA_OpResult;

#endif //DMA_DEF_H