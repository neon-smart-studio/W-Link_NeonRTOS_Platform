
#include <stdbool.h>
#include <stdint.h>

#include "time.h"
#include "string.h"

#include "lwip/opt.h"

#include "lwip/ip_addr.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "lwip/igmp.h"
#include "lwip/sockets.h"

#include "SNTP.h"

#include "NeonRTOS.h"

#define SNTP_TASK_STACK_SIZE    2048
#define SNTP_TASK_PRIORITY      4

#define SNTP_NUM_SERVERS_SUPPORTED 5
/*
#define SNTP_SERVERS 	"tick.stdtime.gov.tw", "tock.stdtime.gov.tw", \
						"time.stdtime.gov.tw", "clock.stdtime.gov.tw", \
						"watch.stdtime.gov.tw"
*/

#define SNTP_SERVER_URL_LEN    30

const char SNTP_Server_URL_List[SNTP_NUM_SERVERS_SUPPORTED][SNTP_SERVER_URL_LEN] = {
	"tick.stdtime.gov.tw",
	"tock.stdtime.gov.tw",
	"time.stdtime.gov.tw",
	"clock.stdtime.gov.tw",
	"watch.stdtime.gov.tw"
};

#define UNUSED_ARG(x)	(void)x

bool SNTP_Updated_Time = false;
int sntp_socket_ID;
bool SNTP_Found_Server = false;

ip_addr_t sntp_server_address;

NeonRTOS_MsgQ_t DnsFoundIp_Queue_Handle;

/** SNTP server port */
#ifndef SNTP_PORT
#define SNTP_PORT                   123
#endif

/** Set this to 1 to allow SNTP_SERVER_ADDRESS to be a DNS name */
#ifndef SNTP_SERVER_DNS
#define SNTP_SERVER_DNS             1
#endif

/** Set to the number of servers supported (at least 1) */
#ifndef SNTP_NUM_SERVERS_SUPPORTED
#define SNTP_NUM_SERVERS_SUPPORTED	1
#endif

#ifndef SNTP_CHECK_RESPONSE
#define SNTP_CHECK_RESPONSE         0
#endif

#ifndef SNTP_RECV_TIMEOUT
#define SNTP_RECV_TIMEOUT           3000
#endif

#ifndef SNTP_UPDATE_DELAY
#define SNTP_UPDATE_DELAY           3600000
//#define SNTP_UPDATE_DELAY           15000
#endif
#if (SNTP_UPDATE_DELAY < 15000) && !SNTP_SUPPRESS_DELAY_CHECK
#error "SNTPv4 RFC 4330 enforces a minimum update time of 15 seconds!"
#endif

/** SNTP macro to change system time and/or the update the RTC clock */
#ifndef SNTP_SET_SYSTEM_TIME
#define SNTP_SET_SYSTEM_TIME(sec) ((void)sec)
#endif

/** SNTP macro to change system time including microseconds */
#ifdef SNTP_SET_SYSTEM_TIME_US
#define SNTP_CALC_TIME_US           1
#define SNTP_RECEIVE_TIME_SIZE      2
#else
#define SNTP_CALC_TIME_US           1
#define SNTP_RECEIVE_TIME_SIZE      1
#endif

#ifndef SNTP_GET_SYSTEM_TIME
#define SNTP_GET_SYSTEM_TIME(sec, us)     do { (sec) = 0; (us) = 0; } while(0)
#endif

#ifndef SNTP_RETRY_TIMEOUT
#define SNTP_RETRY_TIMEOUT          SNTP_RECV_TIMEOUT
#endif

/** Maximum retry timeout (in milliseconds). */
#ifndef SNTP_RETRY_TIMEOUT_MAX
#define SNTP_RETRY_TIMEOUT_MAX      (SNTP_RETRY_TIMEOUT * 10)
#endif

/** Increase retry timeout with every retry sent
 * Default is on to conform to RFC.
 */
#ifndef SNTP_RETRY_TIMEOUT_EXP
#define SNTP_RETRY_TIMEOUT_EXP      1
#endif

/* the various debug levels for this file */
#define SNTP_DEBUG_TRACE        (SNTP_DEBUG | LWIP_DBG_TRACE)
#define SNTP_DEBUG_STATE        (SNTP_DEBUG | LWIP_DBG_STATE)
#define SNTP_DEBUG_WARN         (SNTP_DEBUG | LWIP_DBG_LEVEL_WARNING)
#define SNTP_DEBUG_WARN_STATE   (SNTP_DEBUG | LWIP_DBG_LEVEL_WARNING | LWIP_DBG_STATE)
#define SNTP_DEBUG_SERIOUS      (SNTP_DEBUG | LWIP_DBG_LEVEL_SERIOUS)

