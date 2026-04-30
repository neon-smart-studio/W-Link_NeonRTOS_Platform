
#ifndef CAN_PIN_STM32F1_H
#define CAN_PIN_STM32F1_H

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

static const CAN_Pinset_t CAN_Index_Map_Alt[hwCAN_Index_MAX] = {
#if defined (CAN1_BASE)
    CONFIG_CAN0_PINSET,
#endif
#if defined (CAN2_BASE)
    CONFIG_CAN1_PINSET
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
};

#endif //CAN_PIN_STM32F1_H