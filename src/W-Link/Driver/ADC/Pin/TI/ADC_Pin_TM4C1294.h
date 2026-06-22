
#ifndef ADC_PIN_STM32H723xx_H
#define ADC_PIN_STM32H723xx_H

#include "ADC_Pin_TM4C1294_Def.h"

#define PWM_CH1_PIN		            hwGPIO_Pin_F0
#define PWM_CH2_PIN		            hwGPIO_Pin_F1
#define PWM_CH3_PIN		            hwGPIO_Pin_F2
#define PWM_CH4_PIN		            hwGPIO_Pin_F3
#define PWM_CH5_PIN		            hwGPIO_Pin_G0
#define PWM_CH6_PIN		            hwGPIO_Pin_G1
#define PWM_CH7_PIN		            hwGPIO_Pin_K4
#define PWM_CH8_PIN		            hwGPIO_Pin_K5

const hwGPIO_Pin PWM_Pins[] = { PWM_CH1_PIN, PWM_CH2_PIN, PWM_CH3_PIN, PWM_CH4_PIN, \
                                PWM_CH5_PIN, PWM_CH6_PIN, PWM_CH7_PIN, PWM_CH8_PIN};

#endif //ADC_PIN_STM32H723xx_H