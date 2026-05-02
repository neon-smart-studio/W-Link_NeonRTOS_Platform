#ifndef CAN_PIN_STM32F3_H
#define CAN_PIN_STM32F3_H

#include "CAN_Pin_STM32_Def.h"

typedef enum {
    CAN_Pinset_DEFAULT = 0,
    CAN_Pinset_ALT1,
    CAN_Pinset_ALT2,
    CAN_Pinset_MAX
} CAN_Pinset_t;

#ifndef CONFIG_CAN0_PINSET
#define CONFIG_CAN0_PINSET CAN_Pinset_DEFAULT
#endif

static const CAN_Pinset_t CAN_Index_Map_Alt[hwCAN_Index_MAX] = {
#if defined(CAN1_BASE)
    CONFIG_CAN0_PINSET,
#endif
};

static const CAN_Pin_Def CAN_Pin_Def_Table[hwCAN_Index_MAX][CAN_Pinset_MAX] =
{
#if defined(CAN1_BASE)
    /* ================= CAN1 / CAN0 ================= */
    {
        /* DEFAULT: PA12 TX / PA11 RX */
        { hwGPIO_Pin_A12, hwGPIO_Pin_A11 },

        /* ALT1: PB9 TX / PB8 RX */
        { hwGPIO_Pin_B9,  hwGPIO_Pin_B8  },

        /* ALT2: PD1 TX / PD0 RX, not every F3 package has PD0/PD1 */
        { hwGPIO_Pin_D1,  hwGPIO_Pin_D0  },
    },
#endif
};

static const CAN_AF_Map CAN_Pin_AF_Map[] = {
#if defined(CAN1_BASE)
    /* CAN1 DEFAULT */
    { hwCAN_Index_0, hwGPIO_Pin_A12, GPIO_AF9_CAN1 },
    { hwCAN_Index_0, hwGPIO_Pin_A11, GPIO_AF9_CAN1 },

    /* CAN1 ALT1 */
    { hwCAN_Index_0, hwGPIO_Pin_B9,  GPIO_AF9_CAN1 },
    { hwCAN_Index_0, hwGPIO_Pin_B8,  GPIO_AF9_CAN1 },

    /* CAN1 ALT2 */
    { hwCAN_Index_0, hwGPIO_Pin_D1,  GPIO_AF9_CAN1 },
    { hwCAN_Index_0, hwGPIO_Pin_D0,  GPIO_AF9_CAN1 },
#endif
};

#endif // CAN_PIN_STM32F3_H