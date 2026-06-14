
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"

#include "DMA/DMA.h"

#include "NeonRTOS.h"

#ifdef DEVICE_NUVOTON

hwDMA_OpResult DMA_Init()
{
    return hwDMA_OK;
}

hwDMA_OpResult DMA_DeInit()
{
    return hwDMA_OK;
}

#endif //DEVICE_NUVOTON
