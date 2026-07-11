
#ifndef NEONAPPINTERFACE_H
#define NEONAPPINTERFACE_H

#include "Utils/cJSON/cJSON.h"

#include "GPIO/GPIO.h"

#define USER_APP_MODEL_NAME    "NeonRT"
#define USER_APP_VERSION       "V1.0.0"
#define MANUFACTURE_NAME       "Neon Smart"
#define USER_APP_DEVIXE_TYPE   "W-Link"

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum App_Interface_Protocol_t
{
    None = 0,
    WebSocket = 1,
    Telnet = 2,
    MQTT = 3
}App_Interface_Protocol;

typedef int(*APP_Interface_Recv_This_Node_POST_Msg_CB)(App_Interface_Protocol protocol, cJSON* in_msg);
typedef int(*APP_Interface_Recv_This_Node_GET_Msg_CB)(App_Interface_Protocol protocol, cJSON* in_msg, cJSON* rsp_msg);

void Neon_App_Init(void);
int Register_Neon_APP_Interface_Msg_CallBack(const char* topic, APP_Interface_Recv_This_Node_POST_Msg_CB postCB, APP_Interface_Recv_This_Node_GET_Msg_CB getCB);
int UnRegister_Neon_APP_Interface_Msg_CallBack(const char* topic);

extern int Neon_APP_Device_Init();

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //NEONAPPINTERFACE_H