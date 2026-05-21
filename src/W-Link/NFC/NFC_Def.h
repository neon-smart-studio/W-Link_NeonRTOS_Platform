
#ifndef NFC_DEF_H
#define NFC_DEF_H

typedef enum {
    NFC_OK = 0,
    NFC_NotInit = -1,
    NFC_InvalidParameter = -2,
    NFC_MemoryError = -3,
    NFC_MemoryCorrupt = -4,
    NFC_MutexTimeout = -5,
    NFC_SlaveTimeout = -6,
    NFC_IO_Error = -7,
    NFC_Syntax = -8,
    NFC_Busy = -9,
    NFC_Again = -10,
    NFC_Ignore = -11,
    NFC_NotFound = -12,
    NFC_Hw_Mismatch = -13,
    NFC_Hw_OverRun = -14,
    NFC_System = -15,
    NFC_WrongState = -16,
    NFC_CRC_Error = -17,
    NFC_ParityError = -18,
    NFC_CalibrateError = -19,
    NFC_FrameTimeout = -20,
    NFC_FramingError = -21,
    NFC_RF_Collision = -22,
    NFC_LinkLoss = -23,
    NFC_InternalError = -24,
    NFC_RequestError = -25,
    NFC_ImcompleteByte = -26,
    NFC_ProtocolError = -27,
    NFC_WriteFailed = -28,
    NFC_SleepRequest = -29,
    NFC_ReleaseRequest = -30,
    NFC_Unsupport = -31
} NFC_OpResult;

#endif // NFC_DEF_H