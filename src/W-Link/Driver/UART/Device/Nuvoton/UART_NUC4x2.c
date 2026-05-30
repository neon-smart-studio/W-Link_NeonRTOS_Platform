#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"
#include "UART/UART.h"

#include "NeonRTOS.h"

#if defined(NUC442) || defined(NUC472)

#include "GPIO/Device/Nuvoton/GPIO_Nuvoton.h"

#include "UART/Pin/Nuvoton/UART_Pin_Nuvoton.h"

#include "UART_Nuvoton.h"

static volatile uint8_t *UART_TxBuf[hwUART_Index_MAX] = {0};
static volatile uint8_t *UART_RxBuf[hwUART_Index_MAX] = {0};

static volatile size_t UART_TxLen[hwUART_Index_MAX] = {0};
static volatile size_t UART_RxLen[hwUART_Index_MAX] = {0};

static volatile size_t UART_TxCnt[hwUART_Index_MAX] = {0};
static volatile size_t UART_RxCnt[hwUART_Index_MAX] = {0};

static volatile bool UART_TxBusy[hwUART_Index_MAX] = {false};
static volatile bool UART_RxBusy[hwUART_Index_MAX] = {false};

UART_T *UART_Map_Soc_Base(hwUART_Index index)
{
    switch (index)
    {
#if defined(UART0_BASE)
        case hwUART_Index_0: return UART0;
#endif
#if defined(UART1_BASE)
        case hwUART_Index_1: return UART1;
#endif
#if defined(UART2_BASE)
        case hwUART_Index_2: return UART2;
#endif
#if defined(UART3_BASE)
        case hwUART_Index_3: return UART3;
#endif
#if defined(UART4_BASE)
        case hwUART_Index_4: return UART4;
#endif
#if defined(UART5_BASE)
        case hwUART_Index_5: return UART5;
#endif
        default: return NULL;
    }
}

