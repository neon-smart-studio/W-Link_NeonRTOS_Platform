
#ifndef ETHERNET_DEF_H
#define ETHERNET_DEF_H

#include <stdbool.h>
#include <stdint.h>

typedef enum hwEthernet_OpResult_t
{
  hwEthernet_OK = 0,
  hwEthernet_NotInit = -1,
  hwEthernet_InvalidParameter = -2,
  hwEthernet_Busy = -3,
  hwEthernet_MemoryError = -4,
  hwEthernet_BufferError = -5,
  hwEthernet_HwError = -6,
  hwEthernet_Unsupport = -7,
}hwEthernet_OpResult;

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //ETHERNET_DEF_H