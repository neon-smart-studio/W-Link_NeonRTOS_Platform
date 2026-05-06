
#ifndef I2C_INDEX_H
#define I2C_INDEX_H

#ifdef DEVICE_STM32
typedef enum hwI2C_Index_t
{
#if defined (I2C1_BASE)
  hwI2C_Index_0 = 0,
#endif
#if defined (I2C2_BASE)
  hwI2C_Index_1,
#endif
#if defined (I2C3_BASE)
  hwI2C_Index_2,
#endif
#if defined (I2C4_BASE)
  hwI2C_Index_3,
#endif
#if defined (I2C5_BASE)
  hwI2C_Index_4,
#endif
#if defined (I2C6_BASE)
  hwI2C_Index_5,
#endif
  hwI2C_Index_MAX,
}hwI2C_Index;
#endif // DEVICE_STM32

#ifdef DEVICE_RP2
typedef enum hwI2C_Index_t
{
  hwI2C_Index_0 = 0,
  hwI2C_Index_1,
  hwI2C_Index_MAX,
}hwI2C_Index;
#endif // DEVICE_RP2

#endif //I2C_INDEX_H