void UART_GPIO_ConfigAF(hwUART_Index index, bool rts_cts)
{
    UART_Pin_Def def = UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]];

    SYS_UnlockReg();

    switch(index)
    {
#if defined(UART0_BASE)
        case hwUART_Index_0:
            if(def.tx_pin == hwGPIO_Pin_B12 && def.rx_pin == hwGPIO_Pin_B13)
            {
                SYS->GPB_MFPH =
                    (SYS->GPB_MFPH &
                    ~(SYS_GPB_MFPH_PB12MFP_Msk |
                      SYS_GPB_MFPH_PB13MFP_Msk)) |
                    (0x1UL << SYS_GPB_MFPH_PB12MFP_Pos) |
                    (0x1UL << SYS_GPB_MFPH_PB13MFP_Pos);
            }
            else if(def.tx_pin == hwGPIO_Pin_A14 && def.rx_pin == hwGPIO_Pin_A15)
            {
                SYS->GPA_MFPH =
                    (SYS->GPA_MFPH &
                    ~(SYS_GPA_MFPH_PA14MFP_Msk |
                      SYS_GPA_MFPH_PA15MFP_Msk)) |
                    (0x3UL << SYS_GPA_MFPH_PA14MFP_Pos) |
                    (0x3UL << SYS_GPA_MFPH_PA15MFP_Pos);
            }
            break;
#endif

#if defined(UART1_BASE)
        case hwUART_Index_1:
            if(def.tx_pin == hwGPIO_Pin_B2 && def.rx_pin == hwGPIO_Pin_B3)
            {
                SYS->GPB_MFPL =
                    (SYS->GPB_MFPL &
                    ~(SYS_GPB_MFPL_PB2MFP_Msk |
                      SYS_GPB_MFPL_PB3MFP_Msk)) |
                    (0x1UL << SYS_GPB_MFPL_PB2MFP_Pos) |
                    (0x1UL << SYS_GPB_MFPL_PB3MFP_Pos);
            }
            else if(def.tx_pin == hwGPIO_Pin_A8 && def.rx_pin == hwGPIO_Pin_A9)
            {
                SYS->GPA_MFPH =
                    (SYS->GPA_MFPH &
                    ~(SYS_GPA_MFPH_PA8MFP_Msk |
                      SYS_GPA_MFPH_PA9MFP_Msk)) |
                    (0x3UL << SYS_GPA_MFPH_PA8MFP_Pos) |
                    (0x3UL << SYS_GPA_MFPH_PA9MFP_Pos);
            }
            break;
#endif

#if defined(UART2_BASE)
        case hwUART_Index_2:
            if(def.tx_pin == hwGPIO_Pin_B6 && def.rx_pin == hwGPIO_Pin_B7)
            {
                SYS->GPB_MFPL =
                    (SYS->GPB_MFPL &
                    ~(SYS_GPB_MFPL_PB6MFP_Msk |
                      SYS_GPB_MFPL_PB7MFP_Msk)) |
                    (0x1UL << SYS_GPB_MFPL_PB6MFP_Pos) |
                    (0x1UL << SYS_GPB_MFPL_PB7MFP_Pos);
            }
            else if(def.tx_pin == hwGPIO_Pin_D14 && def.rx_pin == hwGPIO_Pin_D15)
            {
                SYS->GPD_MFPH =
                    (SYS->GPD_MFPH &
                    ~(SYS_GPD_MFPH_PD14MFP_Msk |
                      SYS_GPD_MFPH_PD15MFP_Msk)) |
                    (0x3UL << SYS_GPD_MFPH_PD14MFP_Pos) |
                    (0x3UL << SYS_GPD_MFPH_PD15MFP_Pos);
            }
            break;
#endif

#if defined(UART3_BASE)
        case hwUART_Index_3:
            if(def.tx_pin == hwGPIO_Pin_B10 && def.rx_pin == hwGPIO_Pin_B11)
            {
                SYS->GPB_MFPH =
                    (SYS->GPB_MFPH &
                    ~(SYS_GPB_MFPH_PB10MFP_Msk |
                      SYS_GPB_MFPH_PB11MFP_Msk)) |
                    (0x1UL << SYS_GPB_MFPH_PB10MFP_Pos) |
                    (0x1UL << SYS_GPB_MFPH_PB11MFP_Pos);
            }
            else if(def.tx_pin == hwGPIO_Pin_C10 && def.rx_pin == hwGPIO_Pin_C11)
            {
                SYS->GPC_MFPH =
                    (SYS->GPC_MFPH &
                    ~(SYS_GPC_MFPH_PC10MFP_Msk |
                      SYS_GPC_MFPH_PC11MFP_Msk)) |
                    (0x3UL << SYS_GPC_MFPH_PC10MFP_Pos) |
                    (0x3UL << SYS_GPC_MFPH_PC11MFP_Pos);
            }
            break;
#endif

#if defined(UART4_BASE)
        case hwUART_Index_4:
            if(def.tx_pin == hwGPIO_Pin_C12 && def.rx_pin == hwGPIO_Pin_C13)
            {
                SYS->GPC_MFPH =
                    (SYS->GPC_MFPH &
                    ~(SYS_GPC_MFPH_PC12MFP_Msk |
                      SYS_GPC_MFPH_PC13MFP_Msk)) |
                    (0x1UL << SYS_GPC_MFPH_PC12MFP_Pos) |
                    (0x1UL << SYS_GPC_MFPH_PC13MFP_Pos);
            }
            else if(def.tx_pin == hwGPIO_Pin_E8 && def.rx_pin == hwGPIO_Pin_E9)
            {
                SYS->GPE_MFPH =
                    (SYS->GPE_MFPH &
                    ~(SYS_GPE_MFPH_PE8MFP_Msk |
                      SYS_GPE_MFPH_PE9MFP_Msk)) |
                    (0x3UL << SYS_GPE_MFPH_PE8MFP_Pos) |
                    (0x3UL << SYS_GPE_MFPH_PE9MFP_Pos);
            }
            break;
#endif

#if defined(UART5_BASE)
        case hwUART_Index_5:
            if(def.tx_pin == hwGPIO_Pin_E0 && def.rx_pin == hwGPIO_Pin_E1)
            {
                SYS->GPE_MFPL =
                    (SYS->GPE_MFPL &
                    ~(SYS_GPE_MFPL_PE0MFP_Msk |
                      SYS_GPE_MFPL_PE1MFP_Msk)) |
                    (0x1UL << SYS_GPE_MFPL_PE0MFP_Pos) |
                    (0x1UL << SYS_GPE_MFPL_PE1MFP_Pos);
            }
            else if(def.tx_pin == hwGPIO_Pin_C14 && def.rx_pin == hwGPIO_Pin_C15)
            {
                SYS->GPC_MFPH =
                    (SYS->GPC_MFPH &
                    ~(SYS_GPC_MFPH_PC14MFP_Msk |
                      SYS_GPC_MFPH_PC15MFP_Msk)) |
                    (0x3UL << SYS_GPC_MFPH_PC14MFP_Pos) |
                    (0x3UL << SYS_GPC_MFPH_PC15MFP_Pos);
            }
            break;
#endif
    }

    SYS_LockReg();
}

