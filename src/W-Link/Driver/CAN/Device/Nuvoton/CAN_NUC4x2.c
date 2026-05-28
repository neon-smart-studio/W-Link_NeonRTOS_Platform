#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "CAN/CAN.h"

#if defined(NUC442) || defined(NUC472)

#include "GPIO/Device/Nuvoton/GPIO_Nuvoton.h"

#include "CAN/Pin/Nuvoton/CAN_Pin_Nuvoton.h"

#include "CAN_Nuvoton.h"

#if defined(CAN0_BASE)
void CAN0_IRQHandler(void)
{
    uint32_t status = CAN_GET_INT_PENDING_STATUS(CAN0);

    if(status & CAN_STATUS_TXOK_Msk)
    {
        CAN_CLR_INT_PENDING_BIT(CAN0, CAN_STATUS_TXOK_Msk);
        CAN_TxMailbox0CompleteCallback(hwCAN_Index_0);
    }

    if(status & CAN_STATUS_RXOK_Msk)
    {
        uint8_t data[8] = {0};

        CAN_Receive(CAN0, 0, data);
        CAN_CLR_INT_PENDING_BIT(CAN0, CAN_STATUS_RXOK_Msk);

        CAN_RxFifo0MsgPendingCallback(hwCAN_Index_0, data);
    }
}
#endif

#if defined(CAN1_BASE)
void CAN1_IRQHandler(void)
{
    uint32_t status = CAN_GET_INT_PENDING_STATUS(CAN1);

    if(status & CAN_STATUS_TXOK_Msk)
    {
        CAN_CLR_INT_PENDING_BIT(CAN1, CAN_STATUS_TXOK_Msk);
        CAN_TxMailbox0CompleteCallback(hwCAN_Index_1);
    }

    if(status & CAN_STATUS_RXOK_Msk)
    {
        uint8_t data[8] = {0};

        CAN_Receive(CAN1, 0, data);
        CAN_CLR_INT_PENDING_BIT(CAN1, CAN_STATUS_RXOK_Msk);

        CAN_RxFifo0MsgPendingCallback(hwCAN_Index_1, data);
    }
}
#endif

CAN_T *CAN_Map_Soc_Base(hwCAN_Index index)
{
    switch(index)
    {
#if defined(CAN0_BASE)
        case hwCAN_Index_0:
            return CAN0;
#endif

#if defined(CAN1_BASE)
        case hwCAN_Index_1:
            return CAN1;
#endif

        default:
            return NULL;
    }
}

void CAN_GPIO_ConfigAF(hwCAN_Index index)
{
    CAN_Pin_Def def = CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]];

    SYS_UnlockReg();

    switch(index)
    {
#if defined(CAN0_BASE)
        case hwCAN_Index_0:
            if(def.tx_pin == hwGPIO_Pin_A12 && def.rx_pin == hwGPIO_Pin_A11)
            {
                SYS->GPA_MFPH =
                    (SYS->GPA_MFPH &
                    ~(SYS_GPA_MFPH_PA12MFP_Msk |
                      SYS_GPA_MFPH_PA11MFP_Msk)) |
                    (0x2UL << SYS_GPA_MFPH_PA12MFP_Pos) |
                    (0x2UL << SYS_GPA_MFPH_PA11MFP_Pos);
            }
            else if(def.tx_pin == hwGPIO_Pin_B9 && def.rx_pin == hwGPIO_Pin_B8)
            {
                SYS->GPB_MFPH =
                    (SYS->GPB_MFPH &
                    ~(SYS_GPB_MFPH_PB9MFP_Msk |
                      SYS_GPB_MFPH_PB8MFP_Msk)) |
                    (0x2UL << SYS_GPB_MFPH_PB9MFP_Pos) |
                    (0x2UL << SYS_GPB_MFPH_PB8MFP_Pos);
            }
            break;
#endif

#if defined(CAN1_BASE)
        case hwCAN_Index_1:
            if(def.tx_pin == hwGPIO_Pin_E7 && def.rx_pin == hwGPIO_Pin_E6)
            {
                SYS->GPE_MFPL =
                    (SYS->GPE_MFPL &
                    ~(SYS_GPE_MFPL_PE7MFP_Msk |
                      SYS_GPE_MFPL_PE6MFP_Msk)) |
                    (0x2UL << SYS_GPE_MFPL_PE7MFP_Pos) |
                    (0x2UL << SYS_GPE_MFPL_PE6MFP_Pos);
            }
            else if(def.tx_pin == hwGPIO_Pin_D7 && def.rx_pin == hwGPIO_Pin_D6)
            {
                SYS->GPD_MFPL =
                    (SYS->GPD_MFPL &
                    ~(SYS_GPD_MFPL_PD7MFP_Msk |
                      SYS_GPD_MFPL_PD6MFP_Msk)) |
                    (0x2UL << SYS_GPD_MFPL_PD7MFP_Pos) |
                    (0x2UL << SYS_GPD_MFPL_PD6MFP_Pos);
            }
            break;
#endif

        default:
            break;
    }

    SYS_LockReg();
}

void CAN_GPIO_DeConfigAF(hwCAN_Index index)
{
    CAN_Pin_Def def = CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]];

    SYS_UnlockReg();

