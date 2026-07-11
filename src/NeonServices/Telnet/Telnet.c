/*
ESP8266 Simple Service Discovery
Copyright (c) 2015 Hristo Gochkov

Original (Arduino) version by Filippo Sallemi, July 23, 2014.
Can be found at: https://github.com/nomadnt/uTELNET

License (MIT license):
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*/

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "lwip/opt.h"
#include "lwip/ip_addr.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "lwip/igmp.h"
#include "lwip/sockets.h"

#include "Utils/cJSON/cJSON.h"

#include "Telnet.h"

#include "NeonRTOS.h"

#define TELNET_TASK_STACK_SIZE    2*1024

#define TELNET_TASK_PRIORITY      3

#define max(a,b) ((a)>(b)?(a):(b))  /**< Find the maximum of 2 numbers. */

Telnet_Server_On_Client_Connect_CB Telnetd_On_Client_Connect_CallBack = NULL;
Telnet_Server_On_Client_Disconnect_CB Telnetd_On_Client_Disconnect_CallBack = NULL;
Telnet_Server_On_Client_Message_CB Telnetd_On_Client_Message_CallBack = NULL;

Telnet_Client_On_Connect_Server_CB Telnetc_On_Connect_Server_CallBack = NULL;
Telnet_Client_On_Disconnect_Server_CB Telnetc_On_Disconnect_Server_CallBack = NULL;
Telnet_Client_On_Server_Message_CB Telnetc_On_Server_Message_CallBack = NULL;

typedef struct Telnetd_Client_Connection
{
	int socket_id;
	struct sockaddr client_socket_addr;
	bool client_destruct_flag;
	bool connection_time_unlimit;
	NeonRTOS_TimerHandle connection_timeout_timer;
	bool connection_timeout_flag;
	bool log_print_terminal;
}Telnetd_Client_Connection;

typedef struct Telnetc_Server_Session
{
	int conn_socket_id;
        bool connected_to_server;
	bool connection_timeout_flag;
	NeonRTOS_TimerHandle ping_server_interval_timer;
	NeonRTOS_TimerHandle ping_server_timeout_timer;
        NeonRTOS_MsgQ_t conn_sockaddr_QueueHandle;
        NeonRTOS_SyncObj_t client_conn_SyncHandle;
        NeonRTOS_SyncObj_t client_disconn_SyncHandle;
	struct sockaddr server_socket_addr;
}Telnetc_Server_Session;

const char Telnet_Ping_Command[] = "ping";
const char Telnet_Pong_Command[] = "pong";
const char Telnet_Exit_Command[] = "exit";
const char Telnet_Log_Print_Command[] = "log_print";

Telnetd_Client_Connection Telnetd_Client_List[TELNET_SERVER_MAX_CLIENTS];
Telnetc_Server_Session Telnetc_Conn_Server_Session;

uint16_t Size_Of_Telnet_Server_Client_Connection_Obj(){return sizeof(Telnetd_Client_Connection);}
uint16_t Size_Of_Telnet_Client_Conn_Server_Session_Obj(){return sizeof(Telnetc_Server_Session);}

int Telnet_Server_Socket = -1;

void Telnetd_Connection_Timeout_CB(NeonRTOS_TimerHandle connection_timeout_timer_handle)
{
        uint32_t TimerID;//use timer ID to identify client connection
	NeonRTOS_TimerStop(&connection_timeout_timer_handle);
        if(NeonRTOS_GetTimerID(&connection_timeout_timer_handle, &TimerID)==NeonRTOS_OK)
        {
              TimerID-=TELNETD_TIMER_ID_OFFSET;
              Telnetd_Client_List[TimerID].connection_timeout_flag = true;
        }
}

void Telnetd_Restore_Variables(uint8_t client_index)
{
	Telnetd_Client_List[client_index].socket_id = -1;
	Telnetd_Client_List[client_index].client_destruct_flag = false;
	Telnetd_Client_List[client_index].connection_time_unlimit = false;
	Telnetd_Client_List[client_index].connection_timeout_flag = false;
}

uint8_t GetNumOfTelnetdClient()
{
	uint8_t current_telnet_client = 0;
	for (uint8_t k = 0;k < TELNET_SERVER_MAX_CLIENTS; k++)
	{
		if (Telnetd_Client_List[k].socket_id < 0)
		{
			continue;
		}
		current_telnet_client++;
	}
	return current_telnet_client;
}

int FindTelnetdClient(int telnetd_client_sockID)
{
	for (uint8_t k = 0;k < TELNET_SERVER_MAX_CLIENTS; k++)
	{
		if (Telnetd_Client_List[k].socket_id == telnetd_client_sockID)
		{
			return k;
		}
	}
	return -1;
}