void UART_GPIO_DeConfigAF(hwUART_Index index, bool rts_cts)
{
    UART_Pin_Def def = UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]];

    GPIO_T *tx_port = GPIO_Map_Soc_Base(def.tx_pin);
    GPIO_T *rx_port = GPIO_Map_Soc_Base(def.rx_pin);

    uint16_t tx_mask = GPIO_Map_Soc_Pin(def.tx_pin);
    uint16_t rx_mask = GPIO_Map_Soc_Pin(def.rx_pin);

    SYS_UnlockReg();

    if(tx_port && tx_mask) GPIO_SetMode(tx_port, tx_mask, GPIO_MODE_INPUT);
    if(rx_port && rx_mask) GPIO_SetMode(rx_port, rx_mask, GPIO_MODE_INPUT);

    switch(index)
    {
#if defined(UART0_BASE)
        case hwUART_Index_0:
            if(def.tx_pin == hwGPIO_Pin_B12 && def.rx_pin == hwGPIO_Pin_B13)
            {
                SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB12MFP_Msk |
                                   SYS_GPB_MFPH_PB13MFP_Msk);
            }
            else if(def.tx_pin == hwGPIO_Pin_A14 && def.rx_pin == hwGPIO_Pin_A15)
            {
                SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA14MFP_Msk |
                                   SYS_GPA_MFPH_PA15MFP_Msk);
            }
            break;
#endif

#if defined(UART1_BASE)
        case hwUART_Index_1:
            if(def.tx_pin == hwGPIO_Pin_B2 && def.rx_pin == hwGPIO_Pin_B3)
            {
                SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB2MFP_Msk |
                                   SYS_GPB_MFPL_PB3MFP_Msk);
            }
            else if(def.tx_pin == hwGPIO_Pin_A8 && def.rx_pin == hwGPIO_Pin_A9)
            {
                SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA8MFP_Msk |
                                   SYS_GPA_MFPH_PA9MFP_Msk);
            }
            break;
#endif

#if defined(UART2_BASE)
        case hwUART_Index_2:
            if(def.tx_pin == hwGPIO_Pin_B6 && def.rx_pin == hwGPIO_Pin_B7)
            {
                SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB6MFP_Msk |
                                   SYS_GPB_MFPL_PB7MFP_Msk);
            }
            else if(def.tx_pin == hwGPIO_Pin_D14 && def.rx_pin == hwGPIO_Pin_D15)
            {
                SYS->GPD_MFPH &= ~(SYS_GPD_MFPH_PD14MFP_Msk |
                                   SYS_GPD_MFPH_PD15MFP_Msk);
            }
            break;
