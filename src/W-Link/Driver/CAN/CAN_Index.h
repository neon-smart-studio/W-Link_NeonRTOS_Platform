
#ifndef CAN_INDEX_H
#define CAN_INDEX_H

#include "soc.h"

#ifdef DEVICE_NUVOTON
typedef enum {
#if defined (CAN0_BASE)
    hwCAN_Index_0 = 0,
#endif
#if defined (CAN1_BASE)
    hwCAN_Index_1,
#endif
    hwCAN_Index_MAX
} hwCAN_Index;
#endif // DEVICE_NUVOTON

#ifdef DEVICE_STM32
typedef enum {
#if defined (CAN1_BASE)
    hwCAN_Index_0 = 0,
#endif
#if defined (CAN2_BASE)
    hwCAN_Index_1,
#endif
#if defined (CAN3_BASE)
    hwCAN_Index_2,
#endif
    hwCAN_Index_MAX
} hwCAN_Index;
#endif // DEVICE_STM32

#ifdef DEVICE_TITIVAC
typedef enum {
#if defined (CAN0_BASE)
    hwCAN_Index_0 = 0,
#endif
#if defined (CAN1_BASE)
    hwCAN_Index_1,
#endif
    hwCAN_Index_MAX
} hwCAN_Index;
#endif // DEVICE_TITIVAC

#ifdef DEVICE_TIMSP432P
typedef enum {
    hwCAN_Index_MAX
} hwCAN_Index;
#endif // DEVICE_TIMSP432P

#ifdef DEVICE_TIMSP432E
typedef enum {
#if defined (CAN0_BASE)
    hwCAN_Index_0 = 0,
#endif
#if defined (CAN1_BASE)
    hwCAN_Index_1,
#endif
    hwCAN_Index_MAX
} hwCAN_Index;
#endif // DEVICE_TIMSP432E

#endif //CAN_INDEX_H