
#include <stdbool.h>
#include <stdint.h>

#include "NeonRTOS.h"

#include "NeonTCPIP.h"

#ifdef CONFIG_INTERNET_LWIP
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "lwip/inet.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "lwip/dns.h"
#include "lwip/tcpbase.h"
#include "lwip/ip_addr.h"
#include "netif/ethernet.h"
#include "lwip/err.h"
#include "lwip/sockets.h"

#include "ethernet_if/ethernet_if_lwip.h"
#endif

#ifdef CONFIG_INTERNET_SIMPLELINK
#include "simplelink.h"
#endif

#ifdef CONFIG_INTERNET_LWIP

/* Maximum number of client per server */
#define MAX_CLIENT  32

// 定義網絡任務的相關屬性
#define NEON_TCPIP_TASK_STACK_SIZE 2048
#define NEON_TCPIP_TASK_PRIORITY   tskIDLE_PRIORITY + 1

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

#if LWIP_DHCP
void NeonTCPIP_DHCP_Manual_Config(void);
uint8_t NeonTCPIP_Get_DHCP_Lease_State(void);
void NeonTCPIP_Set_DHCP_State(uint8_t state);
uint8_t NeonTCPIP_Get_DHCP_State(void);
#endif /* LWIP_DHCP */

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
#if LWIP_DHCP
volatile uint8_t DHCP_state = DHCP_OFF;
volatile uint8_t DHCP_lease_state = DHCP_CHECK_NONE;
static volatile bool DHCP_enabled = 0;
#endif

/* Ethernet link status periodic timer */
static uint32_t gEhtLinkTickStart = 0;

static bool NeonTCPIP_Is_Valid_Netmask(uint32_t mask)
{
    uint32_t m = lwip_ntohl(mask);

    if (m == 0 || m == 0xFFFFFFFF) return false;

    /* 合法 netmask 必須是連續 1 後接連續 0 */
    return ((m | (m - 1)) == 0xFFFFFFFF);
}

static bool NeonTCPIP_Is_Valid_Static_Address(uint32_t ip,
                                              uint32_t netmask,
                                              uint32_t gw)
{
    uint32_t ip_h = lwip_ntohl(ip);
    uint32_t gw_h = lwip_ntohl(gw);
    uint32_t nm_h = lwip_ntohl(netmask);

    if (!NeonTCPIP_Is_Valid_Netmask(netmask)) return false;

    if (ip_h == 0 || ip_h == 0xFFFFFFFF) return false;
    if (gw_h == 0 || gw_h == 0xFFFFFFFF) return false;

    if ((ip_h & 0xFF000000) == 0x7F000000) return false; /* 127.x.x.x */
    if ((ip_h & 0xF0000000) == 0xE0000000) return false; /* multicast */
    if ((ip_h & 0xF0000000) == 0xF0000000) return false;

    uint32_t network   = ip_h & nm_h;
    uint32_t broadcast = network | ~nm_h;

    if (ip_h == network || ip_h == broadcast) return false;
    if (gw_h == network || gw_h == broadcast) return false;

    if ((ip_h & nm_h) != (gw_h & nm_h)) return false;

    return true;
}

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

static void NeonTCPIP_Task(void *pValue) {
    while (1) {
      NeonTCPIP_ETH_Scheduler(); // 調用調度函數
      NeonRTOS_Sleep(1);    // 每 1 毫秒調用一次
    }
}