#endif

#if defined(UART3_BASE)
        case hwUART_Index_3:
            if(def.tx_pin == hwGPIO_Pin_B10 && def.rx_pin == hwGPIO_Pin_B11)
            {
                SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB10MFP_Msk |
                                   SYS_GPB_MFPH_PB11MFP_Msk);
            }
            else if(def.tx_pin == hwGPIO_Pin_C10 && def.rx_pin == hwGPIO_Pin_C11)
            {
                SYS->GPC_MFPH &= ~(SYS_GPC_MFPH_PC10MFP_Msk |
                                   SYS_GPC_MFPH_PC11MFP_Msk);
            }
            break;
#endif

#if defined(UART4_BASE)
        case hwUART_Index_4:
            if(def.tx_pin == hwGPIO_Pin_C12 && def.rx_pin == hwGPIO_Pin_C13)
            {
                SYS->GPC_MFPH &= ~(SYS_GPC_MFPH_PC12MFP_Msk |
                                   SYS_GPC_MFPH_PC13MFP_Msk);
            }
            else if(def.tx_pin == hwGPIO_Pin_E8 && def.rx_pin == hwGPIO_Pin_E9)
            {
                SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE8MFP_Msk |
                                   SYS_GPE_MFPH_PE9MFP_Msk);
            }
            break;
#endif

#if defined(UART5_BASE)
        case hwUART_Index_5:
            if(def.tx_pin == hwGPIO_Pin_E0 && def.rx_pin == hwGPIO_Pin_E1)
            {
                SYS->GPE_MFPL &= ~(SYS_GPE_MFPL_PE0MFP_Msk |
                                   SYS_GPE_MFPL_PE1MFP_Msk);
            }
            else if(def.tx_pin == hwGPIO_Pin_C14 && def.rx_pin == hwGPIO_Pin_C15)
            {
                SYS->GPC_MFPH &= ~(SYS_GPC_MFPH_PC14MFP_Msk |
                                   SYS_GPC_MFPH_PC15MFP_Msk);
            }
            break;
#endif

        default:
            break;
    }

    SYS_LockReg();
}

static void UART_EnableClock(hwUART_Index index)
{
    switch (index)
    {
#if defined(UART0_BASE)
        case hwUART_Index_0:
            CLK_EnableModuleClock(UART0_MODULE);
            break;
#endif
#if defined(UART1_BASE)
        case hwUART_Index_1:
            CLK_EnableModuleClock(UART1_MODULE);
            break;
#endif
#if defined(UART2_BASE)
        case hwUART_Index_2:
            CLK_EnableModuleClock(UART2_MODULE);
            break;
#endif
#if defined(UART3_BASE)
        case hwUART_Index_3:
            CLK_EnableModuleClock(UART3_MODULE);
            break;
#endif
#if defined(UART4_BASE)
        case hwUART_Index_4:
            CLK_EnableModuleClock(UART4_MODULE);
            break;
#endif
#if defined(UART5_BASE)
        case hwUART_Index_5:
            CLK_EnableModuleClock(UART5_MODULE);
            break;
#endif
        default:
            break;
    }
}

static void UART_DisableClock(hwUART_Index index)
{
    switch (index)
    {
#if defined(UART0_BASE)
        case hwUART_Index_0:
            CLK_DisableModuleClock(UART0_MODULE);
            break;
#endif
#if defined(UART1_BASE)
        case hwUART_Index_1:
            CLK_DisableModuleClock(UART1_MODULE);
            break;
#endif
#if defined(UART2_BASE)
        case hwUART_Index_2:
            CLK_DisableModuleClock(UART2_MODULE);
            break;
#endif
#if defined(UART3_BASE)
        case hwUART_Index_3:
            CLK_DisableModuleClock(UART3_MODULE);
            break;
#endif
#if defined(UART4_BASE)
        case hwUART_Index_4:
            CLK_DisableModuleClock(UART4_MODULE);
            break;
#endif
#if defined(UART5_BASE)
        case hwUART_Index_5:
            CLK_DisableModuleClock(UART5_MODULE);
            break;
#endif
        default:
            break;
    }
}

