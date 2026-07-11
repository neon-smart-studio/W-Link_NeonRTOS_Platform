/*
ESP8266 Simple Service Discovery
Copyright (c) 2015 Hristo Gochkov

Original (Arduino) version by Filippo Sallemi, July 23, 2014.
Can be found at: https://github.com/nomadnt/uSSDP

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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lwip/ip_addr.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "lwip/igmp.h"
#include "lwip/sockets.h"

#include "Utils/cJSON/cJSON.h"

#include "SSDP.h"

#include "NeonServices/HTTPd/HTTPd.h"

#include "NeonRTOS.h"

#include "NeonTCPIP.h"

#include "ethernet_if/ethernet_if_lwip.h"

// #define DEBUG_SSDP  Serial

#define SSDP_HEADER_BUF_SIZE    1024

#define SSDP_TASK_STACK_SIZE    2*1024

#define SSDP_TASK_PRIORITY      3
#define SSDP_PORT               1900
#define SSDP_SEND_BLOCK_TIME    500

#define SSDP_METHOD_SIZE     10
#define SSDP_URI_SIZE        2
#define SSDP_BUFFER_SIZE     64
#define SSDP_MULTICAST_TTL   2

#define SSDP_MULTICAST_ADDRESS        "239.255.255.250"

const char SSDP_HTTP_Method_NOTIFY[]      =  "NOTIFY";
const char SSDP_HTTP_Method_M_SEARCH[]    =  "M-SEARCH";
const char SSDP_HTTP_Method_RESPONSE[]    =  "RESPONSE";

const char SSDP_Ext[]                      =  "EXT";
const char SSDP_Host_Addr[]                =  "HOST";
const char SSDP_Notify_State[]             =  "NTS";
const char SSDP_All[]                      =  "ssdp:all";
const char SSDP_Alive[]                    =  "ssdp:alive";
const char SSDP_Updatee[]                  =  "ssdp:update";
const char SSDP_Discover[]                 =  "ssdp:discover";
const char SSDP_Cache_Control[]            =  "CACHE-CONTROL";
const char SSDP_Server_Name[]              =  "SERVER";
const char SSDP_Unique_Serial_Number[]     =  "USN";
const char SSDP_Notify_Target[]            =  "NT";
const char SSDP_Search_Target[]            =  "ST";
const char SSDP_URI_Location[]             =  "LOCATION";
const char SSDP_MAN[]                      =  "MAN";
const char SSDP_MX[]                       =  "MX";

const char SSDP_Model[]                    =  "Model";
const char SSDP_Device_Type[]              =  "DeviceType";
const char SSDP_Device_ID[]                =  "DeviceID";

static int SSDP_Socket = -1;

static NeonRTOS_TimerHandle ssdp_notify_timeout_timer;
static NeonRTOS_TimerHandle ssdp_search_timeout_timer;
static NeonRTOS_TimerHandle ssdp_search_result_del_timer;

static SSDP_Status SSDP_Current_Status = SSDP_Status_Idle;

static ip_addr_t multicast_addr;
static char SSDP_Self_UUID[SSDP_UUID_SIZE];
static char SSDP_DeviceType[SSDP_DEVICE_TYPE_SIZE];
static char SSDP_ServiceURL[SSDP_SERVICE_URL_SIZE];
static char SSDP_ManufacturerName[SSDP_MANUFACTURER_NAME_SIZE];
static char SSDP_ManufacturerURL[SSDP_MANUFACTURER_URL_SIZE];
static char SSDP_ModelName[SSDP_MODEL_NAME_SIZE];
static char SSDP_ModelVersion[SSDP_MODEL_VERSION_SIZE];
static uint16_t SSDP_ServicePort;

static SSDP_Search_Node_List SSDP_Search_Result;

static SSDP_On_Send_Notify_Ind_CB On_Send_Notify_Ind_Callback = NULL;
static SSDP_On_Recv_Notify_Ind_CB On_Recv_Notify_Ind_Callback = NULL;
static SSDP_On_Recv_Search_Req_CB On_Recv_Search_Req_Callback  = NULL;
static SSDP_On_Search_Nodes_Done_CB On_Search_Nodes_Done_Callback  = NULL;

void SSDP_SetServicePort(uint16_t port)
{
  SSDP_ServicePort = port;
}

void SSDP_SetServiceURL(const char *url)
{
    memset(SSDP_ServiceURL, 0, SSDP_SERVICE_URL_SIZE);
    strncpy(SSDP_ServiceURL, url, strlen(url));
}

void SSDP_SetDeviceType(const char *deviceType)
{
    memset(SSDP_DeviceType, 0, SSDP_DEVICE_TYPE_SIZE);
    strncpy(SSDP_DeviceType, deviceType, strlen(deviceType));
}

void SSDP_SetModelName(const char *name)
{
    memset(SSDP_ModelName, 0, SSDP_MODEL_NAME_SIZE);
    strncpy(SSDP_ModelName, name, strlen(name));
}

void SSDP_SetModelVersion(const char *num)
{
    memset(SSDP_ModelVersion, 0, SSDP_MODEL_VERSION_SIZE);
    strncpy(SSDP_ModelVersion, num, strlen(num));
}

void SSDP_SetManufacturerName(const char *name)
{
    memset(SSDP_ManufacturerName, 0, SSDP_MANUFACTURER_NAME_SIZE);
    strncpy(SSDP_ManufacturerName, name, strlen(name));
}

void SSDP_SetManufacturerURL(const char *url)
{
    memset(SSDP_ManufacturerURL, 0, SSDP_MANUFACTURER_URL_SIZE);
    strncpy(SSDP_ManufacturerURL, url, strlen(url));
}

void SSDP_Register_OnSendNotifyIndCallBack(SSDP_On_Send_Notify_Ind_CB cb)
{
    On_Send_Notify_Ind_Callback = cb;
}

void SSDP_Register_OnRecvNotifyIndCallback(SSDP_On_Recv_Notify_Ind_CB cb)
{
    On_Recv_Notify_Ind_Callback = cb;
}

void SSDP_Register_OnRecvSearchReqCallBack(SSDP_On_Recv_Search_Req_CB cb)
{
    On_Recv_Search_Req_Callback = cb;
}

uint8_t* SSDP_Parse_Reguest_Method(uint8_t* buf, SSDP_Method req_method, uint8_t* req_path)
{
	if (buf == NULL || req_path==NULL)
	{
		return NULL;
	}
	
	uint8_t* buf_ptr = buf;
	uint16_t offset;
	
	switch (req_method)
	{
	case SSDP_Method_NOTIFY:
		memcpy(buf_ptr, SSDP_HTTP_Method_NOTIFY, strlen(SSDP_HTTP_Method_NOTIFY));
		buf_ptr += strlen(SSDP_HTTP_Method_NOTIFY);
		break;
	case SSDP_Method_M_SEARCH:
		memcpy(buf_ptr, SSDP_HTTP_Method_M_SEARCH, strlen(SSDP_HTTP_Method_M_SEARCH));
		buf_ptr += strlen(SSDP_HTTP_Method_M_SEARCH);
		break;
	default:
		return NULL;
	}
	offset = sprintf((char*)buf_ptr, " %s ", (char*)req_path);
	buf_ptr += offset;
	
	memcpy(buf_ptr, HTTP_Version_1P1, strlen(HTTP_Version_1P1));
	buf_ptr += strlen(HTTP_Version_1P1);
	
	memcpy(buf_ptr, "\r\n", strlen("\r\n"));
	buf_ptr += strlen("\r\n");
	
	return buf_ptr;
}

uint8_t* SSDP_Parse_Headers(uint8_t* buf, const char* header_str, const char* header_val)
{
    if(buf==NULL || header_str==NULL || header_val==NULL){return NULL;}
	return HTTPd_Parse_Headers(buf, header_str, header_val);
}

uint8_t* SSDP_Parse_End_Of_Headers(uint8_t* buf)
{
    if(buf==NULL){return NULL;}
	return HTTPd_Parse_End_Of_Headers(buf);
}

uint8_t* SSDP_Get_Header_Value(uint8_t* buf, uint8_t* header_str)
{
    if(buf==NULL || header_str==NULL){return NULL;}
	return HTTPd_Get_Header_Value(buf, header_str);
}

int SSDP_Get_Header_Value_Length(uint8_t* header_val_str)
{
    if(header_val_str==NULL){return -1;}
    uint8_t* ssdp_new_line = (uint8_t*)strstr((const char*)header_val_str, "\r\n");
    if(ssdp_new_line==NULL){return -1;}
    return (ssdp_new_line - header_val_str);
}

void SSDP_Send_Notify()
{
    if(SSDP_Socket<0){return;}
    
    uint8_t* ssdp_nt_hd_buf = mem_Malloc(SSDP_HEADER_BUF_SIZE);
    if (ssdp_nt_hd_buf == NULL)
    {
            return;
    }
    memset(ssdp_nt_hd_buf, 0, SSDP_HEADER_BUF_SIZE);
    
    uint8_t* buf_ptr = ssdp_nt_hd_buf;
    uint8_t* ssdp_header_value = mem_Malloc(150);
    uint8_t* ssdp_header_value_ptr;
    if(ssdp_header_value==NULL){
            goto exit;
    }
    memset(ssdp_header_value, 0, 150);
    
    buf_ptr = SSDP_Parse_Reguest_Method(buf_ptr, SSDP_Method_NOTIFY, "*");
    
    //Host Address Header
    ssdp_header_value_ptr = ssdp_header_value;
    ssdp_header_value_ptr+=sprintf((char*)ssdp_header_value, "%s:%d", SSDP_MULTICAST_ADDRESS, SSDP_PORT);
    *ssdp_header_value_ptr++ = '\0';
    buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Host_Addr, (const char*)ssdp_header_value);
    
    //Cache Controll Header
    ssdp_header_value_ptr = ssdp_header_value;
    ssdp_header_value_ptr+=sprintf((char*)ssdp_header_value, "max-age=%u", SSDP_NOTIFY_INTERVAL);
    *ssdp_header_value_ptr++ = '\0';
    buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Cache_Control, (const char*)ssdp_header_value);
    
    //URI Location Header
    uint32_t ip_addr = NeonTCPIP_IF_Get_IP_Address();
    ssdp_header_value_ptr = ssdp_header_value;
    ssdp_header_value_ptr+=sprintf((char*)ssdp_header_value, "http://%u.%u.%u.%u:%u/", ip_addr&0xFF, (ip_addr>>8)&0xFF,\
                                            (ip_addr>>16)&0xFF, ip_addr>>24, SSDP_ServicePort);
    *ssdp_header_value_ptr++ = '\0';
    buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_URI_Location, (const char*)ssdp_header_value);
    
    //Notify Target Header
    ssdp_header_value_ptr = ssdp_header_value;
    ssdp_header_value_ptr+=sprintf((char*)ssdp_header_value, "device_type:%s", SSDP_DeviceType);
    *ssdp_header_value_ptr++ = '\0';
    buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Notify_Target, (const char*)ssdp_header_value);
    
    //Notify State Header
    buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Notify_State, SSDP_Alive);
    
    //Server Name Header
    ssdp_header_value_ptr = ssdp_header_value;
    ssdp_header_value_ptr+=sprintf((char*)ssdp_header_value, "%s/%s UPNP/1.1 %s/%s", "NeonRT", NeonRTOS_GetOsVersion(), SSDP_ModelName, SSDP_ModelVersion);
    *ssdp_header_value_ptr++ = '\0';
    buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Server_Name, (const char*)ssdp_header_value);
    
    //Model Header
    ssdp_header_value_ptr = ssdp_header_value;
    ssdp_header_value_ptr+=sprintf((char*)ssdp_header_value, "%s/%s", SSDP_ModelName, SSDP_ModelVersion);
    *ssdp_header_value_ptr++ = '\0';
    buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Model, (const char*)ssdp_header_value);
    
    //DeviceType Header
    buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Device_Type, SSDP_DeviceType);
    
    //DeviceID Header
    //buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Device_ID, (const char*)Node_Device_ID);
    
    //UUID Header
    ssdp_header_value_ptr = ssdp_header_value;
    ssdp_header_value_ptr+=sprintf((char*)ssdp_header_value, "uuid:%s", SSDP_Self_UUID);
    *ssdp_header_value_ptr++ = '\0';
    buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Unique_Serial_Number, (const char*)ssdp_header_value);
    
    //Add Custom Headers Via Send Notify Callback
    if(On_Send_Notify_Ind_Callback!=NULL)
    {
        uint8_t* cb_return_header = On_Send_Notify_Ind_Callback();
        uint8_t* cb_return_header_buf_ptr = cb_return_header;
        //skip all headers
        while(cb_return_header_buf_ptr!=NULL){
            cb_return_header_buf_ptr = (uint8_t*)strstr((const char*)cb_return_header_buf_ptr, "\r\n");
            cb_return_header_buf_ptr+=strlen("\r\n");
        }
        if(cb_return_header_buf_ptr!=NULL)
        {
            uint16_t custom_header_size = cb_return_header_buf_ptr-cb_return_header;
            
            uint8_t* new_buf = mem_Malloc(SSDP_HEADER_BUF_SIZE + custom_header_size);
            if(new_buf==NULL){goto exit;}
            
            uint16_t orig_header_size = buf_ptr-ssdp_nt_hd_buf;
            memcpy(new_buf, ssdp_nt_hd_buf, orig_header_size);
            
            mem_Free(ssdp_nt_hd_buf);
            ssdp_nt_hd_buf = new_buf;
            buf_ptr = ssdp_nt_hd_buf+orig_header_size;
            
            memcpy(buf_ptr, cb_return_header, custom_header_size);
            buf_ptr+=custom_header_size;
            
            mem_Free(cb_return_header_buf_ptr);
        }
    }
    
    //End Of Header
    buf_ptr = SSDP_Parse_End_Of_Headers(buf_ptr);
    
    struct sockaddr_in SSDP_Socket_addr;
    memset(&SSDP_Socket_addr, 0, sizeof(SSDP_Socket_addr)); /* Zero out structure */
    SSDP_Socket_addr.sin_family = AF_INET;            /* Internet address family */
    SSDP_Socket_addr.sin_addr.s_addr = multicast_addr.addr;   /* Any incoming interface */
    SSDP_Socket_addr.sin_len = sizeof(multicast_addr);  
    SSDP_Socket_addr.sin_port = htons(SSDP_PORT); /* Local port */
    
    sendto(SSDP_Socket, ssdp_nt_hd_buf, buf_ptr-ssdp_nt_hd_buf, 0, (struct sockaddr*)&SSDP_Socket_addr, sizeof(struct sockaddr));
    
    mem_Free(ssdp_header_value);
