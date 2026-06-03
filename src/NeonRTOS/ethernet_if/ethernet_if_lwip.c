
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "soc.h"

#include "lwip/timeouts.h"
#include "lwip/etharp.h"
#include "lwip/igmp.h"

#include "NeonRTOS.h"

#include "Ethernet/Ethernet.h"

#include "ethernet_if_lwip.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Network interface name */
#define IFNAME0 'e'
#define IFNAME1 '0'

#if LWIP_IGMP
uint32_t ETH_HashTableHigh = 0x0;
uint32_t ETH_HashTableLow = 0x0;
#endif

//#define PKT_TX_DUMP
//#define PKT_RX_DUMP
//#define PKT_DUMP_RAW

struct netif * pNetif = NULL;

static uint8_t tx_frame[ETH_MAX_PACKET_SIZE];
static uint8_t rx_frame[ETH_MAX_PACKET_SIZE];

#if defined(PKT_RX_DUMP) ||  defined(PKT_TX_DUMP)
static void packet_dump(const char * msg, const struct pbuf* p)
{
#ifdef PKT_DUMP_RAW    
    const struct pbuf* q;
    uint32_t i,j;
    uint8_t *ptr;

    UART_Printf("%s %d byte\n", msg, p->tot_len);

    i=0;
    for(q=p; q != NULL; q= q->next)
    {
        ptr = q->payload;

        for(j=0; j<q->len; j++)
        {
            if( (i%8) == 0 )
            {
                UART_Printf("  ");
            }
            if( (i%16) == 0 )
            {
                UART_Printf("\r\n");
            }
            UART_Printf("%02X ", *ptr);

            i++;
            ptr++;
        }
    }

    UART_Printf("\n\n");
#else /* DM9051_DUMP_RAW */
    uint8_t header[6 + 6 + 2];
    uint16_t type;

    pbuf_copy_partial(p, header, sizeof(header), 0);
    type = (header[12] << 8) | header[13];

    UART_Printf("%02X:%02X:%02X:%02X:%02X:%02X <== %02X:%02X:%02X:%02X:%02X:%02X ",
               header[0], header[1], header[2], header[3], header[4], header[5],
               header[6], header[7], header[8], header[9], header[10], header[11]);

    switch (type)
    {
    case 0x0800:
        UART_Printf("IPv4. ");
        break;

    case 0x0806:
        UART_Printf("ARP.  ");
        break;

    case 0x86DD:
        UART_Printf("IPv6. ");
        break;

    default:
        UART_Printf("%04X. ", type);
        break;
    }

    UART_Printf("%s %d byte. \n", msg, p->tot_len);
#endif /* DM9051_DUMP_RAW */
}
#else
#define packet_dump(...)
#endif /* dump */

static err_t Map_Ethernet_OpResult_to_err_t(hwEthernet_OpResult result)
{
    switch (result) {
    case hwEthernet_OK:
        return ERR_OK;
    case hwEthernet_NotInit:
        return ERR_ARG;
    case hwEthernet_InvalidParameter:
        return ERR_ARG;
    case hwEthernet_Busy:
        return ERR_USE;
    case hwEthernet_MemoryError:
    case hwEthernet_BufferError:
        return ERR_MEM;
    case hwEthernet_HwError:
        return ERR_IF;
    case hwEthernet_Unsupport:
        return ERR_VAL;
    default:
        return ERR_IF;
    }
}

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    uint16_t frame_len;
    hwEthernet_OpResult result;

    UNUSED(netif);

    if (p == NULL) {
        return ERR_ARG;
    }

    if ((p->tot_len == 0U) || (p->tot_len > ETH_MAX_PACKET_SIZE)) {
        return ERR_MEM;
    }

    frame_len = (uint16_t)pbuf_copy_partial(p, tx_frame, p->tot_len, 0U);

    if (frame_len != p->tot_len) {
        return ERR_BUF;
    }

    result = Ethernet_Output(tx_frame, frame_len);

    return Map_Ethernet_OpResult_to_err_t(result);
}

static struct pbuf *low_level_input(struct netif *netif)
{
    hwEthernet_OpResult result;
    struct pbuf *p;
    uint32_t framelength = 0U;

    UNUSED(netif);

    result = Ethernet_Get_Input_Frame_Length(&framelength);
    if ((result != hwEthernet_OK) || (framelength == 0U)) {
        return NULL;
    }

    if (framelength > ETH_MAX_PACKET_SIZE) {
        return NULL;
    }

    result = Ethernet_Input(rx_frame, framelength);
    if (result != hwEthernet_OK) {
        return NULL;
    }

    p = pbuf_alloc(PBUF_RAW, (u16_t)framelength, PBUF_POOL);
    if (p == NULL) {
        return NULL;
    }

    if (pbuf_take(p, rx_frame, (u16_t)framelength) != ERR_OK) {
        pbuf_free(p);
        return NULL;
    }

    return p;
}

