
#ifndef NFC_DEF_H
#define NFC_DEF_H

typedef enum {
    NFC_OK = 0,
    NFC_NotInit = -1,
    NFC_InvalidParameter = -2,
    NFC_MemoryError = -3,
    NFC_MutexTimeout = -4,
    NFC_SlaveTimeout = -5,
    NFC_IO_Error = -6,
    NFC_Busy = -6,
    NFC_Again = -7,
    NFC_Hw_Mismatch = -8,
    NFC_Hw_OverRun = -9,
    NFC_System = -10,
    NFC_WrongState = -11,
    NFC_CRC_Error = -12,
    NFC_ParityError = -13,
    NFC_CalibrateError = -14,
    NFC_FrameTimeout = -15,
    NFC_FramingError = -16,
    NFC_RF_Collision = -17,
    NFC_LinkLoss = -18,
    NFC_InternalError = -19,
    NFC_RequestError = -20,
    NFC_ImcompleteByte = -21,
    NFC_Unsupport = -22
} NFC_OpResult;

#endif // NFC_DEF_H