exit:
    mem_Free(ssdp_nt_hd_buf);
}

void SSDP_Search_Nodes(SSDP_Search_Target_Type SSDP_stt, uint8_t* SSDP_search_target, NeonRTOS_Time_t search_time, SSDP_On_Search_Nodes_Done_CB cb)
{
    if(SSDP_Socket<0){return;}
    if(SSDP_Current_Status!=SSDP_Status_Idle){return;}
    if(SSDP_stt>=SSDP_Search_Target_MAX || SSDP_stt==SSDP_Search_Target_None){return;}
    if(SSDP_stt!=SSDP_Search_Target_All && SSDP_search_target==NULL){return;}
    
    uint8_t* ssdp_nt_hd_buf = mem_Malloc(SSDP_HEADER_BUF_SIZE);
    if (ssdp_nt_hd_buf == NULL)
    {
            return;
    }
    memset(ssdp_nt_hd_buf, 0, SSDP_HEADER_BUF_SIZE);
    
    uint8_t* buf_ptr = ssdp_nt_hd_buf;
    
    buf_ptr = SSDP_Parse_Reguest_Method(buf_ptr, SSDP_Method_M_SEARCH, "*");
    
    uint8_t* ssdp_header_value = mem_Malloc(150);
    uint8_t* ssdp_header_value_ptr;
    if(ssdp_header_value==NULL){
            goto exit;
    }
    memset(ssdp_header_value, 0, 150);
    
    //Host Address Header
    ssdp_header_value_ptr = ssdp_header_value;
    ssdp_header_value_ptr+=sprintf((char*)ssdp_header_value, "%s:%d", SSDP_MULTICAST_ADDRESS, SSDP_PORT);
    *ssdp_header_value_ptr++ = '\0';
    buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Host_Addr, (const char*)ssdp_header_value);
    
    mem_Free(ssdp_header_value);
    
    //MAN Header
    buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_MAN, SSDP_Discover);
    
    //MX Header
    buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_MX, "5");
    
    //Search Target Header
    if(SSDP_stt==SSDP_Search_Target_All){
        buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Search_Target, SSDP_All);
    }
    else{
        uint8_t* st_buf = mem_Malloc(128);
        if(st_buf==NULL){
            mem_Free(ssdp_header_value);
            goto exit;
        }
        memset(st_buf, 0, 128);
        switch(SSDP_stt)
        {
        case SSDP_Search_Target_Device_Type:
            sprintf((char*)st_buf, "device_type:%s", SSDP_search_target);
            buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Search_Target, (const char*)st_buf);
            break;
        case SSDP_Search_Target_Device_ID:
            sprintf((char*)st_buf, "device_ID:%s", SSDP_search_target);
            buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Search_Target, (const char*)st_buf);
            break;
        case SSDP_Search_Target_Model_Name:
            sprintf((char*)st_buf, "model:%s", SSDP_search_target);
            buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Search_Target, (const char*)st_buf);
            break;
        case SSDP_Search_Target_UUID:
            sprintf((char*)st_buf, "uuid:%s", SSDP_search_target);
            buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Search_Target, (const char*)st_buf);
            break;
        }
        mem_Free(st_buf);
    }
    
    //End Of Header
    buf_ptr = SSDP_Parse_End_Of_Headers(buf_ptr);
    
    struct sockaddr_in SSDP_Socket_addr;
    memset(&SSDP_Socket_addr, 0, sizeof(SSDP_Socket_addr)); /* Zero out structure */
    SSDP_Socket_addr.sin_family = AF_INET;            /* Internet address family */
    SSDP_Socket_addr.sin_addr.s_addr = multicast_addr.addr;   /* Any incoming interface */
    SSDP_Socket_addr.sin_len = sizeof(ip_addr_t);
    SSDP_Socket_addr.sin_port = htons(SSDP_PORT); /* Local port */
    
    SSDP_Search_Result.num_found_nodes = 0;
    SSDP_Search_Result.start = NULL;
    SSDP_Current_Status = SSDP_Status_Searching;
    
    On_Search_Nodes_Done_Callback = cb;
    
    NeonRTOS_TimerChangePeriod(&ssdp_search_timeout_timer, search_time);
    NeonRTOS_TimerStart(&ssdp_search_timeout_timer);
    
    sendto(SSDP_Socket, ssdp_nt_hd_buf, buf_ptr-ssdp_nt_hd_buf, 0, (struct sockaddr*)&SSDP_Socket_addr, sizeof(struct sockaddr));
    