int AddTelnetdClient(int telnetd_client_sockID, struct sockaddr* telnetd_client_sockaddr)
{
	uint8_t j = 0;
	
	if (FindTelnetdClient(telnetd_client_sockID) >= 0)
	{
		return 0;
	}
	if (GetNumOfTelnetdClient() < TELNET_SERVER_MAX_CLIENTS)
	{
		while (j < TELNET_SERVER_MAX_CLIENTS)
		{
			if (Telnetd_Client_List[j].socket_id < 0)
			{
				Telnetd_Restore_Variables(j);
				
                                if(NeonRTOS_TimerCreate(&Telnetd_Client_List[j].connection_timeout_timer, "Connection Timeout", TELNET_SERVER_CONN_TIMEOUT, 1, j+TELNETD_TIMER_ID_OFFSET, Telnetd_Connection_Timeout_CB)!=NeonRTOS_OK)
                                {
					return -1;
                                }
                                NeonRTOS_TimerStop(&Telnetd_Client_List[j].connection_timeout_timer);
                                
				Telnetd_Client_List[j].socket_id = telnetd_client_sockID;
				memcpy(&Telnetd_Client_List[j].client_socket_addr, telnetd_client_sockaddr, sizeof(struct sockaddr));
				Telnetd_Client_List[j].connection_timeout_flag = false;
				Telnetd_Client_List[j].log_print_terminal = false;
                                
				return j;
			}
			j++;
		}
		return -1;
	}
	else
	{
		return -1;
	}
}

int RemoveTelnetdClient(int telnetd_client_sockID)
{
	int result = FindTelnetdClient(telnetd_client_sockID);
	if (result < 0)
	{
                return -1;
	}
        
        Telnetd_Client_List[result].socket_id = -1;
        
        if (Telnetd_Client_List[result].connection_timeout_timer != NULL)
        {
                NeonRTOS_TimerStop(&Telnetd_Client_List[result].connection_timeout_timer);
                NeonRTOS_TimerDelete(&Telnetd_Client_List[result].connection_timeout_timer);
                Telnetd_Client_List[result].connection_timeout_timer = NULL;
        }
        
        return 0;
}

int Telnet_Server_Get_Client_Addr(Telnetd_Client_Connection* conn_client, ip_addr_t* client_ip)
{
        if(conn_client==NULL || client_ip==NULL)
	{
		return -1;
	}
        
        struct sockaddr_in* client_sockaddr_in = (struct sockaddr_in*)&conn_client->client_socket_addr;
      
        client_ip->addr = htonl(client_sockaddr_in->sin_addr.s_addr);
      
        return 0;
}

Telnetd_Client_Connection* Telnet_Server_Get_Client_Conn_Session_By_Addr(ip_addr_t* client_ip)
{
        if(client_ip==NULL)
	{
		return NULL;
	}
        
        Telnetd_Client_Connection* conn_session = NULL;
        struct sockaddr_in* client_sockaddr_in = NULL;
        
        for(uint8_t i = 0; i<TELNET_SERVER_MAX_CLIENTS; i++)
        {
                client_sockaddr_in = (struct sockaddr_in*)&Telnetd_Client_List[i].client_socket_addr;
                
                if(client_ip->addr == htonl(client_sockaddr_in->sin_addr.s_addr))
                {
                      conn_session = &Telnetd_Client_List[i];
                      break;
                }
        }
        
        return conn_session;
}

void Telnet_Server_Register_OnClientConnectCallback(Telnet_Server_On_Client_Connect_CB cb)
{
	Telnetd_On_Client_Connect_CallBack = cb;
}

void Telnet_Server_Register_OnClientDisconnectCallback(Telnet_Server_On_Client_Disconnect_CB cb)
{
	Telnetd_On_Client_Disconnect_CallBack = cb;
}

void Telnet_Server_Register_OnClientMessageCallback(Telnet_Server_On_Client_Message_CB cb)
{
	Telnetd_On_Client_Message_CallBack = cb;
}

void Telnet_Server_Send_Msg(Telnetd_Client_Connection* conn_client, uint8_t* dat, uint16_t len, bool send_new_line)
{
    if(dat==NULL || len==0){return;}
    
    if(conn_client->socket_id<0){return;}
    
    send(conn_client->socket_id, dat, len, 0);
    
    if(send_new_line){
          send(conn_client->socket_id, "\r\n", strlen("\r\n"), 0);
    }
}

void Telnet_Server_Print_Log_To_Clients(uint8_t* dat, uint16_t len)
{
    if(dat==NULL || len==0){return;}
    
    for(uint8_t i = 0; i<TELNET_SERVER_MAX_CLIENTS; i++)
    {
          if(Telnetd_Client_List[i].socket_id<0){continue;}
          if(Telnetd_Client_List[i].log_print_terminal)
          {
                Telnet_Server_Send_Msg(&Telnetd_Client_List[i], dat, len, false);
          }
    }
    
}

void Telnet_Server_Send_JSON_Msg(Telnetd_Client_Connection* conn_client, cJSON* dat, bool need_delete_json) 
{
      if (dat == NULL){return;}
        
	char* json_str = cJSON_Print(dat);
        
      cJSON_Minify(json_str);
        
	if (need_delete_json == true)
	{
		cJSON_Delete(dat);
	}
        
	Telnet_Server_Send_Msg(conn_client, (uint8_t*)json_str, strlen(json_str), true);
        
	mem_Free(json_str);
}

void Telnet_Server_Remove_Client(Telnetd_Client_Connection* conn_client)
{
	Telnet_Server_Send_Msg(conn_client, (uint8_t*)Telnet_Exit_Command, strlen(Telnet_Exit_Command), true);
        
      conn_client->client_destruct_flag = true;
}

void Telnet_Server_Client_Conn_Time_Unlimit_Enable(Telnetd_Client_Connection* conn_client)
{
      conn_client->connection_time_unlimit = true;
	NeonRTOS_TimerStop(&conn_client->connection_timeout_timer);
}

void Telnet_Server_Client_Conn_Time_Unlimit_Disable(Telnetd_Client_Connection* conn_client)
{
      conn_client->connection_time_unlimit = false;
	NeonRTOS_TimerReStart(&conn_client->connection_timeout_timer);
}

