
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

#ifdef DEVICE_TIMSP432P
typedef enum {
#if defined(EUSCI_B0_BASE)
    hwI2C_Index_0 = 0,
#endif
#if defined(EUSCI_B1_BASE)
    hwI2C_Index_1,
#endif
#if defined(EUSCI_B2_BASE)
    hwI2C_Index_2,
#endif
#if defined(EUSCI_B3_BASE)
    hwI2C_Index_3,
#endif
    hwI2C_Index_MAX
} hwI2C_Index;
#endif // DEVICE_TIMSP432P

#ifdef DEVICE_TIMSP432E
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
#endif // DEVICE_TIMSP432E

#ifdef DEVICE_TIMSPM0
typedef enum hwI2C_Index_t
{
#if defined(I2C0_BASE) || defined(UC0_I2CC_BASE)
    hwI2C_Index_0 = 0,       // I2C0 / UC0 I2C Controller
#endif

#if defined(I2C1_BASE) || defined(UC1_I2CC_BASE)
    hwI2C_Index_1,           // I2C1 / UC1 I2C Controller
#endif

#if defined(I2C2_BASE)
    hwI2C_Index_2,           // I2C2 / UC2 I2C Controller
#endif

#if defined(UC5_I2CC_BASE)
    hwI2C_Index_5,           // I2C5 / UC5 I2C Controller
#endif

#if defined(UC6_I2CC_BASE)
    hwI2C_Index_6,           // I2C6 / UC6 I2C Controller
#endif

    hwI2C_Index_MAX

} hwI2C_Index;
#endif // DEVICE_TIMSPM0

#endif //I2C_INDEX_H