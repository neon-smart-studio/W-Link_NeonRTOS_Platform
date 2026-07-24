#ifndef CAN_PIN_TIMSPM0_H
#define CAN_PIN_TIMSPM0_H

#include "CAN_Pin_TIMSPM0_Def.h"

#if defined(CANFD0_BASE)
    /* ================= CAN0 ================= */
    {
        hwGPIO_Pin_A26,   /* TX: PA26 / PINCM59 */
        hwGPIO_Pin_A27    /* RX: PA27 / PINCM60 */
    },
#endif

#if defined(CANFD1_BASE)
    /* ================= CAN1 ================= */
    {
        hwGPIO_Pin_B21,   /* TX: PB21 / PINCM49 */
        hwGPIO_Pin_B22    /* RX: PB22 / PINCM50 */
    },
#endif

#endif // CAN_PIN_TIMSPM0_H