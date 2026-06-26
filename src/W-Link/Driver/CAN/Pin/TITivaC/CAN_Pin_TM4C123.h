#ifndef CAN_PIN_TM4C123_H
#define CAN_PIN_TM4C123_H

#include "CAN_Pin_TITivaC_Def.h"

static const CAN_Pin_Def CAN_Pin_Def_Table[hwCAN_Index_MAX] =
{
#if defined(CAN0_BASE)
    /* ================= CAN0 ================= */
        { hwGPIO_Pin_E5, hwGPIO_Pin_E4 },   // TX, RX
#endif

#if defined(CAN1_BASE)
    /* ================= CAN1 ================= */
        { hwGPIO_Pin_B1, hwGPIO_Pin_B0 },   // TX, RX
#endif
};

#endif // CAN_PIN_TM4C123_H