static void NeonTCPIP_ETH_Init(const uint8_t *ip, const uint8_t *gw, const uint8_t *netmask)
{
    bool use_dhcp = false;

    lwip_init();

#if LWIP_DHCP
    if (ip == NULL || gw == NULL || netmask == NULL) {
        use_dhcp = true;
    } else {
        ip4_addr_t ipaddr, gwaddr, nmaddr;

        IP_ADDR4(&ipaddr, ip[0], ip[1], ip[2], ip[3]);
        IP_ADDR4(&gwaddr, gw[0], gw[1], gw[2], gw[3]);
        IP_ADDR4(&nmaddr, netmask[0], netmask[1], netmask[2], netmask[3]);

        if (!NeonTCPIP_Is_Valid_Static_Address(ip4_addr_get_u32(&ipaddr),
                                               ip4_addr_get_u32(&nmaddr),
                                               ip4_addr_get_u32(&gwaddr))) {
            use_dhcp = true;
        }
    }
#endif

#if LWIP_DHCP
    if (use_dhcp) {
        ip_addr_set_zero_ip4(&(gconfig.ipaddr));
        ip_addr_set_zero_ip4(&(gconfig.gw));
        ip_addr_set_zero_ip4(&(gconfig.netmask));
    } else
#endif
    {
        if (ip != NULL) {
            IP_ADDR4(&(gconfig.ipaddr), ip[0], ip[1], ip[2], ip[3]);
        } else {
            IP_ADDR4(&(gconfig.ipaddr), IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
        }

        if (gw != NULL) {
            IP_ADDR4(&(gconfig.gw), gw[0], gw[1], gw[2], gw[3]);
        } else {
            IP_ADDR4(&(gconfig.gw), GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
        }

        if (netmask != NULL) {
            IP_ADDR4(&(gconfig.netmask), netmask[0], netmask[1], netmask[2], netmask[3]);
        } else {
            IP_ADDR4(&(gconfig.netmask), NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
        }
    }

    netif_remove(&gnetif);

    netif_add(&gnetif,
              &(gconfig.ipaddr),
              &(gconfig.netmask),
              &(gconfig.gw),
              NULL,
              &ethernetif_init,
              &ethernet_input);

    netif_set_default(&gnetif);

    if (netif_is_link_up(&gnetif)) {
        netif_set_up(&gnetif);
    } else {
        netif_set_down(&gnetif);
    }

#if LWIP_NETIF_LINK_CALLBACK
    netif_set_link_callback(&gnetif, ethernetif_update_config);
#endif

#if LWIP_DHCP
    DHCP_enabled = use_dhcp ? 1 : 0;
#endif

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

#if LWIP_DHCP
    if (netif_is_up(&gnetif)) {
        if (DHCP_enabled) {
            DHCP_state = DHCP_START;
        } else {
            DHCP_state = DHCP_OFF;
        }
    } else {
        DHCP_state = DHCP_enabled ? DHCP_LINK_DOWN : DHCP_OFF;
    }
#endif
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

void NeonTCPIP_DHCP_Enable(void)
{
    DHCP_enabled = 1;

    if (netif_is_link_up(&gnetif)) {
        netif_set_up(&gnetif);

        ip_addr_set_zero_ip4(&gnetif.ip_addr);
        ip_addr_set_zero_ip4(&gnetif.netmask);
        ip_addr_set_zero_ip4(&gnetif.gw);

        DHCP_state = DHCP_START;
    } else {
        DHCP_state = DHCP_LINK_DOWN;
    }
}

void NeonTCPIP_DHCP_Disable(void)
{
    DHCP_enabled = 0;

    /* 只切換模式，不釋放目前 DHCP IP */
    DHCP_state = DHCP_OFF;
}

bool NeonTCPIP_DHCP_IsEnabled(void)
{
    return DHCP_enabled;
}

#endif /* LWIP_DHCP */

void NeonTCPIP_IF_Update_Addresses(uint32_t ip, uint32_t netmask, uint32_t gw)
{
    ip4_addr_t ipaddr, nm, gwaddr;

    if (!NeonTCPIP_Is_Valid_Static_Address(ip, netmask, gw)) {
#if LWIP_DHCP
        NeonTCPIP_DHCP_Enable();
#endif
        return;
    }

    ip4_addr_set_u32(&ipaddr, ip);
    ip4_addr_set_u32(&nm, netmask);
    ip4_addr_set_u32(&gwaddr, gw);

#if LWIP_DHCP
    DHCP_enabled = 0;
    DHCP_state = DHCP_OFF;
    dhcp_stop(&gnetif);
#endif

    netif_set_addr(&gnetif, &ipaddr, &nm, &gwaddr);

    return;
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

void NeonTCPIP_IF_Get_Mac_Address(uint8_t mac[6])
{
  ethernetif_get_hardware_mac(mac);
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
#if LWIP_DHCP
    if (DHCP_enabled) {
        DHCP_state = DHCP_START;
    }
#endif

    /* When the netif is fully configured this function must be called.*/
    netif_set_up(netif);
  } else {
    /* Update DHCP state machine if DHCP used */
#if LWIP_DHCP
    if (DHCP_enabled) {
        DHCP_state = DHCP_LINK_DOWN;
    }
#endif

    /*  When the netif link is down this function must be called.*/
    netif_set_down(netif);
  }
}

#endif /* LWIP_NETIF_LINK_CALLBACK */

#if LWIP_DNS

uint32_t NeonTCPIP_IF_Get_DNS_Address(void)
{
  const ip_addr_t *tmp = dns_getserver(0);
  return ip4_addr_get_u32(tmp);
}

void NeonTCPIP_IF_Set_DNS_Address(const uint8_t *dnsaddr)
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

static int32_t NeonTCPIP_Map_Error(int err)
{
    switch (err)
    {
        case 0:
            return NeonTCPIP_OK;

#if defined(EWOULDBLOCK)
        case EWOULDBLOCK:
            return NeonTCPIP_WouldBlock;
#endif

#if defined(EAGAIN) && (!defined(EWOULDBLOCK) || (EAGAIN != EWOULDBLOCK))
        case EAGAIN:
            return NeonTCPIP_WouldBlock;
#endif

#if defined(ETIMEDOUT)
        case ETIMEDOUT:
            return NeonTCPIP_Timeout;
#endif

        default:
            return NeonTCPIP_Error;
    }
}

NeonTCPIP_SocketHandle NeonTCPIP_Socket_Open(bool udp)
{
    int type = udp ? SOCK_DGRAM : SOCK_STREAM;
    int proto = udp ? IPPROTO_UDP : IPPROTO_TCP;

    int s = lwip_socket(AF_INET, type, proto);
    if (s < 0)
    {
        return s;
    }

    return (NeonTCPIP_SocketHandle)s;
}

NeonTCPIP_Result NeonTCPIP_Socket_SetNonBlock(NeonTCPIP_SocketHandle sock, bool enable)
{
    if (sock < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    int flags = lwip_fcntl(sock, F_GETFL, 0);
    if (flags < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(errno);
    }

    if (enable)
    {
        flags |= O_NONBLOCK;
    }
    else
    {
        flags &= ~O_NONBLOCK;
    }

    if (lwip_fcntl(sock, F_SETFL, flags) < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(errno);
    }

    return NeonTCPIP_OK;
}

NeonTCPIP_Result NeonTCPIP_Socket_Bind(NeonTCPIP_SocketHandle sock, uint16_t port)
{
    if (sock < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = lwip_htons(port);
    addr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);

    if (lwip_bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(errno);
    }

    return NeonTCPIP_OK;
}

NeonTCPIP_Result NeonTCPIP_Socket_Listen(NeonTCPIP_SocketHandle sock, int32_t backlog)
{
    if (sock < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    if (lwip_listen(sock, backlog) < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(errno);
    }

    return NeonTCPIP_OK;
}

NeonTCPIP_SocketHandle NeonTCPIP_Socket_Accept(NeonTCPIP_SocketHandle sock)
{
    if (sock < 0)
    {
        return -1;
    }

    int client_socet = lwip_accept(sock, NULL, NULL);
    if (client_socet < 0)
    {
        return client_socet;
    }

    return (NeonTCPIP_SocketHandle)client_socet;
}

int32_t NeonTCPIP_Socket_Recv(NeonTCPIP_SocketHandle sock, void *buf, uint32_t len)
{
    if ((sock < 0) || (buf == NULL) || (len == 0))
    {
        return NeonTCPIP_InvalidParameter;
    }

    int32_t ret = lwip_recv(sock, buf, len, 0);

    if (ret > 0)
    {
        return ret;
    }

    if (ret == 0)
    {
        return NeonTCPIP_Closed;
    }

    return NeonTCPIP_Map_Error(errno);
}

int32_t NeonTCPIP_Socket_Send(NeonTCPIP_SocketHandle sock, const void *buf, uint32_t len)
{
    if ((sock < 0) || (buf == NULL) || (len == 0))
    {
        return NeonTCPIP_InvalidParameter;
    }

    int32_t ret = lwip_send(sock, buf, len, 0);

    if (ret >= 0)
    {
        return ret;
    }

    return NeonTCPIP_Map_Error(errno);
}

int32_t NeonTCPIP_Socket_RecvFrom(NeonTCPIP_SocketHandle sock,
                                  void *buf,
                                  uint32_t len,
                                  NeonTCPIP_SocketAddr *from)
{
    if ((sock < 0) || (buf == NULL) || (len == 0))
    {
        return NeonTCPIP_InvalidParameter;
    }

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));

    int32_t ret = lwip_recvfrom(sock,
                            buf,
                            len,
                            0,
                            (struct sockaddr *)&addr,
                            &addr_len);

    if (ret >= 0)
    {
        if (from != NULL)
        {
            uint32_t ip = lwip_ntohl(addr.sin_addr.s_addr);

            from->ip[0] = (uint8_t)((ip >> 24) & 0xFF);
            from->ip[1] = (uint8_t)((ip >> 16) & 0xFF);
            from->ip[2] = (uint8_t)((ip >> 8) & 0xFF);
            from->ip[3] = (uint8_t)(ip & 0xFF);
            from->port = lwip_ntohs(addr.sin_port);
        }

        return ret;
    }

    return NeonTCPIP_Map_Error(errno);
}

int32_t NeonTCPIP_Socket_SendTo(NeonTCPIP_SocketHandle sock,
                                const void *buf,
                                uint32_t len,
                                const NeonTCPIP_SocketAddr *to)
{
    if ((sock < 0) || (buf == NULL) || (len == 0) || (to == NULL))
    {
        return NeonTCPIP_InvalidParameter;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    uint32_t ip =
        ((uint32_t)to->ip[0] << 24) |
        ((uint32_t)to->ip[1] << 16) |
        ((uint32_t)to->ip[2] << 8) |
        ((uint32_t)to->ip[3]);

    addr.sin_family = AF_INET;
    addr.sin_port = lwip_htons(to->port);
    addr.sin_addr.s_addr = lwip_htonl(ip);

    int32_t ret = lwip_sendto(sock,
                          buf,
                          len,
                          0,
                          (struct sockaddr *)&addr,
                          sizeof(addr));

    if (ret >= 0)
    {
        return ret;
    }

    return NeonTCPIP_Map_Error(errno);
}

NeonTCPIP_Result NeonTCPIP_Socket_Close(NeonTCPIP_SocketHandle sock)
{
    if (sock < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    if (lwip_close(sock) < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(errno);
    }

    return NeonTCPIP_OK;
}

int32_t NeonTCPIP_Socket_GetLastError(void)
{
    return errno;
}

NeonTCPIP_Result NeonTCPIP_Socket_SetTcpNoDelay(NeonTCPIP_SocketHandle sock, bool enable)
{
    if (sock < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    int value = enable ? 1 : 0;

    if (lwip_setsockopt(sock,
                        IPPROTO_TCP,
                        TCP_NODELAY,
                        &value,
                        sizeof(value)) < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(errno);
    }

    return NeonTCPIP_OK;
}

NeonTCPIP_Result NeonTCPIP_Socket_SetKeepAlive(NeonTCPIP_SocketHandle sock, bool enable)
{
    if (sock < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    int value = enable ? 1 : 0;

    if (lwip_setsockopt(sock,
                        IPPROTO_TCP,
                        SO_KEEPALIVE,
                        &value,
                        sizeof(value)) < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(errno);
    }

    return NeonTCPIP_OK;
}

NeonTCPIP_Result NeonTCPIP_Socket_SetReuseAddr(NeonTCPIP_SocketHandle sock, bool enable)
{
    if (sock < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    int value = enable ? 1 : 0;

    if (lwip_setsockopt(sock,
                        SOL_SOCKET,
                        SO_REUSEADDR,
                        &value,
                        sizeof(value)) < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(errno);
    }

    return NeonTCPIP_OK;
}

NeonTCPIP_Result NeonTCPIP_Socket_SetSendBlockTime(NeonTCPIP_SocketHandle sock, int sendBlockTime)
{
    if (sock < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    int value = sendBlockTime;

    if (lwip_setsockopt(sock,
                        SOL_SOCKET,
                        SO_SNDTIMEO,
                        &value,
                        sizeof(value)) < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(errno);
    }

    return NeonTCPIP_OK;
}

NeonTCPIP_Result NeonTCPIP_Socket_SetRecvBlockTime(NeonTCPIP_SocketHandle sock, int recvBlockTime)
{
    if (sock < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    int value = recvBlockTime;

    if (lwip_setsockopt(sock,
                        SOL_SOCKET,
                        SO_RCVTIMEO,
                        &value,
                        sizeof(value)) < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(errno);
    }

    return NeonTCPIP_OK;
}

NeonTCPIP_Result NeonTCPIP_Socket_SetLinger(NeonTCPIP_SocketHandle sock, bool onoff, bool linger)
{
    if (sock < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    struct linger so_linger;
    so_linger.l_onoff = onoff ? 1 : 0;
    so_linger.l_linger = linger ? 1 : 0;

    if (lwip_setsockopt(sock,
                        SOL_SOCKET,
                        SO_LINGER,
                        &so_linger,
                        sizeof(so_linger)) < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(errno);
    }

    return NeonTCPIP_OK;
}
                        
int32_t NeonTCPIP_Socket_Select(NeonTCPIP_SocketHandle *read_socks,
                                uint32_t read_count,
                                uint32_t timeout_ms)
{
    if ((read_socks == NULL) || (read_count == 0))
    {
        return NeonTCPIP_InvalidParameter;
    }

    fd_set rfds;
    FD_ZERO(&rfds);

    int max_fd = -1;

    for (uint32_t i = 0; i < read_count; i++)
    {
        if (read_socks[i] >= 0)
        {
            FD_SET(read_socks[i], &rfds);

            if (read_socks[i] > max_fd)
            {
                max_fd = read_socks[i];
            }
        }
    }

    if (max_fd < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int ret = lwip_select(max_fd + 1, &rfds, NULL, NULL, &tv);

    if (ret > 0)
    {
        uint32_t ready_count = 0;

        for (uint32_t i = 0; i < read_count; i++)
        {
            if ((read_socks[i] >= 0) && FD_ISSET(read_socks[i], &rfds))
            {
                ready_count++;
            }
            else
            {
                read_socks[i] = -1;
            }
        }

        return (int32_t)ready_count;
    }

    if (ret == 0)
    {
        return 0;
    }

    return NeonTCPIP_Map_Error(errno);
}
#endif

#ifdef CONFIG_INTERNET_SIMPLELINK

void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    if(!pNetAppEvent)
    {
        return;
    }

    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
            UART_Printf("Assigned IP: %d.%d.%d.%d\n",
              SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,3),
              SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,2),
              SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,1),
              SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,0)
            );
            UART_Printf("Gateway: %d.%d.%d.%d\n",
              SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,3),
              SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,2),
              SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,1),
              SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,0)
            );
        }
        break;
        case SL_NETAPP_IP_LEASED_EVENT:
        {
        }
        break;

        case SL_NETAPP_IP_RELEASED_EVENT:
        {
        }
        break;

        default:
        {
        }
        break;
    }
}

static int32_t NeonTCPIP_Map_Error(int32_t err)
{
    if (err == 0)
    {
        return NeonTCPIP_OK;
    }

    switch (err)
    {
#ifdef SL_EAGAIN
        case SL_EAGAIN:
            return NeonTCPIP_WouldBlock;
#endif

#ifdef SL_EALREADY
        case SL_EALREADY:
            return NeonTCPIP_WouldBlock;
#endif
/*
#ifdef SL_EWOULDBLOCK
        case SL_EWOULDBLOCK:
            return NeonTCPIP_WouldBlock;
#endif
*/
#ifdef SL_ETIMEDOUT
        case SL_ETIMEDOUT:
            return NeonTCPIP_Timeout;
#endif

        default:
            return NeonTCPIP_Error;
    }
}

NeonTCPIP_SocketHandle NeonTCPIP_Socket_Open(bool udp)
{
    int16_t type = udp ? SL_SOCK_DGRAM : SL_SOCK_STREAM;
    int16_t proto = udp ? SL_IPPROTO_UDP : SL_IPPROTO_TCP;

    int16_t s = sl_Socket(SL_AF_INET, type, proto);
    if (s < 0)
    {
        return (NeonTCPIP_SocketHandle)s;
    }

    return (NeonTCPIP_SocketHandle)s;
}

NeonTCPIP_Result NeonTCPIP_Socket_SetNonBlock(NeonTCPIP_SocketHandle sock, bool enable)
{
    if (sock < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    SlSockNonblocking_t nb;
    nb.NonblockingEnabled = enable ? 1 : 0;

    int16_t ret = sl_SetSockOpt((int16_t)sock,
                                SL_SOL_SOCKET,
                                SL_SO_NONBLOCKING,
                                &nb,
                                sizeof(nb));

    if (ret < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(ret);
    }

    return NeonTCPIP_OK;
}

NeonTCPIP_Result NeonTCPIP_Socket_Bind(NeonTCPIP_SocketHandle sock, uint16_t port)
{
    if (sock < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    SlSockAddrIn_t addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = SL_AF_INET;
    addr.sin_port = sl_Htons(port);
    addr.sin_addr.s_addr = 0;

    int16_t ret = sl_Bind((int16_t)sock,
                          (SlSockAddr_t *)&addr,
                          sizeof(addr));

    if (ret < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(ret);
    }

    return NeonTCPIP_OK;
}

NeonTCPIP_Result NeonTCPIP_Socket_Listen(NeonTCPIP_SocketHandle sock, int32_t backlog)
{
    if (sock < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    int16_t ret = sl_Listen((int16_t)sock, (int16_t)backlog);
    if (ret < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(ret);
    }

    return NeonTCPIP_OK;
}

NeonTCPIP_SocketHandle NeonTCPIP_Socket_Accept(NeonTCPIP_SocketHandle sock)
{
    if (sock < 0)
    {
        return -1;
    }

    SlSockAddrIn_t addr;
    SlSocklen_t addr_len = sizeof(addr);

    int16_t client = sl_Accept((int16_t)sock,
                               (SlSockAddr_t *)&addr,
                               &addr_len);

    if (client < 0)
    {
        return (NeonTCPIP_SocketHandle)client;
    }

    return (NeonTCPIP_SocketHandle)client;
}

int32_t NeonTCPIP_Socket_Recv(NeonTCPIP_SocketHandle sock, void *buf, uint32_t len)
{
    if ((sock < 0) || (buf == NULL) || (len == 0))
    {
        return NeonTCPIP_InvalidParameter;
    }

    int16_t ret = sl_Recv((int16_t)sock, buf, (int16_t)len, 0);

    if (ret > 0)
    {
        return ret;
    }

    if (ret == 0)
    {
        return NeonTCPIP_Closed;
    }

    return NeonTCPIP_Map_Error(ret);
}

int32_t NeonTCPIP_Socket_Send(NeonTCPIP_SocketHandle sock, const void *buf, uint32_t len)
{
    if ((sock < 0) || (buf == NULL) || (len == 0))
    {
        return NeonTCPIP_InvalidParameter;
    }

    int16_t ret = sl_Send((int16_t)sock, buf, (int16_t)len, 0);

    if (ret >= 0)
    {
        return ret;
    }

    return NeonTCPIP_Map_Error(ret);
}

int32_t NeonTCPIP_Socket_RecvFrom(NeonTCPIP_SocketHandle sock,
                                  void *buf,
                                  uint32_t len,
                                  NeonTCPIP_SocketAddr *from)
{
    if ((sock < 0) || (buf == NULL) || (len == 0))
    {
        return NeonTCPIP_InvalidParameter;
    }

    SlSockAddrIn_t addr;
    SlSocklen_t addr_len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));

    int16_t ret = sl_RecvFrom((int16_t)sock,
                              buf,
                              (int16_t)len,
                              0,
                              (SlSockAddr_t *)&addr,
                              &addr_len);

    if (ret >= 0)
    {
        if (from != NULL)
        {
            uint32_t ip = sl_Htonl(addr.sin_addr.s_addr);

            from->ip[0] = (uint8_t)((ip >> 24) & 0xFF);
            from->ip[1] = (uint8_t)((ip >> 16) & 0xFF);
            from->ip[2] = (uint8_t)((ip >> 8) & 0xFF);
            from->ip[3] = (uint8_t)(ip & 0xFF);
            from->port = sl_Htons(addr.sin_port);
        }

        return ret;
    }

    return NeonTCPIP_Map_Error(ret);
}

int32_t NeonTCPIP_Socket_SendTo(NeonTCPIP_SocketHandle sock,
                                const void *buf,
                                uint32_t len,
                                const NeonTCPIP_SocketAddr *to)
{
    if ((sock < 0) || (buf == NULL) || (len == 0) || (to == NULL))
    {
        return NeonTCPIP_InvalidParameter;
    }

    SlSockAddrIn_t addr;
    memset(&addr, 0, sizeof(addr));

    uint32_t ip =
        ((uint32_t)to->ip[0] << 24) |
        ((uint32_t)to->ip[1] << 16) |
        ((uint32_t)to->ip[2] << 8) |
        ((uint32_t)to->ip[3]);

    addr.sin_family = SL_AF_INET;
    addr.sin_port = sl_Htons(to->port);
    addr.sin_addr.s_addr = sl_Htonl(ip);

    int16_t ret = sl_SendTo((int16_t)sock,
                            buf,
                            (int16_t)len,
                            0,
                            (SlSockAddr_t *)&addr,
                            sizeof(addr));

    if (ret >= 0)
    {
        return ret;
    }

    return NeonTCPIP_Map_Error(ret);
}

NeonTCPIP_Result NeonTCPIP_Socket_Close(NeonTCPIP_SocketHandle sock)
{
    if (sock < 0)
    {
        return NeonTCPIP_InvalidParameter;
    }

    int16_t ret = sl_Close((int16_t)sock);
    if (ret < 0)
    {
        return (NeonTCPIP_Result)NeonTCPIP_Map_Error(ret);
    }

    return NeonTCPIP_OK;
}

int32_t NeonTCPIP_Socket_GetLastError(void)
{
    return 0;
}

NeonTCPIP_Result NeonTCPIP_Socket_SetTcpNoDelay(NeonTCPIP_SocketHandle sock, bool enable)
{
    (void)sock;
    (void)enable;

    return NeonTCPIP_Unsupported;
}

NeonTCPIP_Result NeonTCPIP_Socket_SetKeepAlive(NeonTCPIP_SocketHandle sock, bool enable)
{
    (void)sock;
    (void)enable;

    return NeonTCPIP_Unsupported;
}

NeonTCPIP_Result NeonTCPIP_Socket_SetReuseAddr(NeonTCPIP_SocketHandle sock, bool enable)
{
    (void)sock;
    (void)enable;

    return NeonTCPIP_Unsupported;
}

NeonTCPIP_Result NeonTCPIP_Socket_SetSendBlockTime(NeonTCPIP_SocketHandle sock, int sendBlockTime)
{
    (void)sock;
    (void)sendBlockTime;

    return NeonTCPIP_Unsupported;
}

NeonTCPIP_Result NeonTCPIP_Socket_SetRecvBlockTime(NeonTCPIP_SocketHandle sock, int recvBlockTime)
{
    (void)sock;
    (void)recvBlockTime;

    return NeonTCPIP_Unsupported;
}

NeonTCPIP_Result NeonTCPIP_Socket_SetLinger(NeonTCPIP_SocketHandle sock, bool onoff, bool linger)
{
    (void)sock;
    (void)onoff;
    (void)linger;

    return NeonTCPIP_Unsupported;
}

int32_t NeonTCPIP_Socket_Select(NeonTCPIP_SocketHandle *read_socks,
                                uint32_t read_count,
                                uint32_t timeout_ms)
{
    (void)read_socks;
    (void)read_count;
    (void)timeout_ms;

    return NeonTCPIP_Unsupported;
}
#endif


void NeonTCPIP_init(const uint8_t *ip, const uint8_t *gw, const uint8_t *netmask)
{
#ifdef CONFIG_INTERNET_LWIP
    NeonTCPIP_ETH_Init(ip, gw, netmask);
#endif
#ifdef CONFIG_INTERNET_SIMPLELINK
    long lRetVal = -1;
    
    lRetVal = sl_WlanSetMode(ROLE_STA);
    if(lRetVal < 0)
    {
        UART_Printf("Failed to set WLAN mode\n");
        return;
    }

    lRetVal = sl_Stop(0xFF);
    if(lRetVal < 0)
    {
        UART_Printf("Failed to stop SimpleLink\n");
        return;
    }

    lRetVal = sl_Start(0, 0, 0);
    if(lRetVal < 0)
    {
        UART_Printf("Failed to start SimpleLink\n");
        return;
    }
#endif
}