void ethernetif_input(struct netif *netif)
{
  err_t err;
  struct pbuf *p;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);

  /* no packet could be read, silently ignore this */
  if (p == NULL) {
    return;
  }

  /* entry point to the LwIP stack */
  err = netif->input(p, netif);

  if (err != ERR_OK) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
    pbuf_free(p);
    p = NULL;
  }
}

uint8_t ethernetif_is_init(void)
{
  return Ethernet_isInit();
}

static void ethernetif_onLinkUp()
{
#if LWIP_IGMP
    if (!(pNetif->flags & NETIF_FLAG_IGMP)) {
      pNetif->flags |= NETIF_FLAG_IGMP;
      igmp_init();
      igmp_start(pNetif);
    }
#endif
    netif_set_link_up(pNetif);
}

static void ethernetif_onLinkDown()
{
    netif_set_link_down(pNetif);
}

err_t ethernetif_init(struct netif *netif)
{
  uint8_t macaddress[6];

  Ethernet_Get_Hardware_Mac(macaddress);

  LWIP_ASSERT("netif != NULL", (netif != NULL));

  pNetif = netif;

  hwEthernet_OpResult result;

  result = Ethernet_Init(macaddress, ethernetif_onLinkUp, ethernetif_onLinkDown);
  if(result < hwEthernet_OK)
  {
      return Map_Ethernet_OpResult_to_err_t(result);
  }

  /* Initialize interface hostname */
  pNetif->hostname = "NeonRT";

  pNetif->name[0] = IFNAME0;
  pNetif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  pNetif->output = etharp_output;
  pNetif->linkoutput = low_level_output;

  pNetif->flags |= NETIF_FLAG_LINK_UP;

  /* set MAC hardware address length */
  pNetif->hwaddr_len = ETH_HWADDR_LEN;

  /* set MAC hardware address */
  pNetif->hwaddr[0] =  macaddress[0];
  pNetif->hwaddr[1] =  macaddress[1];
  pNetif->hwaddr[2] =  macaddress[2];
  pNetif->hwaddr[3] =  macaddress[3];
  pNetif->hwaddr[4] =  macaddress[4];
  pNetif->hwaddr[5] =  macaddress[5];

  /* maximum transfer unit */
  pNetif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  pNetif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

#if LWIP_IGMP
  netif_set_igmp_mac_filter(pNetif, igmp_mac_filter);
#endif

#if LWIP_IGMP
  ETH_HashTableHigh = EthHandle.Instance->MACHTHR;
  ETH_HashTableLow = EthHandle.Instance->MACHTLR;
#endif

  return ERR_OK;
}

u32_t sys_now(void)
{
  return Ethernet_Get_Tick();
}

void ethernetif_set_link(struct netif *netif)
{
  Ethernet_Set_Link();
}

void ethernetif_update_config(struct netif *netif)
{
  Ethernet_Update_Config(netif_is_link_up(netif));

  ethernetif_notify_conn_changed(netif);
}

__weak void ethernetif_notify_conn_changed(struct netif *netif)
{
  /* NOTE : This is function clould be implemented in user file
            when the callback is needed,
  */
  UNUSED(netif);
}

#if LWIP_IGMP
err_t igmp_mac_filter(struct netif *netif, const ip4_addr_t *ip4_addr, netif_mac_filter_action action)
{
  uint8_t mac[6];
  const uint8_t *p = (const uint8_t *)ip4_addr;

  mac[0] = 0x01;
  mac[1] = 0x00;
  mac[2] = 0x5E;
  mac[3] = *(p + 1) & 0x7F;
  mac[4] = *(p + 2);
  mac[5] = *(p + 3);

  register_multicast_address(mac);

  return 0;
}

#ifndef HASH_BITS
#define HASH_BITS 6 /* #bits in hash */
#endif

uint32_t ethcrc(const uint8_t *data, size_t length)
{
  uint32_t crc = 0xffffffff;
  size_t i;
  int j;

  for (i = 0; i < length; i++) {
    for (j = 0; j < 8; j++) {
      if (((crc >> 31) ^ (data[i] >> j)) & 0x01) {
        /* x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1 */
        crc = (crc << 1) ^ 0x04C11DB7;
      } else {
        crc = crc << 1;
      }
    }
  }
  return ~crc;
}

void register_multicast_address(const uint8_t *mac)
{
  uint32_t crc;
  uint8_t hash;

  /* Calculate crc32 value of mac address */
  crc = ethcrc(mac, HASH_BITS);

  /*
   * Only upper HASH_BITS are used
   * which point to specific bit in the hash registers
   */
  hash = (crc >> 26) & 0x3F;

  if (hash > 31) {
    ETH_HashTableHigh |= 1 << (hash - 32);
    EthHandle.Instance->MACHTHR = ETH_HashTableHigh;
  } else {
    ETH_HashTableLow |= 1 << hash;
    EthHandle.Instance->MACHTLR = ETH_HashTableLow;
  }
}
#endif /* LWIP_IGMP */

uint32_t ethernetif_get_tick(void)
{
  return sys_now();
}

#ifdef __cplusplus
}
#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/