hwUART_OpResult UART_Instance_Init(
    hwUART_Index index,
    uint32_t baudrate,
    bool rts_cts,
    uint8_t data_bits,
    UART_Parity parity,
    uint8_t stop_bits
)
{
    UART_T *uart = UART_Map_Soc_Base(index);

    if (uart == NULL)
        return hwUART_InvalidParameter;

    if (parity >= UART_Parity_MAX) {
        return hwUART_InvalidParameter;
    }

    if (stop_bits != 1 && stop_bits != 2) {
        return hwUART_InvalidParameter;
    }

    if (data_bits != 5 && data_bits != 6 && data_bits != 7 && data_bits != 8)
    {
        return hwUART_InvalidParameter;
    }

    uint32_t word_len;
    switch (data_bits)
    {
#ifdef UART_WORD_LEN_5
        case 5: word_len = UART_WORD_LEN_5; break;
#endif
#ifdef UART_WORD_LEN_6
        case 6: word_len = UART_WORD_LEN_6; break;
#endif
#ifdef UART_WORD_LEN_7
        case 7: word_len = UART_WORD_LEN_7; break;
#endif
        case 8: word_len = UART_WORD_LEN_8; break;
    }
    
    uint32_t parity_cfg;
    switch (parity)
    {
        case UART_Parity_None: parity_cfg = UART_PARITY_NONE; break;
        case UART_Parity_Even: parity_cfg = UART_PARITY_EVEN; break;
        case UART_Parity_Odd:  parity_cfg = UART_PARITY_ODD; break;
    }

    uint32_t stop_cfg;
    switch (stop_bits)
    {
        case 1: stop_cfg = UART_STOP_BIT_1; break;
        case 2: stop_cfg = UART_STOP_BIT_2; break;
    }

    UART_EnableClock(index);

    UART_Open_Port(uart, baudrate);

    UART_SetLine_Config(
        uart,
        baudrate,
        word_len,
        parity_cfg,
        stop_cfg
    );

    if (rts_cts)
    {
        uart->MODEM |= UART_MODEM_RTSACTLV_Msk;
        uart->MODEMSTS |= UART_MODEMSTS_CTSACTLV_Msk;
    }

    return hwUART_OK;
}

hwUART_OpResult UART_Instance_DeInit(hwUART_Index index)
{
    UART_T *uart = UART_Map_Soc_Base(index);

    if (uart == NULL)
        return hwUART_InvalidParameter;

    UART_Close_Port(uart);
    UART_DisableClock(index);

    return hwUART_OK;
}

hwUART_OpResult UART_Instance_Read_IT(hwUART_Index index, uint8_t *data_rd, size_t size)
{
    if (index >= hwUART_Index_MAX)
        return hwUART_InvalidParameter;

    if (!data_rd || size == 0)
        return hwUART_InvalidParameter;

    if (!UART_Init_Status[index])
        return hwUART_NotInit;

    if (UART_RxBusy[index])
        return hwUART_Busy;

    UART_T *uart = UART_Map_Soc_Base(index);

    if (uart == NULL)
        return hwUART_InvalidParameter;

    UART_RxBuf[index] = data_rd;
    UART_RxLen[index] = size;
    UART_RxCnt[index] = 0;
    UART_RxBusy[index] = true;

    UART_EnableInt(uart, UART_INTEN_RDAIEN_Msk);

    return hwUART_OK;
}