exit:
    mem_Free(ssdp_nt_hd_buf);
}

static void SSDP_Handle_Notify_Event(uint8_t* notify_headers, struct sockaddr_in* source_host_addr)
{
    if(source_host_addr==NULL){return;}
    
    int header_len;
    SSDP_Notify_Inf notify_inf;
    
    notify_inf.Host_Address = SSDP_Get_Header_Value(notify_headers, (uint8_t*)SSDP_Host_Addr);
    
    notify_inf.Cache_Controll = SSDP_Get_Header_Value(notify_headers, (uint8_t*)SSDP_Cache_Control);
    notify_inf.Location = SSDP_Get_Header_Value(notify_headers, (uint8_t*)SSDP_URI_Location);
    notify_inf.Notify_Target = SSDP_Get_Header_Value(notify_headers, (uint8_t*)SSDP_Notify_Target);
    notify_inf.Notify_State = SSDP_Get_Header_Value(notify_headers, (uint8_t*)SSDP_Notify_State);
    notify_inf.Server_Name = SSDP_Get_Header_Value(notify_headers, (uint8_t*)SSDP_Server_Name);
    notify_inf.Model = SSDP_Get_Header_Value(notify_headers, (uint8_t*)SSDP_Model);
    notify_inf.DeviceType = SSDP_Get_Header_Value(notify_headers, (uint8_t*)SSDP_Device_Type);
    notify_inf.DeviceID = SSDP_Get_Header_Value(notify_headers, (uint8_t*)SSDP_Device_ID);
    notify_inf.UUID = SSDP_Get_Header_Value(notify_headers, (uint8_t*)SSDP_Unique_Serial_Number);
    
    header_len = SSDP_Get_Header_Value_Length(notify_inf.Host_Address);
    if(header_len<0){return;}
    notify_inf.Host_Address[header_len] = '\0';
    
    header_len = SSDP_Get_Header_Value_Length(notify_inf.Location);
    if(header_len<0){return;}
    notify_inf.Location[header_len] = '\0';
    
    header_len = SSDP_Get_Header_Value_Length(notify_inf.Notify_Target);
    if(header_len<0){return;}
    notify_inf.Notify_Target[header_len] = '\0';
    
    header_len = SSDP_Get_Header_Value_Length(notify_inf.Notify_State);
    if(header_len<0){return;}
    notify_inf.Notify_State[header_len] = '\0';
    
    header_len = SSDP_Get_Header_Value_Length(notify_inf.Server_Name);
    if(header_len<0){return;}
    notify_inf.Server_Name[header_len] = '\0';
    
    header_len = SSDP_Get_Header_Value_Length(notify_inf.Model);
    if(header_len<0){return;}
    notify_inf.Model[header_len] = '\0';
    
    header_len = SSDP_Get_Header_Value_Length(notify_inf.DeviceType);
    if(header_len<0){return;}
    notify_inf.DeviceType[header_len] = '\0';
    
    header_len = SSDP_Get_Header_Value_Length(notify_inf.DeviceID);
    if(header_len<0){return;}
    notify_inf.DeviceID[header_len] = '\0';
    
    header_len = SSDP_Get_Header_Value_Length(notify_inf.UUID);
    if(header_len<0){return;}
    notify_inf.UUID[header_len] = '\0';
    
    if(On_Recv_Notify_Ind_Callback!=NULL)
    {
        On_Recv_Notify_Ind_Callback(&notify_inf, notify_headers);
    }
}