#define SNTP_ERR_KOD                1

/* SNTP protocol defines */
#define SNTP_MSG_LEN                48

#define SNTP_OFFSET_LI_VN_MODE      0
#define SNTP_LI_MASK                0xC0
#define SNTP_LI_NO_WARNING          0x00
#define SNTP_LI_LAST_MINUTE_61_SEC  0x01
#define SNTP_LI_LAST_MINUTE_59_SEC  0x02
#define SNTP_LI_ALARM_CONDITION     0x03 /* (clock not synchronized) */

#define SNTP_VERSION_MASK           0x38
#define SNTP_VERSION                (4/* NTP Version 4*/<<3) 

#define SNTP_MODE_MASK              0x07
#define SNTP_MODE_CLIENT            0x03
#define SNTP_MODE_SERVER            0x04
#define SNTP_MODE_BROADCAST         0x05

#define SNTP_OFFSET_STRATUM         1
#define SNTP_STRATUM_KOD            0x00

#define SNTP_OFFSET_ORIGINATE_TIME  24
#define SNTP_OFFSET_RECEIVE_TIME    32
#define SNTP_OFFSET_TRANSMIT_TIME   40

/* number of seconds between 1900 and 1970 */
#define DIFF_SEC_1900_1970         (2208988800UL)

#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
	PACK_STRUCT_BEGIN
	struct sntp_msg {
		PACK_STRUCT_FIELD(u8_t           li_vn_mode);
		PACK_STRUCT_FIELD(u8_t           stratum);
		PACK_STRUCT_FIELD(u8_t           poll);
		PACK_STRUCT_FIELD(u8_t           precision);
		PACK_STRUCT_FIELD(u32_t          root_delay);
		PACK_STRUCT_FIELD(u32_t          root_dispersion);
		PACK_STRUCT_FIELD(u32_t          reference_identifier);
		PACK_STRUCT_FIELD(u32_t          reference_timestamp[2]);
		PACK_STRUCT_FIELD(u32_t          originate_timestamp[2]);
		PACK_STRUCT_FIELD(u32_t          receive_timestamp[2]);
		PACK_STRUCT_FIELD(u32_t          transmit_timestamp[2]);
	} PACK_STRUCT_STRUCT;
	PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif

	/** Addresses of servers */
	static char* sntp_server_addresses[SNTP_NUM_SERVERS_SUPPORTED];
	/** The currently used server (initialized to 0) */
	static u8_t sntp_num_servers;
#if (SNTP_NUM_SERVERS_SUPPORTED > 1)
	static u8_t sntp_current_server;
#else
#define sntp_current_server 0
#endif /* SNTP_NUM_SERVERS_SUPPORTED */

#if SNTP_RETRY_TIMEOUT_EXP
#define SNTP_RESET_RETRY_TIMEOUT() sntp_retry_timeout = SNTP_RETRY_TIMEOUT
	/** Retry time, initialized with SNTP_RETRY_TIMEOUT and doubled with each retry. */
	static u32_t sntp_retry_timeout;
#else /* SNTP_RETRY_TIMEOUT_EXP */
#define SNTP_RESET_RETRY_TIMEOUT()
#define sntp_retry_timeout SNTP_RETRY_TIMEOUT
#endif /* SNTP_RETRY_TIMEOUT_EXP */

#if SNTP_CHECK_RESPONSE >= 1
	/** Saves the last server address to compare with response */
	static ip_addr_t sntp_last_server_address;
#endif /* SNTP_CHECK_RESPONSE >= 1 */

#if SNTP_CHECK_RESPONSE >= 2
	/** Saves the last timestamp sent (which is sent back by the server)
	 * to compare against in response */
	static u32_t sntp_last_timestamp_sent[2];
#endif /* SNTP_CHECK_RESPONSE >= 2 */

#if (SNTP_NUM_SERVERS_SUPPORTED > 1)
#define sntp_try_next_server    sntp_retry
/**
 * If Kiss-of-Death is received (or another packet parsing error),
 * try the next server or retry the current server and increase the retry
 * timeout if only one server is available.
 *
 * @param arg is unused (only necessary to conform to sys_timeout)
 */
static void
sntp_try_next_server(void* arg)
{
	LWIP_UNUSED_ARG(arg);

	if (sntp_num_servers > 1) {
		/* new server: reset retry timeout */
		SNTP_RESET_RETRY_TIMEOUT();
		sntp_current_server++;
		if (sntp_current_server >= sntp_num_servers) {
			sntp_current_server = 0;
		}
		//printf("sntp_try_next_server: Sending request to server %"U16_F"\n",(u16_t)sntp_current_server);
		/* instantly send a request to the next server */
	}
}

#endif