#if defined(CAN0_BASE)
    if(def.tx_pin == hwGPIO_Pin_A12 || def.rx_pin == hwGPIO_Pin_A11)
    {
        SYS->GPA_MFPH &=
            ~(SYS_GPA_MFPH_PA12MFP_Msk |
              SYS_GPA_MFPH_PA11MFP_Msk);
    }

    if(def.tx_pin == hwGPIO_Pin_B9 || def.rx_pin == hwGPIO_Pin_B8)
    {
        SYS->GPB_MFPH &=
            ~(SYS_GPB_MFPH_PB9MFP_Msk |
              SYS_GPB_MFPH_PB8MFP_Msk);
    }
#endif

#if defined(CAN1_BASE)
    if(def.tx_pin == hwGPIO_Pin_E7 || def.rx_pin == hwGPIO_Pin_E6)
    {
        SYS->GPE_MFPL &=
            ~(SYS_GPE_MFPL_PE7MFP_Msk |
              SYS_GPE_MFPL_PE6MFP_Msk);
    }

    if(def.tx_pin == hwGPIO_Pin_D7 || def.rx_pin == hwGPIO_Pin_D6)
    {
        SYS->GPD_MFPL &=
            ~(SYS_GPD_MFPL_PD7MFP_Msk |
              SYS_GPD_MFPL_PD6MFP_Msk);
    }
#endif

    SYS_LockReg();
}

hwCAN_OpResult CAN_Instance_Init(hwCAN_Index index, uint32_t baudrate)
{
    CAN_T *can = CAN_Map_Soc_Base(index);

    if(can == NULL)
        return hwCAN_InvalidParameter;

    switch(index)
    {
#if defined(CAN0_BASE)
        case hwCAN_Index_0:
            CLK_EnableModuleClock(CAN0_MODULE);
            SYS_ResetModule(CAN0_RST);
            break;
#endif

#if defined(CAN1_BASE)
        case hwCAN_Index_1:
            CLK_EnableModuleClock(CAN1_MODULE);
            SYS_ResetModule(CAN1_RST);
            break;
#endif

        default:
            return hwCAN_InvalidParameter;
    }

    if(CAN_Open(can, baudrate, CAN_NORMAL_MODE) != baudrate)
        return hwCAN_HwError;

    return hwCAN_OK;
}

hwCAN_OpResult CAN_Instance_DeInit(hwCAN_Index index)
{
    CAN_T *can = CAN_Map_Soc_Base(index);

    if(can == NULL)
        return hwCAN_InvalidParameter;

    CAN_Close(can);

    switch(index)
    {
#if defined(CAN0_BASE)
        case hwCAN_Index_0:
            CLK_DisableModuleClock(CAN0_MODULE);
            break;
#endif

#if defined(CAN1_BASE)
        case hwCAN_Index_1:
            CLK_DisableModuleClock(CAN1_MODULE);
            break;
#endif

        default:
            return hwCAN_InvalidParameter;
    }

    return hwCAN_OK;
}

hwCAN_OpResult CAN_ConfigFilter(hwCAN_Index index)
{
    CAN_T *can = CAN_Map_Soc_Base(index);

    if(can == NULL)
        return hwCAN_InvalidParameter;

    /*
     * Message Object 0: RX
     * 接收所有 Standard ID
     */
    STR_CANMASK_T mask = {0};

    mask.u8Xtd    = 1;
    mask.u8Dir    = 1;
    mask.u8IdType = 0;
    mask.u32Id    = 0x00000000;

    CAN_SetRxMsgObj(can,
                    0,
                    CAN_STD_ID,
                    0x000,
                    TRUE);

    CAN_SetRxMsgObjMask(can,
                        0,
                        CAN_STD_ID,
                        0x000,
                        FALSE);

    return hwCAN_OK;
}

hwCAN_OpResult CAN_StartHardware(hwCAN_Index index)
{
    CAN_T *can = CAN_Map_Soc_Base(index);

    if(can == NULL)
        return hwCAN_InvalidParameter;

    CAN_EnableInt(can,
                  CAN_CON_IE_Msk |
                  CAN_CON_SIE_Msk |
                  CAN_CON_EIE_Msk);

    return hwCAN_OK;
}

hwCAN_OpResult CAN_StopHardware(hwCAN_Index index)
{
    CAN_T *can = CAN_Map_Soc_Base(index);

    if(can == NULL)
        return hwCAN_InvalidParameter;

    CAN_DisableInt(can,
                   CAN_CON_IE_Msk |
                   CAN_CON_SIE_Msk |
                   CAN_CON_EIE_Msk);

    return hwCAN_OK;
}

void CAN_NVIC_Init(hwCAN_Index index)
{
    switch(index)
    {
#if defined(CAN0_BASE)
        case hwCAN_Index_0:
            NVIC_ClearPendingIRQ(CAN0_IRQn);
            NVIC_EnableIRQ(CAN0_IRQn);
            break;
#endif

#if defined(CAN1_BASE)
        case hwCAN_Index_1:
            NVIC_ClearPendingIRQ(CAN1_IRQn);
            NVIC_EnableIRQ(CAN1_IRQn);
            break;
#endif

        default:
            break;
    }
}

void CAN_NVIC_DeInit(hwCAN_Index index)
{
    switch(index)
    {
#if defined(CAN0_BASE)
        case hwCAN_Index_0:
            HAL_NVIC_DisableIRQ(CAN0_IRQn);
            NVIC_ClearPendingIRQ(CAN0_IRQn);
            break;
#endif

#if defined(CAN1_BASE)
        case hwCAN_Index_1:
            HAL_NVIC_DisableIRQ(CAN1_IRQn);
            NVIC_ClearPendingIRQ(CAN1_IRQn);
            break;
#endif

        default:
            break;
    }
}

#endif