static void SSDP_Handle_Search_Request(uint8_t* request_headers, struct sockaddr_in* source_host_addr)
{
    uint8_t* search_target = SSDP_Get_Header_Value(request_headers, (uint8_t*)SSDP_Search_Target);
    if(search_target!=NULL)
    {
        while(*search_target ==  ' '){search_target++;}
        uint8_t* end_st = (uint8_t*)strstr((const char*)search_target, "\r\n");
        if (end_st == NULL) return;
        do {end_st--;} while(*end_st ==  ' ');
        end_st++;
        *end_st = '\0'; //terminate url part
        
        SSDP_Search_Target_Type SSDP_stt = SSDP_Search_Target_None;
        
        if(strlen((const char*)search_target)==strlen(SSDP_All))
        {
            if(strncmp((const char*)search_target, SSDP_All, strlen(SSDP_All))==0)
            {
                SSDP_stt = SSDP_Search_Target_All;
            }
        }
        else{
            uint8_t* st_colon_tag = (uint8_t*)strchr((const char*)search_target, ':');
            if(st_colon_tag==NULL){return;}
            
            if((st_colon_tag-search_target)==strlen("device_type"))
            {
                if(strncmp((const char*)search_target, "device_type", strlen("device_type"))==0)
                {
                    if(strlen((const char*)st_colon_tag+1)==strlen(SSDP_DeviceType))
                    {
                        if(strncmp((const char*)st_colon_tag+1, SSDP_DeviceType, strlen(SSDP_DeviceType))==0)
                        {
                            SSDP_stt = SSDP_Search_Target_Device_Type;
                        }
                    }
                }
            }
            if((st_colon_tag-search_target)==strlen("device_ID"))
            {
                if(strncmp((const char*)search_target, "device_ID", strlen("device_ID"))==0)
                {
                    if(strlen((const char*)st_colon_tag+1)==strlen(SSDP_Device_ID))
                    {
                        if(strncmp((const char*)st_colon_tag+1, SSDP_Device_ID, strlen(SSDP_Device_ID))==0)
                        {
                            SSDP_stt = SSDP_Search_Target_Device_ID;
                        }
                    }
                }
            }
            if((st_colon_tag-search_target)==strlen("model"))
            {
                if(strncmp((const char*)search_target, "model", strlen("model"))==0)
                {
                    if(strlen((const char*)st_colon_tag+1)==strlen(SSDP_ModelName))
                    {
                        if(strncmp((const char*)st_colon_tag+1, SSDP_ModelName, strlen(SSDP_ModelName))==0)
                        {
                            SSDP_stt = SSDP_Search_Target_Model_Name;
                        }
                    }
                }
            }
            if((st_colon_tag-search_target)==strlen("uuid"))
            {
                if(strncmp((const char*)search_target, "uuid", strlen("uuid"))==0)
                {
                    if(strlen((const char*)st_colon_tag+1)==strlen(SSDP_Self_UUID))
                    {
                        if(strncmp((const char*)st_colon_tag+1, SSDP_Self_UUID, strlen(SSDP_Self_UUID))==0)
                        {
                            SSDP_stt = SSDP_Search_Target_UUID;
                        }
                    }
                }
            }
        }
        
        if(SSDP_stt==SSDP_Search_Target_None){return;}
        
        uint8_t* ssdp_rsp_hd_buf = mem_Malloc(SSDP_HEADER_BUF_SIZE);
        if (ssdp_rsp_hd_buf == NULL)
        {
            return;
        }
        memset(ssdp_rsp_hd_buf, 0, SSDP_HEADER_BUF_SIZE);
        
        uint8_t* buf_ptr = ssdp_rsp_hd_buf;
        uint8_t* ssdp_header_value = mem_Malloc(150);
        uint8_t* ssdp_header_value_ptr;
        
        if(ssdp_header_value==NULL)
        {
            goto exit;
        }
        memset(ssdp_header_value, 0, 150);
        
        buf_ptr = HTTPd_Parse_Status_Code(buf_ptr, 200);
        
        //Cache Controll Header
        ssdp_header_value_ptr = ssdp_header_value;
        ssdp_header_value_ptr+=sprintf((char*)ssdp_header_value, "max-age=%u", SSDP_NOTIFY_INTERVAL);
        *ssdp_header_value_ptr++ = '\0';
        buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Cache_Control, (const char*)ssdp_header_value);
        
        //EXT Header
        buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Ext, "");
        
        //URI Location Header
        ssdp_header_value_ptr = ssdp_header_value;
        
        uint32_t ip_addr = NeonTCPIP_IF_Get_IP_Address();
        ssdp_header_value_ptr = ssdp_header_value;
        ssdp_header_value_ptr+=sprintf((char*)ssdp_header_value, "http://%u.%u.%u.%u:%u/", ip_addr&0xFF, (ip_addr>>8)&0xFF,\
                                            (ip_addr>>16)&0xFF, ip_addr>>24, SSDP_ServicePort);
        *ssdp_header_value_ptr++ = '\0';
        buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_URI_Location, (const char*)ssdp_header_value);
        
        //Server Name Header
        ssdp_header_value_ptr = ssdp_header_value;
        ssdp_header_value_ptr+=sprintf((char*)ssdp_header_value, "UPNP/1.1 %s/%s", SSDP_ModelName, SSDP_ModelVersion);
        *ssdp_header_value_ptr++ = '\0';
        buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Server_Name, (const char*)ssdp_header_value);
        
        //Search Target Header
        if(SSDP_stt==SSDP_Search_Target_All){
            buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Search_Target, SSDP_All);
        }
        else{
            buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Search_Target, (const char*)search_target);
        }
        
        //Model Header
        ssdp_header_value_ptr = ssdp_header_value;
        ssdp_header_value_ptr+=sprintf((char*)ssdp_header_value, "%s/%s", SSDP_ModelName, SSDP_ModelVersion);
        *ssdp_header_value_ptr++ = '\0';
        buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Model, (const char*)ssdp_header_value);
        
        //DeviceType Header
        buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Device_Type, SSDP_DeviceType);
        
        //DeviceID Header
        //buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Device_ID, (const char*)Node_Device_ID);
        
        //UUID Header
        ssdp_header_value_ptr = ssdp_header_value;
        ssdp_header_value_ptr+=sprintf((char*)ssdp_header_value, "uuid:%s", SSDP_Self_UUID);
        *ssdp_header_value_ptr++ = '\0';
        buf_ptr = SSDP_Parse_Headers(buf_ptr, SSDP_Unique_Serial_Number, (const char*)ssdp_header_value);
        
        //Add Custom Headers Via Send Search Response Callback
        if(On_Recv_Search_Req_Callback!=NULL)
        {
            uint8_t* cb_return_header = On_Recv_Search_Req_Callback();
            uint8_t* cb_return_header_buf_ptr = cb_return_header;
            //skip all headers
            while(cb_return_header_buf_ptr!=NULL){
                cb_return_header_buf_ptr = (uint8_t*)strstr((const char*)cb_return_header_buf_ptr, "\r\n");
                cb_return_header_buf_ptr+=strlen("\r\n");
            }
            if(cb_return_header_buf_ptr!=NULL)
            {
                uint16_t custom_header_size = cb_return_header_buf_ptr-cb_return_header;
                
                uint8_t* new_buf = mem_Malloc(SSDP_HEADER_BUF_SIZE + custom_header_size);
                if(new_buf==NULL){goto exit;}
                
                uint16_t orig_header_size = buf_ptr-ssdp_rsp_hd_buf;
                memcpy(new_buf, ssdp_rsp_hd_buf, orig_header_size);
                
                mem_Free(ssdp_rsp_hd_buf);
                ssdp_rsp_hd_buf = new_buf;
                buf_ptr = ssdp_rsp_hd_buf+orig_header_size;
                
                memcpy(buf_ptr, cb_return_header, custom_header_size);
                buf_ptr+=custom_header_size;
                
                mem_Free(cb_return_header_buf_ptr);
            }
        }
    
        //End Of Header
        buf_ptr = SSDP_Parse_End_Of_Headers(buf_ptr);
        
        struct sockaddr_in SSDP_Socket_addr;
        memset(&SSDP_Socket_addr, 0, sizeof(SSDP_Socket_addr));
        SSDP_Socket_addr.sin_family = AF_INET;
        SSDP_Socket_addr.sin_addr.s_addr = source_host_addr->sin_addr.s_addr;
        SSDP_Socket_addr.sin_port = source_host_addr->sin_port;

        sendto(SSDP_Socket, ssdp_rsp_hd_buf, buf_ptr-ssdp_rsp_hd_buf, 0, (struct sockaddr*)&SSDP_Socket_addr, sizeof(struct sockaddr));
        
        mem_Free(ssdp_header_value);
