
#ifndef ETHERNETIF_H
#define ETHERNETIF_H

#include "lwip/err.h"
#include "lwip/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
uint8_t ethernetif_is_init(void);
err_t ethernetif_init(struct netif *netif);
void ethernetif_input(struct netif *netif);
void ethernetif_set_link(struct netif *netif);
void ethernetif_update_config(struct netif *netif);
void ethernetif_notify_conn_changed(struct netif *netif);

void ethernetif_set_mac_addr(const uint8_t *mac);
void ethernetif_get_mac_addr(uint8_t *mac);

#if LWIP_IGMP
err_t igmp_mac_filter(struct netif *netif, const ip4_addr_t *ip4_addr, netif_mac_filter_action action);
void register_multicast_address(const uint8_t *mac);
#endif

#if NO_SYS
typedef void(* scheduler_poll_CB)();
void ethernetif_scheduler_gen_event();
void ethernetif_scheduler_config(scheduler_poll_CB poll_cb);
#endif

uint32_t ethernetif_get_tick(void);
void get_hardware_mac(uint8_t *mac);

#ifdef __cplusplus
}
#endif

#endif //ETHERNETIF_H
