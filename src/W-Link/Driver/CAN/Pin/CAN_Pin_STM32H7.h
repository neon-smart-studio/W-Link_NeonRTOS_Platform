
#ifndef CAN_PIN_STM32H7_H
#define CAN_PIN_STM32H7_H

#include "CAN_Pin_STM32.h"

typedef enum {
    CAN_Pinset_DEFAULT = 0,
    CAN_Pinset_ALT1,
    CAN_Pinset_ALT2,
    CAN_Pinset_MAX
} CAN_Pinset_t;

#ifndef CONFIG_CAN0_PINSET
#define CONFIG_CAN0_PINSET CAN_Pinset_DEFAULT
#endif //CONFIG_CAN0_PINSET

#ifndef CONFIG_CAN1_PINSET
#define CONFIG_CAN1_PINSET CAN_Pinset_DEFAULT
#endif //CONFIG_CAN1_PINSET

#ifndef CONFIG_CAN2_PINSET
#define CONFIG_CAN2_PINSET CAN_Pinset_DEFAULT
#endif //CONFIG_CAN2_PINSET

static const CAN_Pinset_t CAN_Index_Map_Alt[hwCAN_Index_MAX] = {
#if defined (CAN1_BASE)
    CONFIG_CAN0_PINSET,
#endif
#if defined (CAN2_BASE)
    CONFIG_CAN1_PINSET
#endif
#if defined (CAN3_BASE)
    CONFIG_CAN2_PINSET
#endif
};

static const CAN_Pin_Def CAN_Pin_Def_Table[hwCAN_Index_MAX][CAN_Pinset_MAX] =
{
#if defined (CAN1_BASE)
    /* ================= CAN1 (CAN0) ================= */
    {
        /* DEFAULT */
        { hwGPIO_Pin_A12, hwGPIO_Pin_A11 },

        /* ALT */
        { hwGPIO_Pin_B9, hwGPIO_Pin_B8 },

        /* ALT */
        { hwGPIO_Pin_D1, hwGPIO_Pin_D0 },
    },
#endif
#if defined (CAN2_BASE)
    /* ================= CAN2 (CAN1) ================= */
    {
        /* DEFAULT */
        { hwGPIO_Pin_B6,  hwGPIO_Pin_B5 },

        /* ALT */
        { hwGPIO_Pin_B13, hwGPIO_Pin_B12 },

        /* ALT */
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC },
    },
#endif
#if defined (CAN3_BASE)
    /* ================= CAN3 (CAN2) ================= */
    {
        /* DEFAULT */
        { hwGPIO_Pin_D13, hwGPIO_Pin_D12 },

        /* ALT */
        { hwGPIO_Pin_F7,  hwGPIO_Pin_F6  },

        /* ALT */
        { hwGPIO_Pin_G9,  hwGPIO_Pin_G10 },
    },
#endif
};

static static const CAN_AF_Map CAN_Pin_AF_Map[] = {
#if defined (CAN1_BASE)
    // CAN1
    { hwCAN_Index_0, hwGPIO_Pin_A12, GPIO_AF9_FDCAN1 },
    { hwCAN_Index_0, hwGPIO_Pin_A11, GPIO_AF9_FDCAN1 },
    { hwCAN_Index_0, hwGPIO_Pin_B8,  GPIO_AF9_FDCAN1 },
    { hwCAN_Index_0, hwGPIO_Pin_B9,  GPIO_AF9_FDCAN1 },
    { hwCAN_Index_0, hwGPIO_Pin_D1,  GPIO_AF9_FDCAN1 },
    { hwCAN_Index_0, hwGPIO_Pin_D0,  GPIO_AF9_FDCAN1 },
#endif
#if defined (CAN2_BASE)
    // CAN2
    { hwCAN_Index_1, hwGPIO_Pin_B6,  GPIO_AF9_FDCAN2 },
    { hwCAN_Index_1, hwGPIO_Pin_B5,  GPIO_AF9_FDCAN2 },
    { hwCAN_Index_1, hwGPIO_Pin_B13, GPIO_AF9_FDCAN2 },
    { hwCAN_Index_1, hwGPIO_Pin_B12, GPIO_AF9_FDCAN2 },
#endif
#if defined (CAN3_BASE)
    // CAN3
    { hwCAN_Index_2, hwGPIO_Pin_D13, GPIO_AF5_FDCAN3 },
    { hwCAN_Index_2, hwGPIO_Pin_D12, GPIO_AF5_FDCAN3 },
    { hwCAN_Index_2, hwGPIO_Pin_F7,  GPIO_AF2_FDCAN3 },
    { hwCAN_Index_2, hwGPIO_Pin_F6,  GPIO_AF2_FDCAN3 },
    { hwCAN_Index_2, hwGPIO_Pin_G9,  GPIO_AF2_FDCAN3 },
    { hwCAN_Index_2, hwGPIO_Pin_G10, GPIO_AF2_FDCAN3 },
#endif
};

#endif //CAN_PIN_STM32H7_H