exit:
        mem_Free(ssdp_rsp_hd_buf);

    }
}

void SSDP_Handle_Search_Response(uint8_t* response_headers, struct sockaddr_in* source_host_addr)
{
    if(SSDP_Current_Status!=SSDP_Status_Searching){return;}
    if(source_host_addr==NULL){return;}
        
	SSDP_Search_Response_Item* start = SSDP_Search_Result.start;
	SSDP_Search_Response_Item* last = start;
	SSDP_Search_Response_Item* newItem = NULL;
        
    if(start!=NULL)
    {
        while (last->next != NULL)
        {
            last = last->next;
        }
    }

    newItem = mem_Malloc(sizeof(SSDP_Search_Response_Item));
    if (newItem == NULL)
    {
            return;
    }
    
    int cache_controll_hd_val_len;
    int ext_hd_val_len;
    int location_hd_val_len;
    int server_name_hd_val_len;
    int search_target_hd_val_len;
    int model_hd_val_len;
    int device_type_hd_val_len;
    int device_ID_hd_val_len;
    int uuid_hd_val_len;
    uint8_t* cache_controll_hd_val;
    uint8_t* ext_hd_val;
    uint8_t* location_hd_val;
    uint8_t* server_name_hd_val;
    uint8_t* search_target_hd_val;
    uint8_t* model_hd_val;
    uint8_t* device_type_hd_val;
    uint8_t* device_ID_hd_val;
    uint8_t* uuid_hd_val;
    
    cache_controll_hd_val = SSDP_Get_Header_Value(response_headers, (uint8_t*)SSDP_Cache_Control);
    if (cache_controll_hd_val == NULL){ mem_Free(newItem); return; }
    ext_hd_val = SSDP_Get_Header_Value(response_headers, (uint8_t*)SSDP_Ext);
    if (ext_hd_val == NULL){ mem_Free(newItem); return; }
    location_hd_val = SSDP_Get_Header_Value(response_headers, (uint8_t*)SSDP_URI_Location);
    if (location_hd_val == NULL){ mem_Free(newItem); return; }
    server_name_hd_val = SSDP_Get_Header_Value(response_headers, (uint8_t*)SSDP_Server_Name);
    if (server_name_hd_val == NULL){ mem_Free(newItem); return; }
    search_target_hd_val = SSDP_Get_Header_Value(response_headers, (uint8_t*)SSDP_Search_Target);
    if (search_target_hd_val == NULL){ mem_Free(newItem); return; }
    model_hd_val = SSDP_Get_Header_Value(response_headers, (uint8_t*)SSDP_Model);
    if (model_hd_val == NULL){ mem_Free(newItem); return; }
    device_type_hd_val = SSDP_Get_Header_Value(response_headers, (uint8_t*)SSDP_Device_Type);
    if (device_type_hd_val == NULL){ mem_Free(newItem); return; }
    device_ID_hd_val = SSDP_Get_Header_Value(response_headers, (uint8_t*)SSDP_Device_ID);
    if (device_ID_hd_val == NULL){ mem_Free(newItem); return; }
    uuid_hd_val = SSDP_Get_Header_Value(response_headers, (uint8_t*)SSDP_Unique_Serial_Number);
    if (uuid_hd_val == NULL){ mem_Free(newItem); return; }
    
    cache_controll_hd_val_len = SSDP_Get_Header_Value_Length(cache_controll_hd_val);
    if(cache_controll_hd_val_len<0){ mem_Free(newItem); return; }
    ext_hd_val_len = SSDP_Get_Header_Value_Length(ext_hd_val);
    if(ext_hd_val_len<0){ mem_Free(newItem); return; }
    location_hd_val_len = SSDP_Get_Header_Value_Length(location_hd_val);
    if(location_hd_val_len<0){ mem_Free(newItem); return; }
    server_name_hd_val_len = SSDP_Get_Header_Value_Length(server_name_hd_val);
    if(server_name_hd_val_len<0){ mem_Free(newItem); return; }
    search_target_hd_val_len = SSDP_Get_Header_Value_Length(search_target_hd_val);
    if(search_target_hd_val_len<0){ mem_Free(newItem); return; }
    model_hd_val_len = SSDP_Get_Header_Value_Length(model_hd_val);
    if(model_hd_val_len<0){ mem_Free(newItem); return; }
    device_type_hd_val_len = SSDP_Get_Header_Value_Length(device_type_hd_val);
    if(device_type_hd_val_len<0){ mem_Free(newItem); return; }
    device_ID_hd_val_len = SSDP_Get_Header_Value_Length(device_ID_hd_val);
    if(device_ID_hd_val_len<0){ mem_Free(newItem); return; }
    uuid_hd_val_len = SSDP_Get_Header_Value_Length(uuid_hd_val);
    if(uuid_hd_val_len<0){ mem_Free(newItem); return; }
    
    newItem->Cache_Controll = mem_Malloc(cache_controll_hd_val_len+1);
    newItem->EXT = mem_Malloc(ext_hd_val_len+1);
    newItem->Location = mem_Malloc(location_hd_val_len+1);
    newItem->Server_Name = mem_Malloc(server_name_hd_val_len+1);
    newItem->Search_Target = mem_Malloc(search_target_hd_val_len+1);
    newItem->Model = mem_Malloc(model_hd_val_len+1);
    newItem->DeviceType = mem_Malloc(device_type_hd_val_len+1);
    newItem->DeviceID = mem_Malloc(device_ID_hd_val_len+1);
    newItem->UUID = mem_Malloc(uuid_hd_val_len+1);
    
    if(newItem->Cache_Controll==NULL || newItem->EXT==NULL || newItem->Location==NULL || newItem->Server_Name==NULL || \
        newItem->Search_Target==NULL || newItem->Model==NULL || newItem->DeviceType==NULL|| newItem->DeviceID==NULL || newItem->UUID==NULL)
    {
        if(newItem->Cache_Controll!=NULL){mem_Free(newItem->Cache_Controll);}
        if(newItem->EXT!=NULL){mem_Free(newItem->EXT);}
        if(newItem->Location!=NULL){mem_Free(newItem->Location);}
        if(newItem->Server_Name!=NULL){mem_Free(newItem->Server_Name);}
        if(newItem->Search_Target!=NULL){mem_Free(newItem->Search_Target);}
        if(newItem->Model!=NULL){mem_Free(newItem->Model);}
        if(newItem->DeviceType!=NULL){mem_Free(newItem->DeviceType);}
        if(newItem->DeviceID!=NULL){mem_Free(newItem->DeviceID);}
        if(newItem->UUID!=NULL){mem_Free(newItem->UUID);}
        mem_Free(newItem);
        return;
    }
    
    memcpy(newItem->Cache_Controll, cache_controll_hd_val, cache_controll_hd_val_len);
    newItem->Cache_Controll[cache_controll_hd_val_len] = '\0';
    memcpy(newItem->EXT, ext_hd_val, ext_hd_val_len);
    newItem->EXT[ext_hd_val_len] = '\0';
    memcpy(newItem->Location, location_hd_val, location_hd_val_len);
    newItem->Location[location_hd_val_len] = '\0';
    memcpy(newItem->Server_Name, server_name_hd_val, server_name_hd_val_len);
    newItem->Server_Name[server_name_hd_val_len] = '\0';
    memcpy(newItem->Search_Target, search_target_hd_val, search_target_hd_val_len);
    newItem->Search_Target[search_target_hd_val_len] = '\0';
    memcpy(newItem->Model, model_hd_val, model_hd_val_len);
    newItem->Model[model_hd_val_len] = '\0';
    memcpy(newItem->DeviceType, device_type_hd_val, device_type_hd_val_len);
    newItem->DeviceType[device_type_hd_val_len] = '\0';
    memcpy(newItem->DeviceID, device_ID_hd_val, device_ID_hd_val_len);
    newItem->DeviceID[device_ID_hd_val_len] = '\0';
    memcpy(newItem->UUID, uuid_hd_val, uuid_hd_val_len);
    newItem->UUID[uuid_hd_val_len] = '\0';
    
    uint8_t* search_target = newItem->Search_Target;
    
    if(strlen((const char*)newItem->Search_Target)==strlen(SSDP_All))
    {
        if(strncmp((const char*)newItem->Search_Target, SSDP_All, strlen(SSDP_All))==0)
        {
            newItem->Search_Target_Mode = SSDP_Search_Target_All;
        }
    }
    else{
        uint8_t* st_colon_tag = (uint8_t*)strchr((const char*)search_target, ':');
        if(st_colon_tag==NULL){mem_Free(newItem); return;}
        
        if((st_colon_tag-search_target)==strlen("device_type"))
        {
            if(strncmp((const char*)search_target, "device_type", strlen("device_type"))==0)
            {
                newItem->Search_Target_Mode = SSDP_Search_Target_Device_Type;
            }
        }
        if((st_colon_tag-search_target)==strlen("device_ID"))
        {
            if(strncmp((const char*)search_target, "device_ID", strlen("device_ID"))==0)
            {
                newItem->Search_Target_Mode = SSDP_Search_Target_Device_ID;
            }
        }
        if((st_colon_tag-search_target)==strlen("model"))
        {
            if(strncmp((const char*)search_target, "model", strlen("model"))==0)
            {
                newItem->Search_Target_Mode = SSDP_Search_Target_Model_Name;
            }
        }
        if((st_colon_tag-search_target)==strlen("uuid"))
        {
            if(strncmp((const char*)search_target, "uuid", strlen("uuid"))==0)
            {
                newItem->Search_Target_Mode = SSDP_Search_Target_UUID;
            }
        }
    }
    
    SSDP_Search_Result.num_found_nodes++;

    newItem->next = NULL;
    
    if(start==NULL)
    {
        SSDP_Search_Result.start = newItem;
    }
    else{
        last->next = newItem;
    }
}

