#ifndef CAN_PIN_STM32F0_H
#define CAN_PIN_STM32F0_H

#include "CAN_Pin_STM32_Def"

typedef enum {
    CAN_Pinset_DEFAULT = 0,
    CAN_Pinset_ALT,
    CAN_Pinset_MAX
} CAN_Pinset_t;

#ifndef CONFIG_CAN0_PINSET
#define CONFIG_CAN0_PINSET CAN_Pinset_DEFAULT
#endif // CONFIG_CAN0_PINSET

static const CAN_Pinset_t CAN_Index_Map_Alt[hwCAN_Index_MAX] = {
#if defined(CAN1_BASE)
    CONFIG_CAN0_PINSET,
#endif
};

static const CAN_Pin_Def CAN_Pin_Def_Table[hwCAN_Index_MAX][CAN_Pinset_MAX] =
{
#if defined(CAN1_BASE)
    /* ================= CAN1 (CAN0) ================= */
    {
        /* DEFAULT: TX, RX */
        { hwGPIO_Pin_A12, hwGPIO_Pin_A11 },

        /* ALT: TX, RX */
        { hwGPIO_Pin_B9,  hwGPIO_Pin_B8  },
    },
#endif
};

#endif // CAN_PIN_STM32F0_H