void Telnet_Server_Task(void* param)
{
      int32_t ret;
      uint8_t i;
      bool add_Telnet_client;
      
      //struct timeval timeout;
      struct sockaddr_in server_addr;
      int current_telnetd_client_socketID;
      struct sockaddr current_telnetd_client_socket_addr;
      socklen_t sin_size = sizeof(current_telnetd_client_socket_addr);
      
      /* Construct local address structure */
      memset(&server_addr, 0, sizeof(server_addr)); /* Zero out structure */
      server_addr.sin_family = AF_INET;            /* Internet address family */
      server_addr.sin_addr.s_addr = INADDR_ANY;   /* Any incoming interface */
      
      int socket_errno;
      const u32_t socket_errno_optlen = sizeof(socket_errno);
      int recvBlockTime = TELNETD_RECV_BLOCK_INTERVAL;
      server_addr.sin_len = sizeof(struct sockaddr_in);

      server_addr.sin_port = htons(TELNET_PORT); /* Local port */
      
      if(Telnet_Server_Socket>=0)
      {
            NeonRTOS_TaskDelete(NULL);
            return;
      }
    
      /* Create socket for incoming connections */
      Telnet_Server_Socket = socket(AF_INET, SOCK_STREAM, 0);
      if (Telnet_Server_Socket < 0)
      {
            close(Telnet_Server_Socket);
            NeonRTOS_TaskDelete(NULL);
            return;
      }

        /* Bind to the local port */
      ret = bind(Telnet_Server_Socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
      if (ret < 0)
      {
            close(Telnet_Server_Socket);
            NeonRTOS_TaskDelete(NULL);
            return;
      }

      /* Listen to the local connection */
      ret = listen(Telnet_Server_Socket, TELNET_SERVER_MAX_CLIENTS);
      if (ret < 0)
      {
            close(Telnet_Server_Socket);
            NeonRTOS_TaskDelete(NULL);
            return;
      }
      
      ret = fcntl(Telnet_Server_Socket, F_GETFL, 0);
      if (ret < 0)
      {
            close(Telnet_Server_Socket);
            NeonRTOS_TaskDelete(NULL);
            return;
      }
      ret = fcntl(Telnet_Server_Socket, F_SETFL, ret | O_NONBLOCK); 
      if (ret < 0)
      {
            close(Telnet_Server_Socket);
            NeonRTOS_TaskDelete(NULL);
            return;
      }
    
      while(1)
      {
            add_Telnet_client = false;
            
            ret = accept(Telnet_Server_Socket, (struct sockaddr *) &current_telnetd_client_socket_addr, &sin_size);
            if (ret >= 0) 
            {
                        add_Telnet_client = true;
            }
            else
            {
                  ret = getsockopt(Telnet_Server_Socket, SOL_SOCKET, SO_ERROR, &socket_errno, &socket_errno_optlen);
                  if(ret<0)
                  {
                          NeonRTOS_Sleep(500);
                          continue;
                  }
                  
                  if(socket_errno != EAGAIN)
                  {
                          NeonRTOS_Sleep(500);
                          continue;
                  }
            }
            
            if(add_Telnet_client)
            {
                  current_telnetd_client_socketID = ret;
                  
                  int keepAlive = 1; //enable keepalive
                  ret = setsockopt(current_telnetd_client_socketID, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
                  if(ret < 0) { close(current_telnetd_client_socketID); continue; }
                  
                  int enable = 1;
                  int keepIdle = 20; //60s
                  int keepInterval = 5; //5s
                  int keepCount = 3; //retry times
                  
                  ret = setsockopt(current_telnetd_client_socketID, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof(enable));
                  if(ret < 0) { close(current_telnetd_client_socketID); continue; }
                  ret = setsockopt(current_telnetd_client_socketID, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
                  if(ret < 0) { close(current_telnetd_client_socketID); continue; }
                  ret = setsockopt(current_telnetd_client_socketID, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
                  if(ret < 0) { close(current_telnetd_client_socketID); continue; }
                  ret = setsockopt(current_telnetd_client_socketID, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
                  if(ret < 0) { close(current_telnetd_client_socketID); continue; }
                  
                  ret = fcntl(current_telnetd_client_socketID, F_GETFL, 0);
                  if(ret < 0) { close(current_telnetd_client_socketID); continue; }
                  
                  ret = fcntl(current_telnetd_client_socketID, F_SETFL, ret & (~O_NONBLOCK)); 
                  if(ret < 0) { close(current_telnetd_client_socketID); continue; }

                  
                  int8_t client_index = AddTelnetdClient(current_telnetd_client_socketID, &current_telnetd_client_socket_addr);
                  if (client_index < 0)
                  {
                        close(current_telnetd_client_socketID);
                        continue;
                  }
                      
                  int blockTime = TELNETD_RECV_BLOCK_INTERVAL/GetNumOfTelnetdClient();
                  recvBlockTime = blockTime;

                  for (i = 0; i < TELNET_SERVER_MAX_CLIENTS; i++)
                  {
                        if (Telnetd_Client_List[i].socket_id < 0) continue;
                        
                        ret = setsockopt(Telnetd_Client_List[i].socket_id, SOL_SOCKET, SO_RCVTIMEO, &recvBlockTime, sizeof(recvBlockTime));

                        if(ret < 0) 
                        {
                              Telnetd_Client_List[i].client_destruct_flag = true;
                        }
                  }
                  
                  if(Telnetd_On_Client_Connect_CallBack!=NULL)
                  {
                      Telnetd_On_Client_Connect_CallBack(&Telnetd_Client_List[client_index]);
                  }
                  
                  NeonRTOS_TimerStart(&Telnetd_Client_List[client_index].connection_timeout_timer);
            }
            
            if(GetNumOfTelnetdClient()<=0)
            {
                  NeonRTOS_Sleep(500);
                  continue;
            }
            
            for (i = 0; i < TELNET_SERVER_MAX_CLIENTS; i++)
            {
                  /* IF this handle there is data/event aviliable, recive it*/
                  if (Telnetd_Client_List[i].socket_id < 0) continue;
                  
                  if(Telnetd_Client_List[i].connection_time_unlimit)
                  {
                        NeonRTOS_TimerStop(&Telnetd_Client_List[i].connection_timeout_timer);
                  }
                  
                  if (Telnetd_Client_List[i].connection_timeout_flag == true)
                  {
                          Telnetd_Client_List[i].connection_timeout_flag = false;
                          Telnetd_Client_List[i].client_destruct_flag = true;
                  }
                  
                  if (Telnetd_Client_List[i].client_destruct_flag == true)
                  {
                          if(Telnetd_On_Client_Disconnect_CallBack!=NULL)
                          {
                                Telnetd_On_Client_Disconnect_CallBack(&Telnetd_Client_List[i]);
                          }
                          Telnetd_Client_List[i].client_destruct_flag = false;
                          close(Telnetd_Client_List[i].socket_id);
                          RemoveTelnetdClient(Telnetd_Client_List[i].socket_id);
                          continue;
                  }
                  
                  uint8_t* telnetd_dat_buf = mem_Malloc(TELNET_SERVER_DAT_BUF_SIZE);
                  if (telnetd_dat_buf == NULL)
                  {
                          Telnetd_Client_List[i].client_destruct_flag = true;
                          continue;
                  }
                  memset(telnetd_dat_buf, 0, TELNET_SERVER_DAT_BUF_SIZE);
                  
                  uint16_t recv_len = 0;
                  do{
                      ret = recv(Telnetd_Client_List[i].socket_id, &telnetd_dat_buf[recv_len], 1, 0);
                      
                      if (ret <= 0)
                      {
                              ret = getsockopt(Telnetd_Client_List[i].socket_id, SOL_SOCKET, SO_ERROR, &socket_errno, &socket_errno_optlen);
                              if(ret<0)
                              {
                                      Telnetd_Client_List[i].client_destruct_flag = true;
                              }
                              
                              if(socket_errno == EAGAIN)
                              {
                                      continue;
                              }
                              
                              Telnetd_Client_List[i].client_destruct_flag = true;

                              break;
                      }
                      
                      recv_len+=ret;
                  }
                  while((uint8_t*)strstr((char*)telnetd_dat_buf, "\r\n")==NULL && recv_len<TELNET_SERVER_DAT_BUF_SIZE);
                  
                  if(recv_len>0)
                  {
                        NeonRTOS_TimerStop(&Telnetd_Client_List[i].connection_timeout_timer);
                        
                        //found exit command
                        if(strncmp((const char*)telnetd_dat_buf, Telnet_Exit_Command, strlen(Telnet_Exit_Command))==0)
                        {
                              uint8_t* telnetd_dat_buf_ptr = telnetd_dat_buf+strlen(Telnet_Exit_Command);
                              if(strncmp((const char*)telnetd_dat_buf_ptr, "\r\n", strlen("\r\n"))==0)
                              {
                                    Telnetd_Client_List[i].client_destruct_flag = true;
                              }
                        }
                        //found ping command
                        else if(strncmp((const char*)telnetd_dat_buf, Telnet_Ping_Command, strlen(Telnet_Ping_Command))==0)
                        {
                              uint8_t* telnetd_dat_buf_ptr = telnetd_dat_buf+strlen(Telnet_Ping_Command);
                              if(strncmp((const char*)telnetd_dat_buf_ptr, "\r\n", strlen("\r\n"))==0)
                              {
                                    Telnet_Server_Send_Msg(&Telnetd_Client_List[i], (uint8_t*)Telnet_Pong_Command, strlen(Telnet_Pong_Command), true);
                              }
                        }
                        //found log command
                        else if(strncmp((const char*)telnetd_dat_buf, Telnet_Log_Print_Command, strlen(Telnet_Log_Print_Command))==0)
                        {
                              uint8_t* telnetd_dat_buf_ptr = telnetd_dat_buf+strlen(Telnet_Log_Print_Command);
                              if(strncmp((const char*)telnetd_dat_buf_ptr, "\r\n", strlen("\r\n"))==0)
                              {
                                    Telnetd_Client_List[i].log_print_terminal = true;
                              }
                        }
                        else{
                              if(Telnetd_On_Client_Message_CallBack!=NULL)
                              {
                                    Telnetd_On_Client_Message_CallBack(&Telnetd_Client_List[i], telnetd_dat_buf, recv_len);
                              }
                        }
                        
                        if(!Telnetd_Client_List[i].connection_time_unlimit)
                        {
                              NeonRTOS_TimerReStart(&Telnetd_Client_List[i].connection_timeout_timer);
                        }
                  }
                  else{
                        NeonRTOS_Sleep(100);
                  }
                  
                  mem_Free(telnetd_dat_buf);
            }
      }
    
      for (uint8_t i=0; i<TELNET_SERVER_MAX_CLIENTS; i++)
      {
            if(Telnetd_Client_List[i].socket_id>=0)
            {
                  close(Telnetd_Client_List[i].socket_id);
                  RemoveTelnetdClient(Telnetd_Client_List[i].socket_id);
            }
      }
    
      if(Telnet_Server_Socket>=0)
      {
            close(Telnet_Server_Socket);
            Telnet_Server_Socket = -1;
      }
    
      NeonRTOS_TaskDelete(NULL);
}

int Telnet_Server_Init()
{
      for (uint8_t i=0; i<TELNET_SERVER_MAX_CLIENTS; i++) {
            Telnetd_Restore_Variables(i);
      }
      
      if(NeonRTOS_TaskCreate(Telnet_Server_Task, (signed const char *)"Telnet Server", TELNET_TASK_STACK_SIZE, NULL, TELNET_TASK_PRIORITY, NULL)!=NeonRTOS_OK)
      {
            return -1;
      }
      return 0;
}

void Telnet_Client_Register_OnConnectServerCallback(Telnet_Client_On_Connect_Server_CB cb)
{
    Telnetc_On_Connect_Server_CallBack = cb;
}

void Telnet_Client_Register_OnDisconnectServerCallback(Telnet_Client_On_Disconnect_Server_CB cb)
{
    Telnetc_On_Disconnect_Server_CallBack = cb;
}

void Telnet_Client_Register_OnServerMessageCallback(Telnet_Client_On_Server_Message_CB cb)
{
    Telnetc_On_Server_Message_CallBack = cb;
}

void Telnetc_DNS_Found(const char* hostname, ip_addr_t *ipaddr, void *arg)
{
    if (ipaddr != NULL) {
        /* Address resolved, send request */
        Telnet_Client_Connect_To_Server_ByIP(ipaddr);
    }
}

int Telnet_Client_Connect_To_Server_ByIP(ip_addr_t *ipaddr)
{
      if (ipaddr == NULL) {
            return -1;
      }
      
      if(ipaddr->addr == IPADDR_NONE ||
         ipaddr->addr == IPADDR_LOOPBACK ||
         ipaddr->addr == IPADDR_ANY ||
         ipaddr->addr == IPADDR_BROADCAST)
      {
            return -1;
      }
      //printf("Telnet_DNS_Found: Server address resolved --> connect\n");
      
      if(Telnetc_Conn_Server_Session.connected_to_server){return -1;}
      
      struct sockaddr_in conn_server;
      conn_server.sin_family = AF_INET;
      conn_server.sin_port = htons(TELNET_PORT);
      conn_server.sin_addr.s_addr = htonl(ipaddr->addr);
      
      NeonRTOS_MsgQWrite(&Telnetc_Conn_Server_Session.conn_sockaddr_QueueHandle, (void*)(&conn_server), 0);
      
      if(NeonRTOS_SyncObjWait(&Telnetc_Conn_Server_Session.client_conn_SyncHandle, TELNET_CLIENT_CONN_TIMEOUT)!=NeonRTOS_OK){return -1;}
      
      return 0;
}

int Telnet_Client_Connect_To_Server(const char* hostname)
{
      if(hostname==NULL){return -1;}
      
      if(Telnetc_Conn_Server_Session.connected_to_server){return -1;}
      
      ip_addr_t server_ip;
      if (dns_gethostbyname(hostname, &server_ip, Telnetc_DNS_Found, NULL) == ERR_INPROGRESS) {
            /* DNS request sent, wait for sntp_dns_found being called */
            //printf("Telnet Client: Waiting for server address to be resolved.\n");
            return -1;
      }
      
      return 0;
}

void Telnet_Client_Disconnect_From_Server()
{
      if(!Telnetc_Conn_Server_Session.connected_to_server){return;}
      
      Telnet_Client_Send_Msg(&Telnetc_Conn_Server_Session, (uint8_t*)Telnet_Exit_Command, strlen(Telnet_Exit_Command), true);
      
      //printf("Telnet Client request Disconnect From Server\n");
      NeonRTOS_SyncObjSignalFromISR(&Telnetc_Conn_Server_Session.client_disconn_SyncHandle);
}

Telnetc_Server_Session* Telnet_Client_Get_Current_Server_Connection()
{
      if(!Telnetc_Conn_Server_Session.connected_to_server){return NULL;}
      
      return &Telnetc_Conn_Server_Session;
}

int Telnet_Client_Get_Server_Addr(Telnetc_Server_Session* session, ip_addr_t* server_ip)
{
      if(session==NULL || server_ip==NULL)
      {
              return -1;
      }
      
      if(session->conn_socket_id<0)
      {
              return -1;
      }
      
      struct sockaddr_in* server_sockaddr_in = (struct sockaddr_in*)&session->server_socket_addr;
      
      server_ip->addr = htonl(server_sockaddr_in->sin_addr.s_addr);
      
      return 0;
}

void Telnet_Client_Send_Msg(Telnetc_Server_Session* conn_client, uint8_t* dat, uint16_t len, bool send_new_line)
{
    if(dat==NULL || len==0){return;}
    
    if(conn_client->conn_socket_id<0){return;}
    
    if(!conn_client->connected_to_server){return;}
    
    send(conn_client->conn_socket_id, dat, len, 0);
    
    if(send_new_line)
    {
      send(conn_client->conn_socket_id, "\r\n", strlen("\r\n"), 0);
    }
}

void Telnet_Client_Send_JSON_Msg(Telnetc_Server_Session* conn_client, cJSON* dat, bool need_delete_json) 
{
      if (dat == NULL){return;}
      
      char* json_str = cJSON_Print(dat);
      
      cJSON_Minify(json_str);
      
      if (need_delete_json == true)
      {
            cJSON_Delete(dat);
      }
      
      Telnet_Client_Send_Msg(conn_client, (uint8_t*)json_str, strlen(json_str), true);
      
      mem_Free(json_str);
}

void Telnetc_Ping_Server_Interval_CB(NeonRTOS_TimerHandle ping_server_interval_timer_handle)
{
	NeonRTOS_TimerStop(&ping_server_interval_timer_handle);
        
      Telnet_Client_Send_Msg(&Telnetc_Conn_Server_Session, (uint8_t*)Telnet_Ping_Command, strlen(Telnet_Ping_Command), true);
        
	NeonRTOS_TimerReStart(&Telnetc_Conn_Server_Session.ping_server_timeout_timer);
        
	NeonRTOS_TimerReStart(&ping_server_interval_timer_handle);
}

void Telnetc_Ping_Server_Timeout_CB(NeonRTOS_TimerHandle ping_server_timeout_timer_handle)
{
	NeonRTOS_TimerStop(&ping_server_timeout_timer_handle);
        
      Telnetc_Conn_Server_Session.connection_timeout_flag = true;
}

void Telnet_Client_Task(void* param)
{
      int ret;
      bool disconnected_from_server;
      Telnetc_Conn_Server_Session.connected_to_server = false;
      Telnetc_Conn_Server_Session.connection_timeout_flag = false;
      Telnetc_Conn_Server_Session.conn_socket_id = -1;
      
      int socket_errno;
      const uint32_t socket_errno_optlen = sizeof(socket_errno);
      
      int recvBlockTime = TELNETC_RECV_BLOCK_INTERVAL;

      while(1)
      {
            if(Telnetc_Conn_Server_Session.conn_socket_id<0)
            {
                  Telnetc_Conn_Server_Session.connected_to_server = false;
                  Telnetc_Conn_Server_Session.connection_timeout_flag = false;
                  
                  NeonRTOS_SyncObjClear(&Telnetc_Conn_Server_Session.client_conn_SyncHandle);
                  
                  Telnetc_Conn_Server_Session.conn_socket_id = socket(AF_INET, SOCK_STREAM, 0);
                  if (Telnetc_Conn_Server_Session.conn_socket_id < 0)
                  {
                        NeonRTOS_Sleep(500);
                        continue;
                  }
                
                  int enable = 1;
                  int keepIdle = 20; //60s
                  int keepInterval = 5; //5s
                  int keepCount = 3; //retry times
                
                  ret = setsockopt(Telnetc_Conn_Server_Session.conn_socket_id, SOL_SOCKET, SO_RCVTIMEO, &recvBlockTime, sizeof(recvBlockTime));
                  if (ret < 0)
                  {
                        close(Telnetc_Conn_Server_Session.conn_socket_id);
                        Telnetc_Conn_Server_Session.conn_socket_id = -1;
                        NeonRTOS_Sleep(500);
                        continue;
                  }
                
                  ret = setsockopt(Telnetc_Conn_Server_Session.conn_socket_id, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof(enable));
                  if (ret < 0){
                        close(Telnetc_Conn_Server_Session.conn_socket_id);
                        Telnetc_Conn_Server_Session.conn_socket_id = -1;
                        NeonRTOS_Sleep(500);
                        continue;
                  }
                
                  ret = setsockopt(Telnetc_Conn_Server_Session.conn_socket_id, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
                  if (ret < 0)
                  {
                        close(Telnetc_Conn_Server_Session.conn_socket_id);
                        Telnetc_Conn_Server_Session.conn_socket_id = -1;
                        NeonRTOS_Sleep(500);
                        continue;
                  }
                
                  ret = setsockopt(Telnetc_Conn_Server_Session.conn_socket_id, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
                  if (ret < 0)
                  {
                        close(Telnetc_Conn_Server_Session.conn_socket_id);
                        Telnetc_Conn_Server_Session.conn_socket_id = -1;
                        NeonRTOS_Sleep(500);
                        continue;
                  }
                  
                  ret = setsockopt(Telnetc_Conn_Server_Session.conn_socket_id, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
                  if (ret < 0)
                  {
                        close(Telnetc_Conn_Server_Session.conn_socket_id);
                        Telnetc_Conn_Server_Session.conn_socket_id = -1;
                        NeonRTOS_Sleep(500);
                        continue;
                  }
            }
            
            if (NeonRTOS_MsgQRead(&Telnetc_Conn_Server_Session.conn_sockaddr_QueueHandle, &Telnetc_Conn_Server_Session.server_socket_addr, 500)==NeonRTOS_OK)
            {
                  struct sockaddr_in* server_socket_addr_in = (struct sockaddr_in*)&Telnetc_Conn_Server_Session.server_socket_addr;
                  //printf("Telnet Client Conn Server IP: %d.%d.%d.%d:%d\n", (server_socket_addr_in->sin_addr.s_addr)&0xFF, ((server_socket_addr_in->sin_addr.s_addr)>>8)&0xFF, ((server_socket_addr_in->sin_addr.s_addr)>>16)&0xFF, (server_socket_addr_in->sin_addr.s_addr)>>24, server_socket_addr_in->sin_port);
                  
                  disconnected_from_server = false;
                  Telnetc_Conn_Server_Session.connection_timeout_flag = false;

                  while(!disconnected_from_server)
                  {
                        ret = connect(Telnetc_Conn_Server_Session.conn_socket_id, (const struct sockaddr *)(&Telnetc_Conn_Server_Session.server_socket_addr), sizeof(Telnetc_Conn_Server_Session.server_socket_addr));
                        if (ret < 0)
                        {
                              Telnetc_Conn_Server_Session.connected_to_server = false;
                              close(Telnetc_Conn_Server_Session.conn_socket_id);
                              Telnetc_Conn_Server_Session.conn_socket_id = -1;
                              break;
                        }

                        Telnetc_Conn_Server_Session.connected_to_server = true;
                        NeonRTOS_SyncObjSignal(&Telnetc_Conn_Server_Session.client_conn_SyncHandle);
                        
                        NeonRTOS_SyncObjClear(&Telnetc_Conn_Server_Session.client_disconn_SyncHandle);
                        
                        if(Telnetc_On_Connect_Server_CallBack!=NULL)
                        {
                              Telnetc_On_Connect_Server_CallBack(&Telnetc_Conn_Server_Session);
                        }
                        
                        NeonRTOS_TimerReStart(&Telnetc_Conn_Server_Session.ping_server_interval_timer);
                        
                        while(!disconnected_from_server)
                        {
                              if(Telnetc_Conn_Server_Session.connection_timeout_flag)
                              {
                                    disconnected_from_server = true;
                                    break;
                              }
                              
                              uint8_t* telnetc_dat_buf = mem_Malloc(TELNET_CLIENT_DAT_BUF_SIZE);
                              if (telnetc_dat_buf == NULL)
                              {
                                    NeonRTOS_Sleep(1000);
                                    continue;
                              }
                              memset(telnetc_dat_buf, 0, TELNET_CLIENT_DAT_BUF_SIZE);
                              
                              uint16_t recv_len = 0;
                              do{
                                    ret = recv(Telnetc_Conn_Server_Session.conn_socket_id, &telnetc_dat_buf[recv_len], 1, 0);
                        
                                    if (ret <= 0)
                                    {
                                          ret = getsockopt(Telnetc_Conn_Server_Session.conn_socket_id, SOL_SOCKET, SO_ERROR, &socket_errno, &socket_errno_optlen);
                                          if(ret<0)
                                          {
                                                disconnected_from_server = true;
                                          }
                                          
                                          if(socket_errno == EAGAIN)
                                          {
                                                continue;
                                          }
                                          
                                          disconnected_from_server = true;

                                          break;
                                    }
                                    
                                    recv_len+=ret;
                              }
                              while((uint8_t*)strstr((char*)telnetc_dat_buf, "\r\n")==NULL && recv_len<TELNET_CLIENT_DAT_BUF_SIZE);
                              
                              if(recv_len>0)
                              {
                                    //found exit command
                                    if(strncmp((const char*)telnetc_dat_buf, Telnet_Exit_Command, strlen(Telnet_Exit_Command))==0)
                                    {
                                          uint8_t* exit_cmd_new_line_ptr = telnetc_dat_buf+strlen(Telnet_Exit_Command);
                                          if(strncmp((const char*)exit_cmd_new_line_ptr, "\r\n", strlen("\r\n"))==0)
                                          {
                                                disconnected_from_server = true;
                                          }
                                    }
                                    //found exit command
                                    else if(strncmp((const char*)telnetc_dat_buf, Telnet_Pong_Command, strlen(Telnet_Pong_Command))==0)
                                    {
                                          uint8_t* exit_cmd_new_line_ptr = telnetc_dat_buf+strlen(Telnet_Pong_Command);
                                          if(strncmp((const char*)exit_cmd_new_line_ptr, "\r\n", strlen("\r\n"))==0)
                                          {
                                                NeonRTOS_TimerStop(&Telnetc_Conn_Server_Session.ping_server_timeout_timer);
                                          }
                                    }
                                    else{
                                          if(Telnetc_On_Server_Message_CallBack!=NULL)
                                          {
                                                Telnetc_On_Server_Message_CallBack(&Telnetc_Conn_Server_Session, telnetc_dat_buf, recv_len);
                                          }
                                    }
                              }
                              
                              mem_Free(telnetc_dat_buf);
                              
                              if(NeonRTOS_SyncObjWait(&Telnetc_Conn_Server_Session.client_disconn_SyncHandle, 0)==NeonRTOS_OK)
                              {
                                    //printf("Telnet Client do Disconnect From Server\n");
                                    disconnected_from_server = true;
                              }
                              //printf("Telnet Client Task OnLine\n");
                        }
                        
                        NeonRTOS_TimerStop(&Telnetc_Conn_Server_Session.ping_server_interval_timer);
                        
                        if(disconnected_from_server)
                        {
                              Telnetc_Conn_Server_Session.connection_timeout_flag = false;
                              Telnetc_Conn_Server_Session.connected_to_server = false;
                              
                              NeonRTOS_SyncObjClear(&Telnetc_Conn_Server_Session.client_disconn_SyncHandle);
                              
                              close(Telnetc_Conn_Server_Session.conn_socket_id);
                              
                              if(Telnetc_On_Disconnect_Server_CallBack!=NULL)
                              {
                                    Telnetc_On_Disconnect_Server_CallBack(&Telnetc_Conn_Server_Session);
                              }
                              
                              Telnetc_Conn_Server_Session.conn_socket_id = -1;
                        }
                  }
            }
      }
      
      if(Telnetc_Conn_Server_Session.conn_socket_id>=0)
      {
            close(Telnetc_Conn_Server_Session.conn_socket_id);
            Telnetc_Conn_Server_Session.conn_socket_id = -1;
      }
      
      if(Telnetc_Conn_Server_Session.client_disconn_SyncHandle!=NULL)
      {
            NeonRTOS_SyncObjDelete(&Telnetc_Conn_Server_Session.client_disconn_SyncHandle);
      }
      
      if(Telnetc_Conn_Server_Session.conn_sockaddr_QueueHandle!=NULL)
      {
            NeonRTOS_MsgQDelete(&Telnetc_Conn_Server_Session.conn_sockaddr_QueueHandle);
      }
      
      NeonRTOS_TaskDelete(NULL);
}

int Telnet_Client_Init()
{
      Telnetc_Conn_Server_Session.conn_socket_id = -1;
      Telnetc_Conn_Server_Session.connected_to_server = false;
      
      if(NeonRTOS_TimerCreate(&Telnetc_Conn_Server_Session.ping_server_interval_timer, "Ping Server Interval", TELNET_CLIENT_PING_SERVER_INTERVAL, 1, 0+TELNETD_TIMER_ID_OFFSET, Telnetc_Ping_Server_Interval_CB)!=NeonRTOS_OK)
      {
            return -1;
      }
      NeonRTOS_TimerStop(&Telnetc_Conn_Server_Session.ping_server_interval_timer);
      
      if(NeonRTOS_TimerCreate(&Telnetc_Conn_Server_Session.ping_server_timeout_timer, "Ping Server Timeoot", TELNET_CLIENT_PING_SERVER_TIMEOUT, 1, 1+TELNETD_TIMER_ID_OFFSET, Telnetc_Ping_Server_Timeout_CB)!=NeonRTOS_OK)
      {
            NeonRTOS_TimerDelete(&Telnetc_Conn_Server_Session.ping_server_interval_timer);
            return -1;
      }
      NeonRTOS_TimerStop(&Telnetc_Conn_Server_Session.ping_server_timeout_timer);
      
      NeonRTOS_SyncObjCreate(&Telnetc_Conn_Server_Session.client_conn_SyncHandle); // initialize sync to 0 - binary semaphore
      if (Telnetc_Conn_Server_Session.client_conn_SyncHandle == NULL)
      {
            NeonRTOS_TimerDelete(&Telnetc_Conn_Server_Session.ping_server_interval_timer);
            NeonRTOS_TimerDelete(&Telnetc_Conn_Server_Session.ping_server_timeout_timer);
            return -1;
      }
      NeonRTOS_SyncObjClear(&Telnetc_Conn_Server_Session.client_conn_SyncHandle);
      
      NeonRTOS_SyncObjCreate(&Telnetc_Conn_Server_Session.client_disconn_SyncHandle); // initialize sync to 0 - binary semaphore
      if (Telnetc_Conn_Server_Session.client_disconn_SyncHandle == NULL)
      {
            NeonRTOS_TimerDelete(&Telnetc_Conn_Server_Session.ping_server_interval_timer);
            NeonRTOS_TimerDelete(&Telnetc_Conn_Server_Session.ping_server_timeout_timer);
            NeonRTOS_SyncObjDelete(&Telnetc_Conn_Server_Session.client_conn_SyncHandle);
            return -1;
      }
      NeonRTOS_SyncObjClear(&Telnetc_Conn_Server_Session.client_disconn_SyncHandle);
      
      if(NeonRTOS_MsgQCreate(&Telnetc_Conn_Server_Session.conn_sockaddr_QueueHandle, "telnet conn queue", sizeof(struct sockaddr), 1)!=NeonRTOS_OK)
      {
            NeonRTOS_TimerDelete(&Telnetc_Conn_Server_Session.ping_server_interval_timer);
            NeonRTOS_TimerDelete(&Telnetc_Conn_Server_Session.ping_server_timeout_timer);
            NeonRTOS_SyncObjDelete(&Telnetc_Conn_Server_Session.client_conn_SyncHandle);
            NeonRTOS_SyncObjDelete(&Telnetc_Conn_Server_Session.client_disconn_SyncHandle);
            return -1;
      }
    
      if(NeonRTOS_TaskCreate(Telnet_Client_Task, (signed const char *)"Telnet Client", TELNET_TASK_STACK_SIZE, NULL, TELNET_TASK_PRIORITY, NULL)!=NeonRTOS_OK)
      {
            NeonRTOS_TimerDelete(&Telnetc_Conn_Server_Session.ping_server_interval_timer);
            NeonRTOS_TimerDelete(&Telnetc_Conn_Server_Session.ping_server_timeout_timer);
            NeonRTOS_SyncObjDelete(&Telnetc_Conn_Server_Session.client_conn_SyncHandle);
            NeonRTOS_SyncObjDelete(&Telnetc_Conn_Server_Session.client_disconn_SyncHandle);
            NeonRTOS_MsgQDelete((void*)&Telnetc_Conn_Server_Session.conn_sockaddr_QueueHandle);
            return -1;
      }
      return 0;
}
