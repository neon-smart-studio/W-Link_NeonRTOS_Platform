
#ifndef LWIP_CONFIG_H
#define LWIP_CONFIG_H

#define CONFIG_LWIP_HEAP_SIZE             (64 * 1024)
#define CONFIG_LWIP_NUM_NETCONN           16
#define CONFIG_LWIP_NUM_PBUF              64
#define CONFIG_LWIP_NUM_TCP_PCB           16
#define CONFIG_LWIP_NUM_TCP_PCB_LISTEN    8
#define CONFIG_LWIP_NUM_TCP_PCB_SEG       128
#define CONFIG_LWIP_NUM_UDP_PCB           8
#define CONFIG_LWIP_PBUF_POOL_SIZE        64
#define CONFIG_LWIP_NUM_WND               16
#define CONFIG_LWIP_NUM_SND_BUF           16
#define CONFIG_LWIP_NUM_SND_BUF_QUEUELEN  64

#endif //LWIP_CONFIG_H