hwUART_OpResult UART_Instance_Write_IT(hwUART_Index index, uint8_t *data_wr, size_t size)
{
    if (index >= hwUART_Index_MAX)
        return hwUART_InvalidParameter;

    if (!data_wr || size == 0)
        return hwUART_InvalidParameter;

    if (!UART_Init_Status[index])
        return hwUART_NotInit;

    if (UART_TxBusy[index])
        return hwUART_Busy;

    UART_T *uart = UART_Map_Soc_Base(index);

    if (uart == NULL)
        return hwUART_InvalidParameter;

    UART_TxBuf[index] = data_wr;
    UART_TxLen[index] = size;
    UART_TxCnt[index] = 0;
    UART_TxBusy[index] = true;

    UART_EnableInt(uart, UART_INTEN_THREIEN_Msk);

    return hwUART_OK;
}

hwUART_OpResult UART_Instance_Stop_Read(hwUART_Index index)
{
    if (index >= hwUART_Index_MAX)
        return hwUART_InvalidParameter;

    if (!UART_Init_Status[index])
        return hwUART_NotInit;

    if (UART_RxBusy[index])
        return hwUART_Busy;

    UART_T *uart = UART_Map_Soc_Base(index);

    if (uart == NULL)
        return hwUART_InvalidParameter;

    UART_DisableInt(uart, UART_INTEN_RDAIEN_Msk);

    size_t recv_bytes = UART_RxCnt[index];

    UART_RxBusy[index] = false;
    UART_RxBuf[index] = NULL;
    UART_RxLen[index] = 0;
    UART_RxCnt[index] = 0;

    return hwUART_OK;
}

hwUART_OpResult UART_Instance_Stop_Write(hwUART_Index index)
{
    if (index >= hwUART_Index_MAX)
        return hwUART_InvalidParameter;

    if (!UART_Init_Status[index])
        return hwUART_NotInit;

    if (UART_TxBusy[index])
        return hwUART_Busy;

    UART_T *uart = UART_Map_Soc_Base(index);

    if (uart == NULL)
        return hwUART_InvalidParameter;

    UART_DisableInt(uart, UART_INTEN_THREIEN_Msk);

    size_t send_bytes = UART_TxCnt[index];

    UART_TxBusy[index] = false;
    UART_TxBuf[index] = NULL;
    UART_TxLen[index] = 0;
    UART_TxCnt[index] = 0;

    return hwUART_OK;
}

void UART_NVIC_Init(hwUART_Index index)
{
    switch (index)
    {
#if defined(UART0_BASE)
        case hwUART_Index_0:
            NVIC_SetPriority(UART0_IRQn, UART_IRQ_NVIC_PRIORITY);
            NVIC_EnableIRQ(UART0_IRQn);
            break;
#endif
#if defined(UART1_BASE)
        case hwUART_Index_1:
            NVIC_SetPriority(UART1_IRQn, UART_IRQ_NVIC_PRIORITY);
            NVIC_EnableIRQ(UART1_IRQn);
            break;
#endif
#if defined(UART2_BASE)
        case hwUART_Index_2:
            NVIC_SetPriority(UART2_IRQn, UART_IRQ_NVIC_PRIORITY);
            NVIC_EnableIRQ(UART2_IRQn);
            break;
#endif
#if defined(UART3_BASE)
        case hwUART_Index_3:
            NVIC_SetPriority(UART3_IRQn, UART_IRQ_NVIC_PRIORITY);
            NVIC_EnableIRQ(UART3_IRQn);
            break;
#endif
#if defined(UART4_BASE)
        case hwUART_Index_4:
            NVIC_SetPriority(UART4_IRQn, UART_IRQ_NVIC_PRIORITY);
            NVIC_EnableIRQ(UART4_IRQn);
            break;
#endif
#if defined(UART5_BASE)
        case hwUART_Index_5:
            NVIC_SetPriority(UART5_IRQn, UART_IRQ_NVIC_PRIORITY);
            NVIC_EnableIRQ(UART5_IRQn);
            break;
#endif
        default:
            break;
    }
}