void SSDP_Notify_Timeout_CB(NeonRTOS_TimerHandle ssdp_notify_timeout_timer_handle)
{
    SSDP_Send_Notify();
}

void SSDP_Delete_Search_Result()
{
    SSDP_Search_Response_Item* current = SSDP_Search_Result.start;
    SSDP_Search_Response_Item* del;
    while(current!=NULL)
    {
        del = current;
        current = current->next;
        if(del->Cache_Controll!=NULL){mem_Free(del->Cache_Controll);}
        if(del->EXT!=NULL){mem_Free(del->EXT);}
        if(del->Location!=NULL){mem_Free(del->Location);}
        if(del->Server_Name!=NULL){mem_Free(del->Server_Name);}
        if(del->Search_Target!=NULL){mem_Free(del->Search_Target);}
        if(del->Model!=NULL){mem_Free(del->Model);}
        if(del->DeviceType!=NULL){mem_Free(del->DeviceType);}
        if(del->DeviceID!=NULL){mem_Free(del->DeviceID);}
        if(del->UUID!=NULL){mem_Free(del->UUID);}
        mem_Free(del);
    }
    
    SSDP_Search_Result.num_found_nodes = 0;
    SSDP_Search_Result.start = NULL;
}

void SSDP_Search_Timeout_CB(NeonRTOS_TimerHandle ssdp_search_timeout_timer_handle)
{
    SSDP_Current_Status = SSDP_Status_KeepingResult;
    
    NeonRTOS_TimerStop(&ssdp_search_timeout_timer);
    
    if(On_Search_Nodes_Done_Callback!=NULL)
    {
        On_Search_Nodes_Done_Callback(&SSDP_Search_Result);
        On_Search_Nodes_Done_Callback = NULL;
    }
    
    NeonRTOS_TimerStart(&ssdp_search_result_del_timer);
}

void SSDP_Search_Result_Delete_Timeout_CB(NeonRTOS_TimerHandle ssdp_search_result_del_timer_handle)
{
    NeonRTOS_TimerStop(&ssdp_search_result_del_timer);
    
    SSDP_Delete_Search_Result();
    
    SSDP_Current_Status = SSDP_Status_Idle;
}

int SSDP_Get_Search_Result(cJSON *rsp_json)
{
	cJSON * ssdp_search_result = cJSON_CreateObject();
	if (NULL == ssdp_search_result) {
		return -1;
	}
	
	cJSON_AddItemToObject(rsp_json, "result", ssdp_search_result);
        
    switch(SSDP_Current_Status)
    {
        case SSDP_Status_Idle:
                cJSON_AddStringToObject(ssdp_search_result, "status", "idle");
                break;
        case SSDP_Status_Searching:
                cJSON_AddStringToObject(ssdp_search_result, "status", "searching");
                break;
        case SSDP_Status_KeepingResult:
                cJSON_AddStringToObject(ssdp_search_result, "status", "search done");
                
                cJSON * searched_node_lst = cJSON_CreateArray();
                if (NULL == searched_node_lst) {
                        //printf("pSubJson_rgb creat fail\n");
                        return -1;
                }
                
                cJSON_AddItemToObject(ssdp_search_result, "Searched_Nodes", searched_node_lst);
        
                cJSON * searched_node_individual;
                
                SSDP_Search_Response_Item* currentNode = SSDP_Search_Result.start;
                
                for (uint8_t i = 0; (i < SSDP_Search_Result.num_found_nodes && currentNode!=NULL); i++)
                {
                        searched_node_individual = cJSON_CreateObject();
                        if (searched_node_individual == NULL)
                        {
                                return -1;
                        }
                        
                        switch(currentNode->Search_Target_Mode)
                        {
                        case SSDP_Search_Target_All:
                            cJSON_AddStringToObject(searched_node_individual, "Search_Target_Mode", "All");
                            break;
                        case SSDP_Search_Target_Device_Type:
                            cJSON_AddStringToObject(searched_node_individual, "Search_Target_Mode", "Specific Device Type");
                            break;
                        case SSDP_Search_Target_Device_ID:
                            cJSON_AddStringToObject(searched_node_individual, "Search_Target_Mode", "Specific Device ID");
                            break;
                        case SSDP_Search_Target_Model_Name:
                            cJSON_AddStringToObject(searched_node_individual, "Search_Target_Mode", "Specific Model");
                            break;
                        case SSDP_Search_Target_UUID:
                            cJSON_AddStringToObject(searched_node_individual, "Search_Target_Mode", "UUID");
                            break;
                        default:
                            cJSON_Delete(searched_node_individual);
                            return -1;
                        }
                        
                        cJSON_AddStringToObject(searched_node_individual, "Cache_Control", (char*)currentNode->Cache_Controll);
                        cJSON_AddStringToObject(searched_node_individual, "EXT", (char*)currentNode->EXT);
                        cJSON_AddStringToObject(searched_node_individual, "Location", (char*)currentNode->Location);
                        cJSON_AddStringToObject(searched_node_individual, "Server_Name", (char*)currentNode->Server_Name);
                        cJSON_AddStringToObject(searched_node_individual, "Search_Target", (char*)currentNode->Search_Target);
                        cJSON_AddStringToObject(searched_node_individual, "Model", (char*)currentNode->Model);
                        cJSON_AddStringToObject(searched_node_individual, "DeviceType", (char*)currentNode->DeviceType);
                        cJSON_AddStringToObject(searched_node_individual, "DeviceID", (char*)currentNode->DeviceID);
                        cJSON_AddStringToObject(searched_node_individual, "UUID", (char*)currentNode->UUID);
                        
                        cJSON_AddItemToArray(searched_node_lst, searched_node_individual);
                        
                        currentNode = currentNode->next;
                }
                break;
        default:
                cJSON_AddStringToObject(ssdp_search_result, "status", "unknown");
                break;
    }
	return 0;
}

