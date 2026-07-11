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

#ifndef SSDP_H
#define SSDP_H

#include "NeonRTOS.h"

#define SSDP_NOTIFY_INTERVAL             30000
#define SSDP_DEFAULT_SEARCH_TIMEOUT      1000
#define SSDP_SEARCHNEARBY_TIME           5000
#define SSDP_SEARCH_RESULT_DEL_TIME      5000
#define SSDP_UUID_SIZE                   43
#define SSDP_DEVICE_TYPE_SIZE            64
#define SSDP_SERVICE_URL_SIZE            64
#define SSDP_MODEL_NAME_SIZE             64
#define SSDP_MODEL_VERSION_SIZE          32
#define SSDP_MANUFACTURER_NAME_SIZE      64
#define SSDP_MANUFACTURER_URL_SIZE       64

typedef enum SSDP_Status_t
{
    SSDP_Status_Idle = 0,
    SSDP_Status_Searching = 1,
    SSDP_Status_KeepingResult = 2,
    SSDP_Status_MAX = 3
}SSDP_Status;

typedef enum SSDP_Method_t
{
    SSDP_Method_NOTIFY = 11,
    SSDP_Method_M_SEARCH = 12,
    SSDP_Method_UNKNOWN = 0,
}SSDP_Method;

typedef enum SSDP_Search_Target_Type_t
{
    SSDP_Search_Target_None = -1,
    SSDP_Search_Target_All = 0,
    SSDP_Search_Target_Device_Type = 1,
    SSDP_Search_Target_Device_ID = 2,
    SSDP_Search_Target_Model_Name = 3,
    SSDP_Search_Target_UUID = 4,
    SSDP_Search_Target_MAX = 5,
}SSDP_Search_Target_Type;

typedef struct SSDP_Notify_Inf_t
{
    uint8_t* Host_Address;
    uint8_t* Cache_Controll;
    uint8_t* Location;
    uint8_t* Notify_Target;
    uint8_t* Notify_State;
    uint8_t* Server_Name;
    uint8_t* Model;
    uint8_t* DeviceType;
    uint8_t* DeviceID;
    uint8_t* UUID;
}SSDP_Notify_Inf;

typedef struct SSDP_Search_Response_Item_t
{
    uint8_t* Cache_Controll;
    uint8_t* EXT;
    uint8_t* Location;
    uint8_t* Server_Name;
    SSDP_Search_Target_Type Search_Target_Mode;
    uint8_t* Search_Target;
    uint8_t* Model;
    uint8_t* DeviceType;
    uint8_t* DeviceID;
    uint8_t* UUID;
    struct SSDP_Search_Response_Item_t* next;
}SSDP_Search_Response_Item;

typedef struct SSDP_Search_Node_List_t
{
    uint16_t num_found_nodes;
    SSDP_Search_Response_Item* start;
}SSDP_Search_Node_List;

typedef uint8_t*(*SSDP_On_Send_Notify_Ind_CB)(void);//return data buf will be free automatic
typedef void(*SSDP_On_Recv_Notify_Ind_CB)(SSDP_Notify_Inf* notify_inf, uint8_t* headers_buf);
typedef uint8_t*(*SSDP_On_Recv_Search_Req_CB)(void);//return data buf will be free automatic
typedef void(*SSDP_On_Search_Nodes_Done_CB)(SSDP_Search_Node_List* ssdp_search_result);

int SSDP_Init();

void SSDP_Register_OnSendNotifyIndCallBack(SSDP_On_Send_Notify_Ind_CB cb);
void SSDP_Register_OnRecvNotifyIndCallback(SSDP_On_Recv_Notify_Ind_CB cb);
void SSDP_Register_OnRecvSearchReqCallBack(SSDP_On_Recv_Search_Req_CB cb);
void SSDP_Search_Nodes(SSDP_Search_Target_Type SSDP_stt, uint8_t* SSDP_search_target, NeonRTOS_Time_t search_time, SSDP_On_Search_Nodes_Done_CB cb);
uint8_t* SSDP_Parse_Reguest_Method(uint8_t* buf, SSDP_Method req_method, uint8_t* req_path);
uint8_t* SSDP_Parse_Headers(uint8_t* buf, const char* header_str, const char* header_val);
uint8_t* SSDP_Parse_End_Of_Headers(uint8_t* buf);
uint8_t* SSDP_Get_Header_Value(uint8_t* buf, uint8_t* header_str);
void SSDP_SetServicePort(uint16_t port);
void SSDP_SetServiceURL(const char *url);
void SSDP_SetDeviceType(const char *deviceType);
void SSDP_SetModelName(const char *name);
void SSDP_SetModelVersion(const char *num);
void SSDP_SetManufacturerName(const char *name);
void SSDP_SetManufacturerURL(const char *url);

int Process_SSDP_JSON_POST_Msg(cJSON* message);
int Process_SSDP_JSON_GET_Msg(cJSON* message, cJSON* rsp_json);

#endif //SSDP_H