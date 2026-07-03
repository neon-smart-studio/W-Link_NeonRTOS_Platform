
#ifndef __NEONTCPIP__H__
#define	__NEONTCPIP__H__

#include <stdint.h>

#include "TCPIP_Config.h"

#ifdef CONFIG_INTERNET_LWIP
#include "lwipopts.h"
#endif

#ifdef CONFIG_INTERNET_LWIP
/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0   (uint8_t) 192
#define IP_ADDR1   (uint8_t) 168
#define IP_ADDR2   (uint8_t) 84
#define IP_ADDR3   (uint8_t) 100

/*NETMASK*/
#define NETMASK_ADDR0   (uint8_t) 255
#define NETMASK_ADDR1   (uint8_t) 255
#define NETMASK_ADDR2   (uint8_t) 255
#define NETMASK_ADDR3   (uint8_t) 0

/*Gateway Address*/
#define GW_ADDR0   (uint8_t) 192
#define GW_ADDR1   (uint8_t) 168
#define GW_ADDR2   (uint8_t) 84
#define GW_ADDR3   (uint8_t) 1

/* DHCP process states */
#define DHCP_OFF                   (uint8_t) 0
#define DHCP_START                 (uint8_t) 1
#define DHCP_WAIT_ADDRESS          (uint8_t) 2
#define DHCP_ADDRESS_ASSIGNED      (uint8_t) 3
#define DHCP_TIMEOUT               (uint8_t) 4
#define DHCP_LINK_DOWN             (uint8_t) 5
#define DHCP_ASK_RELEASE           (uint8_t) 6
#endif //CONFIG_INTERNET_LWIP

typedef int32_t NeonTCPIP_SocketHandle;

typedef enum
{
    NeonTCPIP_OK = 0,
    NeonTCPIP_Error = -1,
    NeonTCPIP_WouldBlock = -2,
    NeonTCPIP_Closed = -3,
    NeonTCPIP_Unsupported = -4,
    NeonTCPIP_InvalidParameter = -5,
    NeonTCPIP_Timeout = -6
} NeonTCPIP_Result;

typedef struct
{
    uint8_t  ip[4];
    uint16_t port;
} NeonTCPIP_SocketAddr;

#ifdef	__cplusplus
extern "C" {
#endif

void NeonTCPIP_init(const uint8_t *ip, const uint8_t *gw, const uint8_t *netmask);

uint8_t NeonTCPIP_IF_isInit(void);
uint8_t NeonTCPIP_IF_isLinkUp(void);

void NeonTCPIP_IF_Update_Addresses(uint32_t ip, uint32_t netmask, uint32_t gw);
uint32_t NeonTCPIP_IF_Get_IP_Address(void);
void NeonTCPIP_IF_Set_IP_Address(uint32_t ip_addr);
uint32_t NeonTCPIP_IF_Get_Gateway_Address(void);
void NeonTCPIP_IF_Set_Gateway_Address(uint32_t gateway);
uint32_t NeonTCPIP_IF_Get_NetMask_Address(void);
void NeonTCPIP_IF_Set_NetMask_Address(uint32_t net_mask);
void NeonTCPIP_IF_Get_Mac_Address(uint8_t mac[6]);
#if LWIP_DHCP
uint32_t NeonTCPIP_IF_Get_DHCP_Address(void);
void NeonTCPIP_DHCP_Enable(void);
void NeonTCPIP_DHCP_Disable(void);
bool NeonTCPIP_DHCP_IsEnabled(void);
#endif

uint32_t NeonTCPIP_IF_Get_DNS_Address(void);
void NeonTCPIP_IF_Set_DNS_Address(const uint8_t *dnsaddr);
int8_t NeonTCPIP_DNS_GetHostByName(const char *hostname, uint32_t *ipaddr);

uint32_t ip_string_to_u32(const char *ip_str);
void ip_u32_to_string(uint32_t ip_u32, char *buf, size_t buflen);


NeonTCPIP_SocketHandle NeonTCPIP_Socket_Open(bool udp);

NeonTCPIP_Result NeonTCPIP_Socket_SetNonBlock(NeonTCPIP_SocketHandle sock, bool enable);
NeonTCPIP_Result NeonTCPIP_Socket_Bind(NeonTCPIP_SocketHandle sock, uint16_t port);
NeonTCPIP_Result NeonTCPIP_Socket_Listen(NeonTCPIP_SocketHandle sock, int32_t backlog);
NeonTCPIP_SocketHandle NeonTCPIP_Socket_Accept(NeonTCPIP_SocketHandle sock);

int32_t NeonTCPIP_Socket_Recv(NeonTCPIP_SocketHandle sock, void *buf, uint32_t len);
int32_t NeonTCPIP_Socket_Send(NeonTCPIP_SocketHandle sock, const void *buf, uint32_t len);

int32_t NeonTCPIP_Socket_RecvFrom(NeonTCPIP_SocketHandle sock,
                                  void *buf,
                                  uint32_t len,
                                  NeonTCPIP_SocketAddr *from);

int32_t NeonTCPIP_Socket_SendTo(NeonTCPIP_SocketHandle sock,
                                const void *buf,
                                uint32_t len,
                                const NeonTCPIP_SocketAddr *to);

NeonTCPIP_Result NeonTCPIP_Socket_Close(NeonTCPIP_SocketHandle sock);
int32_t NeonTCPIP_Socket_GetLastError(void);

NeonTCPIP_Result NeonTCPIP_Socket_SetTcpNoDelay(NeonTCPIP_SocketHandle sock, bool enable);
NeonTCPIP_Result NeonTCPIP_Socket_SetKeepAlive(NeonTCPIP_SocketHandle sock, bool enable);
NeonTCPIP_Result NeonTCPIP_Socket_SetReuseAddr(NeonTCPIP_SocketHandle sock, bool enable);
NeonTCPIP_Result NeonTCPIP_Socket_SetSendBlockTime(NeonTCPIP_SocketHandle sock, int sendBlockTime);
NeonTCPIP_Result NeonTCPIP_Socket_SetRecvBlockTime(NeonTCPIP_SocketHandle sock, int recvBlockTime);
NeonTCPIP_Result NeonTCPIP_Socket_SetLinger(NeonTCPIP_SocketHandle sock, bool onoff, bool linger);

int32_t NeonTCPIP_Socket_Select(NeonTCPIP_SocketHandle *read_socks,
                                uint32_t read_count,
                                uint32_t timeout_ms);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif
