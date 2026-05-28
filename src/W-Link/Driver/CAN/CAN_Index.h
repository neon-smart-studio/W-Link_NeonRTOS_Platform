
#ifndef CAN_INDEX_H
#define CAN_INDEX_H

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

#endif //CAN_INDEX_H