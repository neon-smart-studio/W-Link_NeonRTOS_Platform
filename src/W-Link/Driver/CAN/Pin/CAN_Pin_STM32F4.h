#ifndef CAN_PIN_STM32F4_H
#define CAN_PIN_STM32F4_H

#include "CAN_Pin_STM32.h"

typedef enum {
    CAN_Pinset_DEFAULT = 0,
    CAN_Pinset_ALT1,
    CAN_Pinset_ALT2,
    CAN_Pinset_MAX
} CAN_Pinset_t;

#ifndef CONFIG_CAN0_PINSET
#define CONFIG_CAN0_PINSET CAN_Pinset_DEFAULT
#endif

#ifndef CONFIG_CAN1_PINSET
#define CONFIG_CAN1_PINSET CAN_Pinset_DEFAULT
#endif

static const CAN_Pinset_t CAN_Index_Map_Alt[hwCAN_Index_MAX] = {
#if defined(CAN1_BASE)
    CONFIG_CAN0_PINSET,
#endif
#if defined(CAN2_BASE)
    CONFIG_CAN1_PINSET
#endif
};

static const CAN_Pin_Def CAN_Pin_Def_Table[hwCAN_Index_MAX][CAN_Pinset_MAX] =
{
#if defined(CAN1_BASE)
    /* ================= CAN1 (CAN0) ================= */
    {
        { hwGPIO_Pin_A12, hwGPIO_Pin_A11 }, // DEFAULT: TX, RX
        { hwGPIO_Pin_B9,  hwGPIO_Pin_B8  }, // ALT1
        { hwGPIO_Pin_D1,  hwGPIO_Pin_D0  }, // ALT2
    },
#endif

#if defined(CAN2_BASE)
    /* ================= CAN2 (CAN1) ================= */
    {
        { hwGPIO_Pin_B6,  hwGPIO_Pin_B5  }, // DEFAULT: TX, RX
        { hwGPIO_Pin_B13, hwGPIO_Pin_B12 }, // ALT1
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC  }, // ALT2
    },
#endif
};

static static const CAN_AF_Map CAN_Pin_AF_Map[] = {
#if defined(CAN1_BASE)
    { hwCAN_Index_0, hwGPIO_Pin_A12, GPIO_AF9_CAN1 },
    { hwCAN_Index_0, hwGPIO_Pin_A11, GPIO_AF9_CAN1 },
    { hwCAN_Index_0, hwGPIO_Pin_B9,  GPIO_AF9_CAN1 },
    { hwCAN_Index_0, hwGPIO_Pin_B8,  GPIO_AF9_CAN1 },
    { hwCAN_Index_0, hwGPIO_Pin_D1,  GPIO_AF9_CAN1 },
    { hwCAN_Index_0, hwGPIO_Pin_D0,  GPIO_AF9_CAN1 },
#endif

#if defined(CAN2_BASE)
    { hwCAN_Index_1, hwGPIO_Pin_B6,  GPIO_AF9_CAN2 },
    { hwCAN_Index_1, hwGPIO_Pin_B5,  GPIO_AF9_CAN2 },
    { hwCAN_Index_1, hwGPIO_Pin_B13, GPIO_AF9_CAN2 },
    { hwCAN_Index_1, hwGPIO_Pin_B12, GPIO_AF9_CAN2 },
#endif
};

#endif // CAN_PIN_STM32F4_H