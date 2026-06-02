
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "lwip/inet.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "lwip/dns.h"
#include "lwip/tcpbase.h"
#include "netif/ethernet.h"

#include "ethernet_if/ethernet_if_lwip.h"

#include "NeonTCPIP.h"

#if !NO_SYS
#include "NeonRTOS.h"

// 定義網絡任務的相關屬性
#define NEON_TCPIP_TASK_STACK_SIZE 2048
#define NEON_TCPIP_TASK_PRIORITY   tskIDLE_PRIORITY + 1
#endif

/* Check ethernet link status every seconds */
#define TIME_CHECK_ETH_LINK_STATE 500U

/* Timeout for DNS request */
#define TIMEOUT_DNS_REQUEST 10000U

/* Maximum number of retries for DHCP request */
#define MAX_DHCP_TRIES  10

#define TIMEOUT_DHCP_REQUEST 60000U

#define DHCP_CHECK_NONE         (0)
#define DHCP_CHECK_RENEW_FAIL   (1)
#define DHCP_CHECK_RENEW_OK     (2)
#define DHCP_CHECK_REBIND_FAIL  (3)
#define DHCP_CHECK_REBIND_OK    (4)

static void NeonTCPIP_ETH_Scheduler(void);
static void NeonTCPIP_DHCP_Periodic_Handle(struct netif *netif);
static void NeonTCPIP_DHCP_Process(struct netif *netif);

/* Ethernet configuration: user parameters */
struct neonRT_eth_config {
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
};

typedef struct {
    const uint8_t *mac;
    const uint8_t *ip;
    const uint8_t *gw;
    const uint8_t *netmask;
} NeonTCPIP_Task_Params;

/* Use to give user parameters to netif configuration */
static struct neonRT_eth_config gconfig;

/* Netif global configuration structure */
struct netif gnetif;

/* DHCP periodic timer */
static uint32_t DHCPfineTimer = 0;

/* DHCP current state */
volatile uint8_t DHCP_state = DHCP_OFF;
volatile uint8_t DHCP_lease_state = DHCP_CHECK_NONE;

/* Ethernet link status periodic timer */
static uint32_t gEhtLinkTickStart = 0;

static uint8_t hardware_mac[6] = {0};

static int check_DHCP_lease()
{
  int rc = DHCP_CHECK_NONE;

  NeonTCPIP_ETH_Scheduler();
  rc = NeonTCPIP_Get_DHCP_Lease_State();

  if (rc != DHCP_lease_state) {
    switch (DHCP_lease_state) {
      case DHCP_CHECK_NONE:
        DHCP_lease_state = rc;
        rc = DHCP_CHECK_NONE;
        break;

      case DHCP_CHECK_RENEW_OK:
        DHCP_lease_state = rc;
        if (rc == DHCP_CHECK_NONE) {
          rc = DHCP_CHECK_RENEW_OK;
        } else {
          rc = DHCP_CHECK_RENEW_FAIL;
        }
        break;

      case DHCP_CHECK_REBIND_OK:
        DHCP_lease_state = rc;
        if (rc == DHCP_CHECK_NONE) {
          rc = DHCP_CHECK_REBIND_OK;
        } else {
          rc = DHCP_CHECK_REBIND_FAIL;
        }
        break;

      default:
        DHCP_lease_state = DHCP_CHECK_NONE;
        break;
    }
  }

  return rc;
}

static void NeonTCPIP_ETH_Poll(void)
{
  /* Read a received packet from the Ethernet buffers and send it
  to the lwIP for handling */
  ethernetif_input(&gnetif);

  /* Check ethernet link status */
  if ((ethernetif_get_tick() - gEhtLinkTickStart) >= TIME_CHECK_ETH_LINK_STATE) {
    ethernetif_set_link(&gnetif);
    gEhtLinkTickStart = ethernetif_get_tick();
  }

  /* Handle LwIP timeouts */
  sys_check_timeouts();

#if LWIP_DHCP
  NeonTCPIP_DHCP_Periodic_Handle(&gnetif);
#endif /* LWIP_DHCP */
}

static void NeonTCPIP_ETH_Scheduler(void)
{
  NeonTCPIP_ETH_Poll();
}