void SNTP_Update_Time(time_t t, uint32_t us)
{
	Time_Date current_date_time;
        
	Time_T_Sec_To_Time_Date(t + (int32_t)(Get_Current_Time_Zone() * 60 * 60), &current_date_time);
	
	Set_Current_Time_Date(&current_date_time);
}

#if SNTP_SERVER_DNS
static void
sntp_dns_found(const char* hostname, ip_addr_t *ipaddr, void *arg)
{
	LWIP_UNUSED_ARG(hostname);
	LWIP_UNUSED_ARG(arg);

	if (ipaddr != NULL) {
		/* Address resolved, send request */
		//printf("Sntp Dns Found: Server address \"%x\" has Resolved, so Sending Request\n", ipaddr->addr);
		NeonRTOS_MsgQWrite(&DnsFoundIp_Queue_Handle, ipaddr, NEONRT_NO_WAIT);
	}
	else {
		/* DNS resolving failed -> try another server */
		//printf("Sntp Dns Found: Failed to Resolve Server Aaddress, Trying Next Server\n");
		sntp_try_next_server(NULL);
	}
}
#endif

int SNTP_Set_Servers(char *server_url [], int num_servers)
{
	int i;

	if (SNTP_NUM_SERVERS_SUPPORTED < num_servers) return -1;

	for (i = sntp_num_servers - 1; i >= 0; i--) {
		free(sntp_server_addresses[i]);
		sntp_server_addresses[i] = NULL;
	}

	for (i = 0; i < num_servers; i++) {
		sntp_server_addresses[i] = malloc(strlen(server_url[i]));
		if (sntp_server_addresses[i]) {
			strcpy(sntp_server_addresses[i], server_url[i]);
		}
		else {
			sntp_num_servers = i;
			return -2;
		}
	}
	sntp_num_servers = num_servers;
	return 0;
}

int SNTP_Set_Servers_Default()
{
	int i;

	for (i = sntp_num_servers - 1; i >= 0; i--) {
		free(sntp_server_addresses[i]);
		sntp_server_addresses[i] = NULL;
	}

	for (i = 0; i < SNTP_NUM_SERVERS_SUPPORTED; i++) {
		//printf("Print Server Names\n");
		sntp_server_addresses[i] = malloc(strlen_P(SNTP_Server_URL_List[i]));
		if (sntp_server_addresses[i]) {
			strcpy_P(sntp_server_addresses[i], SNTP_Server_URL_List[i]);
		}
		else {
			sntp_num_servers = i;
			return -2;
		}
	}
	sntp_num_servers = SNTP_NUM_SERVERS_SUPPORTED;
	return 0;
}

bool Is_SNTP_Has_Updated_Time()
{
        return SNTP_Updated_Time;
}

