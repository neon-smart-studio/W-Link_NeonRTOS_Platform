
#ifndef BLUETOOTH_DEF_H
#define BLUETOOTH_DEF_H

#define BDADDR_SIZE 6
#define BNAME_LEN   33

#define BNAME_DEFAULT "NeonSmart"

typedef enum Bluetooth_OpResult_t
{
  Bluetooth_OK = 0,
  Bluetooth_NotInit = -1,
  Bluetooth_InvalidParameter = -2,
  Bluetooth_Busy = -3,
  Bluetooth_MemoryError = -4,
  Bluetooth_HwError = -5,
  Bluetooth_HCI_Error = -6,
  Bluetooth_Unsupport = -7,
}Bluetooth_OpResult;

typedef enum Bluetooth_PA_Level_t
{
  Bluetooth_PA_Level_0 = 0,
  Bluetooth_PA_Level_1 = 1,
  Bluetooth_PA_Level_2 = 2,
  Bluetooth_PA_Level_3 = 3,
  Bluetooth_PA_Level_4 = 4,
  Bluetooth_PA_Level_5 = 5,
  Bluetooth_PA_Level_6 = 6,
  Bluetooth_PA_Level_7 = 7,
  Bluetooth_PA_Level_MAX,
}Bluetooth_PA_Level;

typedef enum Bluetooth_UUID_Type_t
{
  Bluetooth_UUID_Type_16 = 0,
  Bluetooth_UUID_Type_128,
  Bluetooth_UUID_Type_MAX
}Bluetooth_UUID_Type;

typedef enum Bluetooth_Service_Type_t
{
  Bluetooth_Service_Type_Primary = 0,
  Bluetooth_Service_Type_Secondary,
  Bluetooth_Service_Type_MAX
}Bluetooth_Service_Type;

typedef enum Bluetooth_Char_Property_t
{
    Bluetooth_Char_Property_Broadcast         = 0x01,
    Bluetooth_Char_Property_Read              = 0x02,
    Bluetooth_Char_Property_WriteWithoutResp  = 0x04,
    Bluetooth_Char_Property_Write             = 0x08,
    Bluetooth_Char_Property_Notify            = 0x10,
    Bluetooth_Char_Property_Indicate          = 0x20,
    Bluetooth_Char_Property_SignedWrite       = 0x40,
    Bluetooth_Char_Property_Extended          = 0x80,
} Bluetooth_Char_Property;

typedef enum Bluetooth_Char_Permission_t
{
    Bluetooth_Char_Permission_None            = 0x00,

    Bluetooth_Char_Permission_Authen_Read    = 0x01,
    Bluetooth_Char_Permission_Author_Read    = 0x02,
    Bluetooth_Char_Permission_Encrypt_Read   = 0x04,

    Bluetooth_Char_Permission_Authen_Write   = 0x08,
    Bluetooth_Char_Permission_Author_Write   = 0x10,
    Bluetooth_Char_Permission_Encrypt_Write  = 0x20,

} Bluetooth_Char_Permission;

typedef enum Bluetooth_Attr_Access_t
{
    Bluetooth_Attr_Access_ReadOnly  = 0x01,
    Bluetooth_Attr_Access_WriteOnly = 0x02,
    Bluetooth_Attr_Access_ReadWrite = 0x03,
} Bluetooth_Attr_Access;

#endif //BLUETOOTH_DEF_H