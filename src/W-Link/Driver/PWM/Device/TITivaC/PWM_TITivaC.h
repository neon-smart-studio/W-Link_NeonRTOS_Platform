
#ifndef PWM_TITIVAC_H
#define PWM_TITIVAC_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "PWM/PWM.h"

#include "PWM/Pin/TITivaC/PWM_Pin_TITivaC_Def.h"

#ifdef	__cplusplus
extern "C" {
#endif

uint32_t PWM_Map_Gen(hwPWM_Channel ch);
uint32_t PWM_Map_Gen_Mask(hwPWM_Channel ch);
uint32_t PWM_Map_Out(hwPWM_Channel ch);
uint32_t PWM_Map_Out_Mask(hwPWM_Channel ch);
uint32_t PWM_Map_Pin_Cfg(hwPWM_Channel ch, hwGPIO_Pin pin);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //PWM_TITIVAC_H