void UART_NVIC_DeInit(hwUART_Index index)
{
    switch (index)
    {
#if defined(UART0_BASE)
        case hwUART_Index_0:
            NVIC_DisableIRQ(UART0_IRQn);
            break;
#endif
#if defined(UART1_BASE)
        case hwUART_Index_1:
            NVIC_DisableIRQ(UART1_IRQn);
            break;
#endif
#if defined(UART2_BASE)
        case hwUART_Index_2:
            NVIC_DisableIRQ(UART2_IRQn);
            break;
#endif
#if defined(UART3_BASE)
        case hwUART_Index_3:
            NVIC_DisableIRQ(UART3_IRQn);
            break;
#endif
#if defined(UART4_BASE)
        case hwUART_Index_4:
            NVIC_DisableIRQ(UART4_IRQn);
            break;
#endif
#if defined(UART5_BASE)
        case hwUART_Index_5:
            NVIC_DisableIRQ(UART5_IRQn);
            break;
#endif
        default:
            break;
    }
}

static void UART_IRQHandler(hwUART_Index index)
{
    if (index >= hwUART_Index_MAX)
        return;

    UART_T *uart = UART_Map_Soc_Base(index);

    if (uart == NULL)
        return;

    uint32_t intsts = uart->INTSTS;

    if (intsts & UART_INTSTS_RDAIF_Msk)
    {
        while (!UART_GET_RX_EMPTY(uart))
        {
            uint8_t ch = UART_READ(uart);

            if (UART_RxBusy[index] && UART_RxBuf[index] && UART_RxCnt[index] < UART_RxLen[index])
            {
                UART_RxBuf[index][UART_RxCnt[index]++] = ch;

                if (UART_RxCnt[index] >= UART_RxLen[index])
                {
                    UART_DisableInt(uart, UART_INTEN_RDAIEN_Msk);
                    UART_RxBusy[index] = false;
                    UART_RxCpltCallback(index);
                    break;
                }
            }
        }
    }

    if (intsts & UART_INTSTS_THREIF_Msk)
    {
        if (UART_TxBusy[index] && UART_TxBuf[index])
        {
            while (!UART_IS_TX_FULL(uart) && UART_TxCnt[index] < UART_TxLen[index])
            {
                UART_WRITE(uart, UART_TxBuf[index][UART_TxCnt[index]++]);
            }

            if (UART_TxCnt[index] >= UART_TxLen[index])
            {
                UART_DisableInt(uart, UART_INTEN_THREIEN_Msk);
                UART_TxBusy[index] = false;
                UART_TxCpltCallback(index);
            }
        }
        else
        {
            UART_DisableInt(uart, UART_INTEN_THREIEN_Msk);
        }
    }

    if (intsts & UART_INTSTS_BUFERRIF_Msk)
    {
        UART_ClearIntFlag(uart, UART_INTSTS_BUFERRIF_Msk);
    }
}


#if defined(UART0_BASE)
void UART0_IRQHandler(void)
{
    UART_IRQHandler(hwUART_Index_0);
}
#endif

#if defined(UART1_BASE)
void UART1_IRQHandler(void)
{
    UART_IRQHandler(hwUART_Index_1);
}
#endif

#if defined(UART2_BASE)
void UART2_IRQHandler(void)
{
    UART_IRQHandler(hwUART_Index_2);
}
#endif

#if defined(UART3_BASE)
void UART3_IRQHandler(void)
{
    UART_IRQHandler(hwUART_Index_3);
}
#endif

#if defined(UART4_BASE)
void UART4_IRQHandler(void)
{
    UART_IRQHandler(hwUART_Index_4);
}
#endif

#if defined(UART5_BASE)
void UART5_IRQHandler(void)
{
    UART_IRQHandler(hwUART_Index_5);
}
#endif

#endif // NUC442 || NUC472