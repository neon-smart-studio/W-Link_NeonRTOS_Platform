
#ifndef LWIP_CONFIG_H
#define LWIP_CONFIG_H

// dm9051
/*
#define CONFIG_LWIP_HEAP_SIZE             (85 * 1024)
#define CONFIG_LWIP_NUM_NETCONN           24
#define CONFIG_LWIP_NUM_PBUF              64
#define CONFIG_LWIP_NUM_TCP_PCB           24
#define CONFIG_LWIP_NUM_TCP_PCB_LISTEN    8
#define CONFIG_LWIP_NUM_TCP_PCB_SEG       128
#define CONFIG_LWIP_NUM_UDP_PCB           8
#define CONFIG_LWIP_PBUF_POOL_SIZE        48
#define CONFIG_LWIP_NUM_WND               4
#define CONFIG_LWIP_NUM_SND_BUF           4
#define CONFIG_LWIP_NUM_SND_BUF_QUEUELEN  16
*/

#define CONFIG_LWIP_HEAP_SIZE             (256 * 1024)
#define CONFIG_LWIP_NUM_NETCONN           24
#define CONFIG_LWIP_NUM_PBUF              64
#define CONFIG_LWIP_NUM_TCP_PCB           24
#define CONFIG_LWIP_NUM_TCP_PCB_LISTEN    8
#define CONFIG_LWIP_NUM_TCP_PCB_SEG       128
#define CONFIG_LWIP_NUM_UDP_PCB           8
#define CONFIG_LWIP_PBUF_POOL_SIZE        48
#define CONFIG_LWIP_NUM_WND               4
#define CONFIG_LWIP_NUM_SND_BUF           4
#define CONFIG_LWIP_NUM_SND_BUF_QUEUELEN  16

#endif //LWIP_CONFIG_H