static void NeonTCPIP_ETH_Init(const uint8_t *ip, const uint8_t *gw, const uint8_t *netmask)
{
    /* Initialize the LwIP stack */
    lwip_init();

	if (ip != NULL) {
		IP_ADDR4(&(gconfig.ipaddr), ip[0], ip[1], ip[2], ip[3]);
	} else {
	#if LWIP_DHCP
		ip_addr_set_zero_ip4(&(gconfig.ipaddr));
	#else
		IP_ADDR4(&(gconfig.ipaddr), IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
	#endif /* LWIP_DHCP */
	}

	if (gw != NULL) {
		IP_ADDR4(&(gconfig.gw), gw[0], gw[1], gw[2], gw[3]);
	} else {
	#if LWIP_DHCP
		ip_addr_set_zero_ip4(&(gconfig.gw));
	#else
		IP_ADDR4(&(gconfig.gw), GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	#endif /* LWIP_DHCP */
	}

  if (netmask != NULL) {
    IP_ADDR4(&(gconfig.netmask), netmask[0], netmask[1], netmask[2], netmask[3]);
  } else {
#if LWIP_DHCP
    ip_addr_set_zero_ip4(&(gconfig.netmask));
#else
    IP_ADDR4(&(gconfig.netmask), NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
#endif /* LWIP_DHCP */
  }

	/* Configure the Network interface */
	netif_remove(&gnetif);
	/* Add the network interface */
	netif_add(&gnetif, &(gconfig.ipaddr), &(gconfig.netmask), &(gconfig.gw), NULL, &ethernetif_init, &ethernet_input);

	/* Registers the default network interface */
	netif_set_default(&gnetif);

	if (netif_is_link_up(&gnetif)) {
		/* When the netif is fully configured this function must be called */
		netif_set_up(&gnetif);
	} else {
		/* When the netif link is down this function must be called */
		netif_set_down(&gnetif);
	}

#if LWIP_NETIF_LINK_CALLBACK
	/* Set the link callback function, this function is called on change of link status */
	netif_set_link_callback(&gnetif, ethernetif_update_config);
#endif /* LWIP_NETIF_LINK_CALLBACK */
}

void NeonTCPIP_Task(void *pValue) {
    while (1) {
      NeonTCPIP_ETH_Scheduler(); // 調用調度函數
      NeonRTOS_Sleep(1);    // 每 1 毫秒調用一次
    }
}

void NeonTCPIP_init(const uint8_t *ip, const uint8_t *gw, const uint8_t *netmask)
{
    NeonTCPIP_ETH_Init(ip, gw, netmask);

    // 創建 NeonRTOS 網絡任務
    NeonRTOS_ReturnVal_e ret = NeonRTOS_TaskCreate(
        NeonTCPIP_Task,                 // 任務入口函數
        "NeonTCPIP_Task",               // 任務名稱
        NEON_TCPIP_TASK_STACK_SIZE,     // 堆疊大小
        NULL,                        // 任務參數
        NEON_TCPIP_TASK_PRIORITY,       // 任務優先級
        NULL                            // 任務句柄
    );

    if (ret != NeonRTOS_OK) {
        return;
    }

    /* Reset DHCP if used */
    if (netif_is_up(&gnetif)) {
      DHCP_state = DHCP_START;
    } else {
      DHCP_state = DHCP_LINK_DOWN;
    }
}

uint8_t NeonTCPIP_IF_isInit(void)
{
  return ethernetif_is_init();
}

uint8_t NeonTCPIP_IF_isLinkUp(void)
{
  return netif_is_link_up(&gnetif);
}

#if LWIP_DHCP
static void NeonTCPIP_DHCP_Process(struct netif *netif)
{
  struct dhcp *dhcp;

  if (netif_is_link_up(netif)) {
    switch (DHCP_state) {
      case DHCP_START: {
          ip_addr_set_zero_ip4(&netif->ip_addr);
          ip_addr_set_zero_ip4(&netif->netmask);
          ip_addr_set_zero_ip4(&netif->gw);
          DHCP_state = DHCP_WAIT_ADDRESS;
          dhcp_start(netif);
        }
        break;

      case DHCP_WAIT_ADDRESS: {
          if (dhcp_supplied_address(netif)) {
            UART_Printf("Assigned IP: %d.%d.%d.%d\n",
              ip4_addr1(&netif->ip_addr),
              ip4_addr2(&netif->ip_addr),
              ip4_addr3(&netif->ip_addr),
              ip4_addr4(&netif->ip_addr)
            );
            DHCP_state = DHCP_ADDRESS_ASSIGNED;
          } else {
            dhcp = (struct dhcp *)netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

            /* DHCP timeout */
            if (dhcp->tries > MAX_DHCP_TRIES) {
              DHCP_state = DHCP_TIMEOUT;

              /* Stop DHCP */
              dhcp_stop(netif);
            }
          }
        }
        break;
      case DHCP_ASK_RELEASE: {
          /* Force release */
          dhcp_release(netif);
          dhcp_stop(netif);
          DHCP_state = DHCP_OFF;
        }
        break;
      case DHCP_LINK_DOWN: {
          /* Stop DHCP */
          dhcp_stop(netif);
          DHCP_state = DHCP_OFF;
        }
        break;
      default: break;
    }
  } else {
    DHCP_state = DHCP_OFF;
  }
}

static void NeonTCPIP_DHCP_Periodic_Handle(struct netif *netif)
{
  /* Fine DHCP periodic process every 500ms */
  if (ethernetif_get_tick() - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS) {
    DHCPfineTimer =  ethernetif_get_tick();
    /* process DHCP state machine */
    NeonTCPIP_DHCP_Process(netif);
  }
}

void NeonTCPIP_DHCP_Manual_Config(void)
{
  dhcp_inform(&gnetif);
}

uint8_t NeonTCPIP_Get_DHCP_Lease_State(void)
{
  uint8_t res = DHCP_CHECK_NONE;
  struct dhcp *dhcp = (struct dhcp *)netif_get_client_data(&gnetif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

  if (dhcp->state == DHCP_STATE_RENEWING) {
    res = DHCP_CHECK_RENEW_OK;
  } else if (dhcp->state == DHCP_STATE_REBINDING) {
    res = DHCP_CHECK_REBIND_OK;
  }

  return res;
}

void NeonTCPIP_Set_DHCP_State(uint8_t state)
{
  DHCP_state = state;
}

uint8_t NeonTCPIP_Get_DHCP_State(void)
{
  return DHCP_state;
}

#endif /* LWIP_DHCP */

void NeonTCPIP_IF_Update_Addresses(uint32_t ip, uint32_t netmask, uint32_t gw)
{
    ip4_addr_t ipaddr, nm, gwaddr;

    ip4_addr_set_u32(&ipaddr, ip);
    ip4_addr_set_u32(&nm, netmask);
    ip4_addr_set_u32(&gwaddr, gw);

#if LWIP_DHCP
    NeonTCPIP_Set_DHCP_State(DHCP_OFF);
    dhcp_stop(&gnetif);
#endif // LWIP_DHCP
    netif_set_addr(&gnetif, &ipaddr, &nm, &gwaddr);
}

uint32_t NeonTCPIP_IF_Get_IP_Address(void)
{
  return ip4_addr_get_u32(&(gnetif.ip_addr));
}

void NeonTCPIP_IF_Set_IP_Address(uint32_t ip_addr)
{
    ip4_addr_t ipaddr, netmask, gw;

    ip4_addr_set_u32(&ipaddr, ip_addr);
    // 用原本的 netmask 與 gw
    ip4_addr_copy(netmask, gnetif.netmask);
    ip4_addr_copy(gw, gnetif.gw);

#if LWIP_DHCP
    NeonTCPIP_Set_DHCP_State(DHCP_OFF);
    dhcp_stop(&gnetif);
#endif // LWIP_DHCP
    netif_set_addr(&gnetif, &ipaddr, &netmask, &gw);
}

uint32_t NeonTCPIP_IF_Get_Gateway_Address(void)
{
  return ip4_addr_get_u32(&(gnetif.gw));
}

void NeonTCPIP_IF_Set_Gateway_Address(uint32_t gateway)
{
    ip4_addr_t ipaddr, netmask, gw;

    ip4_addr_set_u32(&gw, gateway);
    // 用原本的 netmask 與 gw
    ip4_addr_copy(netmask, gnetif.netmask);
    ip4_addr_copy(ipaddr, gnetif.ip_addr);

#if LWIP_DHCP
    NeonTCPIP_Set_DHCP_State(DHCP_OFF);
    dhcp_stop(&gnetif);
#endif // LWIP_DHCP
    netif_set_addr(&gnetif, &ipaddr, &netmask, &gw);
}

uint32_t NeonTCPIP_IF_Get_NetMask_Address(void)
{
  return ip4_addr_get_u32(&(gnetif.netmask));
}

void NeonTCPIP_IF_Set_NetMask_Address(uint32_t net_mask)
{
    ip4_addr_t ipaddr, netmask, gw;

    ip4_addr_set_u32(&netmask, net_mask);
    // 用原本的 netmask 與 gw
    ip4_addr_copy(gw, gnetif.gw);
    ip4_addr_copy(ipaddr, gnetif.ip_addr);

#if LWIP_DHCP
    NeonTCPIP_Set_DHCP_State(DHCP_OFF);
    dhcp_stop(&gnetif);
#endif // LWIP_DHCP
    netif_set_addr(&gnetif, &ipaddr, &netmask, &gw);
}

uint32_t NeonTCPIP_IF_Get_DNS_Address(void)
{
  const ip_addr_t *tmp = dns_getserver(0);
  return ip4_addr_get_u32(tmp);
}

#if LWIP_DHCP
uint32_t NeonTCPIP_IF_Get_DHCP_Address(void)
{
  struct dhcp *dhcp = (struct dhcp *)netif_get_client_data(&gnetif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);
  return ip4_addr_get_u32(&(dhcp->server_ip_addr));
}
#endif /* LWIP_DHCP */

#if LWIP_NETIF_LINK_CALLBACK

void ethernetif_notify_conn_changed(struct netif *netif)
{
  if (netif_is_link_up(netif)) {
    /* Update DHCP state machine if DHCP used */
    DHCP_state = DHCP_START;

    /* When the netif is fully configured this function must be called.*/
    netif_set_up(netif);
  } else {
    /* Update DHCP state machine if DHCP used */
      DHCP_state = DHCP_LINK_DOWN;

    /*  When the netif link is down this function must be called.*/
    netif_set_down(netif);
  }
}

#endif /* LWIP_NETIF_LINK_CALLBACK */

#if LWIP_DNS

void NeonTCPIP_DNS_Init(const uint8_t *dnsaddr)
{
  ip_addr_t ip;

  /* DNS initialized by DHCP when call dhcp_start() */
#if LWIP_DHCP
  if (NeonTCPIP_Get_DHCP_State!=DHCP_START)
#endif // LWIP_DHCP
  {
    dns_init();
    IP_ADDR4(&ip, dnsaddr[0], dnsaddr[1], dnsaddr[2], dnsaddr[3]);
    dns_setserver(0, &ip);
  }
}

void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
  (void)(name);

  if (ipaddr != NULL) {
    *((uint32_t *)callback_arg) = ip4_addr_get_u32(ipaddr);
  } else {
    *((uint32_t *)callback_arg) = 0;
  }
}

int8_t NeonTCPIP_DNS_GetHostByName(const char *hostname, uint32_t *ipaddr)
{
  ip_addr_t iphost;
  err_t err;
  uint32_t tickstart = 0;
  int8_t ret = 0;

  *ipaddr = 0;
  err = dns_gethostbyname(hostname, &iphost, &dns_callback, ipaddr);

  switch (err) {
    case ERR_OK:
      *ipaddr = ip4_addr_get_u32(&iphost);
      ret = 1;
      break;

    case ERR_INPROGRESS:
      tickstart = ethernetif_get_tick();
      while (*ipaddr == 0) {
        NeonTCPIP_ETH_Scheduler();
        if ((ethernetif_get_tick() - tickstart) >= TIMEOUT_DNS_REQUEST) {
          ret = -1;
          break;
        }
      }

      if (ret == 0) {
        if (*ipaddr == 0) {
          ret = -2;
        } else {
          ret = 1;
        }
      }
      break;

    case ERR_ARG:
      ret = -4;
      break;

    default:
      ret = -4;
      break;
  }

  return ret;
}

#endif /* LWIP_DNS */

ip_addr_t *u8_to_ip_addr(uint8_t *ipu8, ip_addr_t *ipaddr)
{
  IP_ADDR4(ipaddr, ipu8[0], ipu8[1], ipu8[2], ipu8[3]);
  return ipaddr;
}

uint32_t ip_addr_to_u32(ip_addr_t *ipaddr)
{
  return ip4_addr_get_u32(ipaddr);
}

uint32_t ip_string_to_u32(const char *ip_str)
{
    ip4_addr_t ipaddr;
    if (ip4addr_aton(ip_str, &ipaddr)) {
        return ip4_addr_get_u32(&ipaddr);
    } else {
        // 錯誤處理，可回傳 0 或者定義錯誤碼
        return 0;
    }
}

void ip_u32_to_string(uint32_t ip_u32, char *buf, size_t buflen)
{
    ip4_addr_t ipaddr;
    ip4_addr_set_u32(&ipaddr, ip_u32);
    snprintf(buf, buflen, "%s", ip4addr_ntoa(&ipaddr));
}
