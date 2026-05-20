
#ifndef NFC_DEF_H
#define NFC_DEF_H

typedef enum {
    NFC_OK = 0,
    NFC_NotInit = -1,
    NFC_InvalidParameter = -2,
    NFC_MemoryError = -3,
    NFC_MemoryCorrupt = -3,
    NFC_MutexTimeout = -4,
    NFC_SlaveTimeout = -5,
    NFC_IO_Error = -6,
    NFC_Syntax = -6,
    NFC_Busy = -6,
    NFC_Again = -7,
    NFC_Ignore = -8,
    NFC_NotFound = -8,
    NFC_Hw_Mismatch = -9,
    NFC_Hw_OverRun = -10,
    NFC_System = -11,
    NFC_WrongState = -12,
    NFC_CRC_Error = -13,
    NFC_ParityError = -14,
    NFC_CalibrateError = -15,
    NFC_FrameTimeout = -16,
    NFC_FramingError = -17,
    NFC_RF_Collision = -18,
    NFC_LinkLoss = -19,
    NFC_InternalError = -20,
    NFC_RequestError = -21,
    NFC_ImcompleteByte = -22,
    NFC_ProtocolError = -23,
    NFC_WriteFailed = -24,
    NFC_SleepRequest = -25,
    NFC_ReleaseRequest = -26,
    NFC_Unsupport = -27
} NFC_OpResult;

#endif // NFC_DEF_H