int Process_SSDP_JSON_POST_Msg(cJSON* message)
{
	int status = -1;
	cJSON* cmd = cJSON_GetObjectItem(message, "command");
	
	if (cmd != NULL && cmd->type == cJSON_String)
	{
		if (strcmp(cmd->valuestring, "search nearby devices") == 0)
		{
            status = 0;
            SSDP_Search_Nodes(SSDP_Search_Target_All, NULL, SSDP_SEARCHNEARBY_TIME, NULL);
		}
		if (strcmp(cmd->valuestring, "search nearby same model nodes") == 0)
		{
            status = 0;
            SSDP_Search_Nodes(SSDP_Search_Target_Model_Name, (uint8_t*)SSDP_ModelName, SSDP_SEARCHNEARBY_TIME, NULL);
		}
		if (strcmp(cmd->valuestring, "search WiFiIOT specific type node") == 0)
		{
			cJSON* scan_device_type = cJSON_GetObjectItem(message, "device_type");
			if (scan_device_type != NULL && scan_device_type->type == cJSON_String)
			{
                status = 0;
                SSDP_Search_Nodes(SSDP_Search_Target_Device_Type, (uint8_t*)scan_device_type->valuestring, SSDP_SEARCHNEARBY_TIME, NULL);
			}
		}
	}
	return status;
}

int Process_SSDP_JSON_GET_Msg(cJSON* message, cJSON* rsp_json)
{
	int status = -1;
	cJSON* cmd = cJSON_GetObjectItem(message, "command");
	
	if (cmd != NULL && cmd->type == cJSON_String)
	{
		if (strcmp(cmd->valuestring, "get search result") == 0)
		{
			status = SSDP_Get_Search_Result(rsp_json);
			cJSON_AddStringToObject(rsp_json, "command", "get search result");
		}
	}
	return status;
}

int SSDP_CGI_Root(HTTPd_WebSocked_Client_Connection *connData)
{
    if(connData==NULL) return HTTPD_CGI_DONE;
    
    int status = HTTPD_CGI_DONE;
    
    if(HTTPd_Get_CGI_Request_Type(connData)==HTTPd_Method_POST)
    {
          //search nearby
          
          const uint8_t* req_data = HTTPd_Get_CGI_Request_Data(connData);
          if(req_data==NULL)
          {
                  return HTTPd_Send_CGI_Response(connData, 400, "txt", NULL, 0);
          }
          
          cJSON* req_json = cJSON_Parse((const char*)req_data);
          if (req_json==NULL)
          {
                  return HTTPd_Send_CGI_Response(connData, 422, "txt", NULL, 0);
          }
          
          cJSON* same_model = cJSON_GetObjectItem(req_json, "same_model");
          if (same_model == NULL)
          {
                  cJSON_Delete(req_json);
                  return HTTPd_Send_CGI_Response(connData, 400, "txt", NULL, 0);
          }
          
          cJSON* device_type = cJSON_GetObjectItem(req_json, "device_type");
          
          if(same_model->type == cJSON_True)
          {
                  SSDP_Search_Nodes(SSDP_Search_Target_Model_Name, (uint8_t*)SSDP_ModelName, SSDP_SEARCHNEARBY_TIME, NULL);
          }
          else if(same_model->type == cJSON_False)
          {
                  if(device_type!=NULL)
                  {
                        if(device_type->type == cJSON_String)
                        {
                              cJSON_Delete(req_json);
                              return HTTPd_Send_CGI_Response(connData, 400, "txt", NULL, 0);
                        }
                        
                        SSDP_Search_Nodes(SSDP_Search_Target_Device_Type, (uint8_t*)device_type->valuestring, SSDP_SEARCHNEARBY_TIME, NULL);
                  }
                  else{
                        SSDP_Search_Nodes(SSDP_Search_Target_All, NULL, SSDP_SEARCHNEARBY_TIME, NULL);
                  }
          }
          else{
                  cJSON_Delete(req_json);
                  return HTTPd_Send_CGI_Response(connData, 400, "txt", NULL, 0);
          }
          
          cJSON_Delete(req_json);
          
          cJSON* rsp_json = cJSON_CreateObject();
          if(rsp_json==NULL)
          {
                return HTTPd_Send_CGI_Response(connData, 500, "txt", NULL, 0);
          }
          
          if(same_model->type == cJSON_True)
          {
                cJSON_AddTrueToObject(rsp_json, "same_model");
          }
          else if(same_model->type == cJSON_False)
          {
                cJSON_AddFalseToObject(rsp_json, "same_model");
                
                if(device_type!=NULL)
                {
                      cJSON_AddStringToObject(rsp_json, "device_type", device_type->valuestring);
                }
                else{
                      cJSON_AddStringToObject(rsp_json, "device_type", "Not Specific");
                }
          }
          
          status = HTTPd_Send_CGI_JSON_Response(connData, 201, rsp_json, true);
    }
    else if(HTTPd_Get_CGI_Request_Type(connData)==HTTPd_Method_GET)
    {
          cJSON* rsp_json = cJSON_CreateObject();
          if(rsp_json==NULL)
          {
                return HTTPd_Send_CGI_Response(connData, 500, "txt", NULL, 0);
          }
          
          if(SSDP_Get_Search_Result(rsp_json)<0)
          {
                cJSON_Delete(rsp_json);
                return HTTPd_Send_CGI_Response(connData, 400, "txt", NULL, 0);
          }
          
          status = HTTPd_Send_CGI_JSON_Response(connData, 200, rsp_json, true);
    }
    else{
          status = HTTPd_Send_CGI_Response(connData, 404, "txt", NULL, 0);
    }
    
    return status;
}

