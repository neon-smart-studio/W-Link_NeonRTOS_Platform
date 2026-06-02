
#ifndef ETHERNET_H
#define ETHERNET_H

#include <stdbool.h>
#include <stdint.h>

#include "soc.h"  

#include "Ethernet_Def.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef void(* onLinkUpCallback)(void);
typedef void(* onLinkDownCallback)(void);

hwEthernet_OpResult Ethernet_Init(const uint8_t mac[6], onLinkUpCallback link_up_cb, onLinkDownCallback link_down_cb);
hwEthernet_OpResult Ethernet_Output(const uint8_t *out_data, uint16_t out_len);
hwEthernet_OpResult Ethernet_Input(uint8_t *in_data, uint16_t in_len, uint16_t *actual_len);
bool Ethernet_isInit(void);
void Ethernet_Set_Link();
void Ethernet_Update_Config(bool isLinkUp);
uint32_t Ethernet_Get_Tick(void);
void Ethernet_Get_Hardware_Mac(uint8_t mac[6]);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //ETHERNET_H