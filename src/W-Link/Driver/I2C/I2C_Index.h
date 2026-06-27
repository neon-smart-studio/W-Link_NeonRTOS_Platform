
#ifndef I2C_INDEX_H
#define I2C_INDEX_H

#ifdef DEVICE_NUVOTON
typedef enum hwI2C_Index_t
{
#if defined (I2C0_BASE)
  hwI2C_Index_0 = 0,
#endif
#if defined (I2C1_BASE)
  hwI2C_Index_1,
#endif
#if defined (I2C2_BASE)
  hwI2C_Index_2,
#endif
#if defined (I2C3_BASE)
  hwI2C_Index_3,
#endif
#if defined (I2C4_BASE)
  hwI2C_Index_4,
#endif
  hwI2C_Index_MAX,
}hwI2C_Index;
#endif // DEVICE_NUVOTON

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

#ifdef DEVICE_TITIVAC
#if defined(TM4C123)
typedef enum hwI2C_Index_t
{
  hwI2C_Index_0 = 0,
  hwI2C_Index_1,
  hwI2C_Index_2,
  hwI2C_Index_3,
  hwI2C_Index_MAX,
}hwI2C_Index;
#endif
#if defined(TM4C1294)
typedef enum hwI2C_Index_t
{
  hwI2C_Index_0 = 0,
  hwI2C_Index_1,
  hwI2C_Index_2,
  hwI2C_Index_3,
  hwI2C_Index_4,
  hwI2C_Index_5,
  hwI2C_Index_6,
  hwI2C_Index_7,
  hwI2C_Index_8,
  hwI2C_Index_9,
  hwI2C_Index_MAX,
}hwI2C_Index;
#endif
#endif // DEVICE_TITIVAC

#endif //I2C_INDEX_H