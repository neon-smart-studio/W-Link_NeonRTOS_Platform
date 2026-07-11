
#ifndef TELNET_H
#define TELNET_H

#include "Utils/cJSON/cJSON.h"

#define TELNET_PORT                           23
#define TELNET_SERVER_CONN_TIMEOUT            10000
#define TELNET_CLIENT_CONN_TIMEOUT            5000
#define TELNET_CLIENT_PING_SERVER_INTERVAL    5000
#define TELNET_CLIENT_PING_SERVER_TIMEOUT     2000

#define TELNETD_TIMER_ID_OFFSET               100

#define TELNETD_RECV_BLOCK_INTERVAL           500
#define TELNETC_RECV_BLOCK_INTERVAL           500

#define TELNET_SERVER_DAT_BUF_SIZE            4*1024
#define TELNET_CLIENT_DAT_BUF_SIZE            4*1024
#define TELNET_SERVER_MAX_CLIENTS             5

typedef struct Telnetd_Client_Connection Telnetd_Client_Connection;
typedef struct Telnetc_Server_Session Telnetc_Server_Session;

typedef void(*Telnet_Server_On_Client_Connect_CB)(Telnetd_Client_Connection* conn);
typedef void(*Telnet_Server_On_Client_Disconnect_CB)(Telnetd_Client_Connection* conn);
typedef void(*Telnet_Server_On_Client_Message_CB)(Telnetd_Client_Connection* conn, uint8_t* msg, uint16_t len);

typedef void(*Telnet_Client_On_Connect_Server_CB)(Telnetc_Server_Session* sessiom);
typedef void(*Telnet_Client_On_Disconnect_Server_CB)(Telnetc_Server_Session* sessiom);
typedef void(*Telnet_Client_On_Server_Message_CB)(Telnetc_Server_Session* sessiom, uint8_t* msg, uint16_t len);

uint16_t Size_Of_Telnet_Server_Client_Connection_Obj();
uint16_t Size_Of_Telnet_Client_Conn_Server_Session_Obj();

int Telnet_Server_Init();

void Telnet_Server_Register_OnClientConnectCallback(Telnet_Server_On_Client_Connect_CB cb);
void Telnet_Server_Register_OnClientDisconnectCallback(Telnet_Server_On_Client_Disconnect_CB cb);
void Telnet_Server_Register_OnClientMessageCallback(Telnet_Server_On_Client_Message_CB cb);
void Telnet_Server_Remove_Client(Telnetd_Client_Connection* conn_client);
Telnetd_Client_Connection* Telnet_Server_Get_Client_Conn_Session_By_Addr(ip_addr_t* client_ip);
void Telnet_Server_Send_Msg(Telnetd_Client_Connection* conn_client, uint8_t* dat, uint16_t len, bool send_new_line);
void Telnet_Server_Send_JSON_Msg(Telnetd_Client_Connection* conn_client, cJSON* dat, bool need_delete_json);
void Telnet_Server_Print_Log_To_Clients(uint8_t* dat, uint16_t len);
int Telnet_Server_Get_Client_Addr(Telnetd_Client_Connection* conn_client, ip_addr_t* client_ip);
void Telnet_Server_Client_Conn_Time_Unlimit_Enable(Telnetd_Client_Connection* conn_client);
void Telnet_Server_Client_Conn_Time_Unlimit_Disable(Telnetd_Client_Connection* conn_client);

int Telnet_Client_Init();

void Telnet_Client_Register_OnConnectServerCallback(Telnet_Client_On_Connect_Server_CB cb);
void Telnet_Client_Register_OnDisconnectServerCallback(Telnet_Client_On_Disconnect_Server_CB cb);
void Telnet_Client_Register_OnServerMessageCallback(Telnet_Client_On_Server_Message_CB cb);
int Telnet_Client_Connect_To_Server(const char* hostname);
void Telnet_Client_Disconnect_From_Server();
Telnetc_Server_Session* Telnet_Client_Get_Current_Server_Connection();
void Telnet_Client_Send_Msg(Telnetc_Server_Session* conn_client, uint8_t* dat, uint16_t len, bool send_new_line);
void Telnet_Client_Send_JSON_Msg(Telnetc_Server_Session* conn_client, cJSON* dat, bool need_delete_json) ;
int Telnet_Client_Get_Server_Addr(Telnetc_Server_Session* session, ip_addr_t* server_ip);

#endif //TELNET_H