void SSDP_Task(void* param)
{
	int32_t ret;
	struct sockaddr_in server_addr;
    struct sockaddr_in from;
    const socklen_t fromlen = sizeof(struct sockaddr_in);
    
    uint8_t mac[6];
    uint8_t mac_no_colon_str[13];
    ethernetif_get_hardware_mac(mac);
    sprintf(mac_no_colon_str, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    char* uuid_ptr = SSDP_Self_UUID;
    
    uuid_ptr+=sprintf(SSDP_Self_UUID, "38323636-4558-4dda-9188-cda0e6%s", mac_no_colon_str);
    
    *uuid_ptr++ = '\0';
        
	memset(&server_addr, 0, sizeof(server_addr)); /* Zero out structure */
	server_addr.sin_family = AF_INET;            /* Internet address family */
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any incoming interface */
	server_addr.sin_len = sizeof(server_addr);
	server_addr.sin_port = htons(SSDP_PORT); /* Local port */
	
	SSDP_Socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (SSDP_Socket < 0)
    {
        NeonRTOS_TimerStop(&ssdp_notify_timeout_timer);
        NeonRTOS_TimerDelete(&ssdp_notify_timeout_timer);
        
        close(SSDP_Socket);
        SSDP_Socket = -1;
        
        NeonRTOS_TimerStop(&ssdp_search_timeout_timer);
        NeonRTOS_TimerDelete(&ssdp_search_timeout_timer);
        
        NeonRTOS_TimerStop(&ssdp_search_result_del_timer);
        NeonRTOS_TimerDelete(&ssdp_search_result_del_timer);
        
        NeonRTOS_TaskDelete(NULL);
	}
	
	ret = bind(SSDP_Socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
	if (ret < 0)
    {
        NeonRTOS_TimerStop(&ssdp_notify_timeout_timer);
        NeonRTOS_TimerDelete(&ssdp_notify_timeout_timer);
        
        close(SSDP_Socket);
        SSDP_Socket = -1;
        
        NeonRTOS_TimerStop(&ssdp_search_timeout_timer);
        NeonRTOS_TimerDelete(&ssdp_search_timeout_timer);
        
        NeonRTOS_TimerStop(&ssdp_search_result_del_timer);
        NeonRTOS_TimerDelete(&ssdp_search_result_del_timer);
        
        NeonRTOS_TaskDelete(NULL);
	}
        
    int recvBlockTime = 500;
    ret = setsockopt(SSDP_Socket, SOL_SOCKET, SO_RCVTIMEO, &recvBlockTime, sizeof(recvBlockTime));
	if (ret < 0)
    {
        NeonRTOS_TimerStop(&ssdp_notify_timeout_timer);
        NeonRTOS_TimerDelete(&ssdp_notify_timeout_timer);
        
        close(SSDP_Socket);
        SSDP_Socket = -1;
        
        NeonRTOS_TimerStop(&ssdp_search_timeout_timer);
        NeonRTOS_TimerDelete(&ssdp_search_timeout_timer);
        
        NeonRTOS_TimerStop(&ssdp_search_result_del_timer);
        NeonRTOS_TimerDelete(&ssdp_search_result_del_timer);
        
        NeonRTOS_TaskDelete(NULL);
	}
    
    //printf("SSDP Socket ID:%d\n", SSDP_Socket);
    
    NeonRTOS_TimerStart(&ssdp_notify_timeout_timer);
    
    while(1)
    {
        if(NeonTCPIP_IF_isLinkUp())
        {
#if LWIP_DHCP
            if(NeonTCPIP_Get_DHCP_State()==DHCP_ADDRESS_ASSIGNED)
#endif
            {
                ip_addr_t ssdp_host_addr;
                ssdp_host_addr.addr = NeonTCPIP_IF_Get_IP_Address();
                
                if (igmp_joingroup(&ssdp_host_addr, &multicast_addr) != ERR_OK)
                {
                    NeonRTOS_Sleep(100);
                    //printf("udp_join_multigrup failed!\n");
                    continue;
                }
            }
        }

        uint8_t* ssdp_hd_buf = mem_Malloc(SSDP_HEADER_BUF_SIZE);
        if (ssdp_hd_buf == NULL)
        {
            continue;
        }
        memset(ssdp_hd_buf, 0, SSDP_HEADER_BUF_SIZE);
        
        ret = recvfrom(SSDP_Socket, ssdp_hd_buf, SSDP_HEADER_BUF_SIZE, 0, (struct sockaddr *)&from,(socklen_t *)&fromlen);
        
        if (ret < 0 || strstr((char*)ssdp_hd_buf, "\r\n\r\n")==NULL)
        {
            mem_Free(ssdp_hd_buf);
            continue;
        }
        
        SSDP_Method SSDP_Header_Method = SSDP_Method_UNKNOWN;
        
        uint8_t* method_str_ptr;
        uint8_t* first_line_end;
        first_line_end = (uint8_t*)strstr((char*)ssdp_hd_buf, "\r\n");
        if (first_line_end == NULL)
        {
            mem_Free(ssdp_hd_buf);
            continue;
        }
        
        method_str_ptr = (uint8_t*)strstr((char*)ssdp_hd_buf, SSDP_HTTP_Method_NOTIFY);
        if (method_str_ptr != NULL && method_str_ptr < first_line_end)
        {
            SSDP_Header_Method = SSDP_Method_NOTIFY;
        }
        
        method_str_ptr = (uint8_t*)strstr((char*)ssdp_hd_buf, SSDP_HTTP_Method_M_SEARCH);
        if (method_str_ptr != NULL && method_str_ptr < first_line_end)
        {
            SSDP_Header_Method = SSDP_Method_M_SEARCH;
        }
        
        uint8_t* header_line_start = first_line_end + strlen("\r\n");
        
        uint8_t* ssdp_http_uri;
        uint8_t* end_uri;
        
        switch(SSDP_Header_Method)
        {
            case SSDP_Method_NOTIFY:
            case SSDP_Method_M_SEARCH:;
                ssdp_http_uri = HTTPd_Get_Header_Path(ssdp_hd_buf);
                end_uri = (uint8_t*)strstr((const char*)ssdp_http_uri, " ");
                if (end_uri == NULL) break; //wtf? should not happened
                *end_uri = '\0'; //terminate url part
                if(strncmp((const char*)ssdp_http_uri, "*", 1)!=0) break;//invalid ssdp header URI
                
                switch(SSDP_Header_Method)
                {
                case SSDP_Method_NOTIFY:
                    //Process SSDP Server Notify Event
                    SSDP_Handle_Notify_Event(header_line_start, &from);
                    break;
                case SSDP_Method_M_SEARCH:
                    //Process SSDP Client Search Event
                    SSDP_Handle_Search_Request(header_line_start, &from);
                    break;
                }
                break;
            case SSDP_Method_UNKNOWN:
                //Process SSDP Server Search Response Event
                //This is response message due to no header method was found
                SSDP_Handle_Search_Response(header_line_start, &from);
                break;
        }
            
        mem_Free(ssdp_hd_buf);
    }
    
    NeonRTOS_TimerStop(&ssdp_notify_timeout_timer);
    NeonRTOS_TimerDelete(&ssdp_notify_timeout_timer);
    
    if(SSDP_Socket>=0)
    {
        close(SSDP_Socket);
        SSDP_Socket = -1;
    }
    
    NeonRTOS_TimerStop(&ssdp_search_timeout_timer);
    NeonRTOS_TimerDelete(&ssdp_search_timeout_timer);
    
    NeonRTOS_TimerStop(&ssdp_search_result_del_timer);
    NeonRTOS_TimerDelete(&ssdp_search_result_del_timer);
    
    NeonRTOS_TaskDelete(NULL);
}

int SSDP_Init()
{
    SSDP_Current_Status = SSDP_Status_Idle;
    SSDP_Search_Result.num_found_nodes = 0;
    SSDP_Search_Result.start = NULL;
    
    /* initialize default SSDP server address */
    multicast_addr.addr = ipaddr_addr(SSDP_MULTICAST_ADDRESS);
    
    if(NeonRTOS_TimerCreate(&ssdp_search_timeout_timer, "SSDP Search Timeout", SSDP_DEFAULT_SEARCH_TIMEOUT, 1, 0, SSDP_Search_Timeout_CB)!=NeonRTOS_OK)
    {
          return -1;
    }
    NeonRTOS_TimerStop(&ssdp_search_timeout_timer);
    
    if(NeonRTOS_TimerCreate(&ssdp_search_result_del_timer, "SSDP Search Rseult Selete", SSDP_SEARCH_RESULT_DEL_TIME, 1, 0, SSDP_Search_Result_Delete_Timeout_CB)!=NeonRTOS_OK)
    {
        NeonRTOS_TimerDelete(&ssdp_search_timeout_timer);
        return -1;
    }
    NeonRTOS_TimerStop(&ssdp_search_result_del_timer);
    
    if(NeonRTOS_TimerCreate(&ssdp_notify_timeout_timer, "SSDP Notify Timeout", SSDP_NOTIFY_INTERVAL, 1, 0, SSDP_Notify_Timeout_CB)!=NeonRTOS_OK)
    {
        NeonRTOS_TimerDelete(&ssdp_search_timeout_timer);
        NeonRTOS_TimerDelete(&ssdp_search_result_del_timer);
        return -1;
    }
    NeonRTOS_TimerStop(&ssdp_notify_timeout_timer);
    
    if(NeonRTOS_TaskCreate(SSDP_Task, (signed const char *)"SSDP", SSDP_TASK_STACK_SIZE, NULL, SSDP_TASK_PRIORITY, NULL)!=NeonRTOS_OK)
    {
        NeonRTOS_TimerDelete(&ssdp_search_timeout_timer);
        NeonRTOS_TimerDelete(&ssdp_search_result_del_timer);
        NeonRTOS_TimerDelete(&ssdp_notify_timeout_timer);
        return -1;
    }
    
    HTTPd_Register_CGI_URL_Callback("/SSDP", SSDP_CGI_Root, NULL);
        
    return 0;
}