void SNTP_Task(void *pvParameters)
{
	int ret;
	Time_Date current_date_time;
	 
        while(1)
        {
                SNTP_Updated_Time = false;
                
                Get_Current_Time_Date(&current_date_time);
                SNTP_Set_Servers_Default();
                
                struct sockaddr_in node;
                memset(&node, 0, sizeof(node));
                node.sin_family      = AF_INET;
                node.sin_port        = PP_HTONS(INADDR_ANY);
                node.sin_addr.s_addr = PP_HTONL(INADDR_ANY);

                sntp_socket_ID = socket(AF_INET, SOCK_DGRAM, 0);
                if (sntp_socket_ID < 0)
                {
                        sntp_socket_ID = -1;
                        continue;
                }

                if (bind(sntp_socket_ID, (struct sockaddr *)&node, sizeof(node)) < 0)
                {
                        close(sntp_socket_ID);
                        sntp_socket_ID = -1;
                        continue;
                }

                int timeout = SNTP_RECV_TIMEOUT;
                setsockopt(sntp_socket_ID, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
                
                err_t err;

                //printf("SNTP_Task remain %d words, heap %d bytes\n", (int)uxTaskGetStackHighWaterMark(NULL), system_get_free_heap_size());
                        //printf("DNS Start to Resolve SNTP Server URL\n");
                if (!SNTP_Found_Server)
                {
                        do
                        {
#if SNTP_SERVER_DNS
                                err = dns_gethostbyname(sntp_server_addresses[sntp_current_server],
                                        &sntp_server_address,
                                        sntp_dns_found,
                                        NULL);
                                if (err == ERR_INPROGRESS)
                                {
                                        if (NeonRTOS_MsgQRead(&DnsFoundIp_Queue_Handle, &sntp_server_address, 5000)==NeonRTOS_OK)
                                        {
                                                SNTP_Found_Server = true;
                                                break;
                                        }
                                        else
                                        {
                                                //printf("sntp_request: Invalid server address, trying next server.\n");
                                                sys_timeout((u32_t)SNTP_RETRY_TIMEOUT, sntp_try_next_server, NULL);
                                        }
                                }
                                else
                                {
                                        sys_timeout((u32_t)SNTP_RETRY_TIMEOUT, sntp_try_next_server, NULL);
                                        NeonRTOS_Sleep(SNTP_RETRY_TIMEOUT);
                                }
#else 
                                if (ipaddr_aton(sntp_server_addresses[sntp_current_server], &sntp_server_address)) {
                                        break;
                                }
                                else
                                {
                                        //printf("sntp_request: Invalid server address, trying next server.\n");
                                        sys_timeout((u32_t)SNTP_RETRY_TIMEOUT, sntp_try_next_server, NULL);
                                        NeonRTOS_Sleep(SNTP_RETRY_TIMEOUT);
                                }
#endif 
                
                        } while (1);
                }

                if (SNTP_Found_Server)
                {
                        struct sockaddr_in server;

                        memset(&server, 0, sizeof(server));
                        server.sin_family      = AF_INET;
                        server.sin_port        = PP_HTONS(SNTP_PORT);
                    
                        struct sntp_msg    sntpmsg;
                        socklen_t msg_len;
                        
                        //inet_addr_from_ipaddr(&server.sin_addr, &sntp_server_address);
                        server.sin_addr.s_addr = ip_2_ip4(&sntp_server_address)->addr;    
                        
                        int status;
                        
                        memset(&sntpmsg, 0, SNTP_MSG_LEN);
                        sntpmsg.li_vn_mode = SNTP_LI_NO_WARNING | SNTP_VERSION | SNTP_MODE_CLIENT;
                        
#if SNTP_CHECK_RESPONSE >= 2
                        {
                                u32_t sntp_time_sec, sntp_time_us;
                                SNTP_GET_SYSTEM_TIME(sntp_time_sec, sntp_time_us);
                                sntp_last_timestamp_sent[0] = htonl(sntp_time_sec + DIFF_SEC_1900_1970);
                                req->transmit_timestamp[0] = sntp_last_timestamp_sent[0];
                                sntp_last_timestamp_sent[1] = htonl(sntp_time_us);
                                req->transmit_timestamp[1] = sntp_last_timestamp_sent[1];
                        }
#endif

                        if (sendto(sntp_socket_ID, &sntpmsg, SNTP_MSG_LEN, 0, (struct sockaddr *)&server, sizeof(server)) >= 0) {
                                msg_len = sizeof(server);
                                status = recvfrom(sntp_socket_ID, &sntpmsg, SNTP_MSG_LEN, 0, (struct sockaddr *)&server, (socklen_t *)&msg_len);
                                        
                                if (status < 0)
                                {
                                        //printf("SNTP Cannot Receive Data From Server, Error Code: %d\n", errno);
                                        //printf("Try Next Server\n");
                                        SNTP_Found_Server = false;
                                        sntp_try_next_server(NULL);
                                        continue;
                                }
                                        
                                msg_len = status;
                                //printf("sntp_request: receive len==%d\n", msg_len);
                                        
                                if (msg_len == SNTP_MSG_LEN) {
                                        if (((sntpmsg.li_vn_mode & SNTP_MODE_MASK) == SNTP_MODE_SERVER) ||
                                                ((sntpmsg.li_vn_mode & SNTP_MODE_MASK) == SNTP_MODE_BROADCAST)) {

                                                time_t t = (ntohl(sntpmsg.receive_timestamp[0]) - DIFF_SEC_1900_1970);

#if SNTP_CALC_TIME_US
                                                u32_t us = ntohl(sntpmsg.receive_timestamp[1]) / 4295;
                                                SNTP_Update_Time(t, us);
#else
                                                SNTP_Update_Time(t, 0);
#endif
                                                
                                                SNTP_Updated_Time = true;
                                                
                                        }
                                }
                        }
                        
                        NeonRTOS_Sleep(SNTP_UPDATE_INTERVAL);
                }
                
                close(sntp_socket_ID);
        }
        
	NeonRTOS_TaskDelete(NULL);
}

int SNTP_Init(void)
{
	SNTP_Found_Server = false;
        SNTP_Updated_Time = false;
	sntp_socket_ID = -1;
	int status;
	 
#if SNTP_SERVER_DNS
        if(NeonRTOS_MsgQCreate(&DnsFoundIp_Queue_Handle, "SNTP DNS", sizeof(ip_addr_t), 1)!=NeonRTOS_OK)
        {
                return -1;
        }
#endif
	if(NeonRTOS_TaskCreate(SNTP_Task, (signed char *)"SNTP", SNTP_TASK_STACK_SIZE, NULL, SNTP_TASK_PRIORITY, NULL)!=NeonRTOS_OK){return -1;}
        
        return 0;
}