
#ifndef I2C_PIN_STM32F2_H
#define I2C_PIN_STM32F2_H

#include "I2C_Pin_STM32_Def.h"

typedef enum {
    I2C_Pinset_DEFAULT = 0,
    I2C_Pinset_ALT,
    I2C_Pinset_MAX
} I2C_Pinset_t;

#ifndef CONFIG_I2C0_PINSET
#define CONFIG_I2C0_PINSET I2C_Pinset_DEFAULT
#endif //CONFIG_I2C0_PINSET

#ifndef CONFIG_I2C1_PINSET
#define CONFIG_I2C1_PINSET I2C_Pinset_DEFAULT
#endif //CONFIG_I2C1_PINSET

#ifndef CONFIG_I2C2_PINSET
#define CONFIG_I2C2_PINSET I2C_Pinset_DEFAULT
#endif //CONFIG_I2C2_PINSET

static const I2C_Pinset_t I2C_Index_Map_Alt[hwI2C_Index_MAX] = {
#if defined (I2C1_BASE)
    CONFIG_I2C0_PINSET,
#endif
#if defined (I2C2_BASE)
    CONFIG_I2C1_PINSET,
#endif
#if defined (I2C3_BASE)
    CONFIG_I2C2_PINSET
#endif
};

static const I2C_Pin_Def I2C_Pin_Def_Table[hwI2C_Index_MAX][I2C_Pinset_MAX] =
{
#if defined (I2C1_BASE)
    /* ================= I2C1 (I2C0) ================= */
    {
        { hwGPIO_Pin_B6, hwGPIO_Pin_B7 },  // DEFAULT
        { hwGPIO_Pin_B8, hwGPIO_Pin_B9 },  // ALT
    },
#endif
#if defined (I2C2_BASE)
    /* ================= I2C2 (I2C1) ================= */
    {
        /* DEFAULT */
        { hwGPIO_Pin_B10, hwGPIO_Pin_B11 },

        /* ALT */
        { hwGPIO_Pin_F1,  hwGPIO_Pin_F0  },
    },
#endif
#if defined (I2C3_BASE)
    /* ================= I2C3 (I2C2) ================= */
    {
        /* DEFAULT */
        { hwGPIO_Pin_A8, hwGPIO_Pin_C9 },

        /* ALT */
        { hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif
};

static const I2C_AF_Map I2C_Pin_AF_Map[] =
{
#if defined (I2C1_BASE)
    /* I2C1 */
    { hwI2C_Index_0, hwGPIO_Pin_B6, GPIO_AF4_I2C1 },
    { hwI2C_Index_0, hwGPIO_Pin_B7, GPIO_AF4_I2C1 },
    { hwI2C_Index_0, hwGPIO_Pin_B8, GPIO_AF4_I2C1 },
    { hwI2C_Index_0, hwGPIO_Pin_B9, GPIO_AF4_I2C1 },
#endif
#if defined (I2C2_BASE)
    /* I2C2 */
    { hwI2C_Index_1, hwGPIO_Pin_B10, GPIO_AF4_I2C2 },
    { hwI2C_Index_1, hwGPIO_Pin_B11, GPIO_AF4_I2C2 },
    { hwI2C_Index_1, hwGPIO_Pin_F0,  GPIO_AF4_I2C2 },
    { hwI2C_Index_1, hwGPIO_Pin_F1,  GPIO_AF4_I2C2 },
#endif
#if defined (I2C3_BASE)
    /* I2C3 */
    { hwI2C_Index_2, hwGPIO_Pin_A8, GPIO_AF4_I2C3 },
    { hwI2C_Index_2, hwGPIO_Pin_C9, GPIO_AF4_I2C3 },
#endif
};

#endif //I2C_PIN_STM32F2_H