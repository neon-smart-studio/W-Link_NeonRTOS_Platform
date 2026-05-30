#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#if defined(NUC442) || defined(NUC472)

#include "GPIO/Device/Nuvoton/GPIO_Nuvoton.h"

#include "I2C/Pin/Nuvoton/I2C_Pin_Nuvoton.h"

#include "I2C_Master_Nuvoton.h"

typedef enum {
    NUC4x2_I2C_IDLE = 0,
    NUC4x2_I2C_START,
    NUC4x2_I2C_ADDR_W,
    NUC4x2_I2C_ADDR_R,
    NUC4x2_I2C_TX,
    NUC4x2_I2C_RX,
    NUC4x2_I2C_DONE,
    NUC4x2_I2C_ERROR
} NUC4x2_I2C_State;

typedef struct {
    NUC4x2_I2C_State state;

    uint8_t addr;

    uint8_t *tx_buf;
    uint8_t tx_len;
    uint8_t tx_pos;

    uint8_t *rx_buf;
    uint8_t rx_len;
    uint8_t rx_pos;

    bool stop;
    hwI2C_OpResult result;
} NUC4x2_I2C_Transfer;

static NUC4x2_I2C_Transfer i2c_xfer[hwI2C_Index_MAX];

I2C_T *I2C_Map_Soc_Base(hwI2C_Index index)
{
    switch(index)
    {
#if defined(I2C0_BASE)
        case hwI2C_Index_0: return I2C0;
#endif
#if defined(I2C1_BASE)
        case hwI2C_Index_1: return I2C1;
#endif
#if defined(I2C2_BASE)
        case hwI2C_Index_2: return I2C2;
#endif
#if defined(I2C3_BASE)
        case hwI2C_Index_3: return I2C3;
#endif
#if defined(I2C4_BASE)
        case hwI2C_Index_4: return I2C4;
#endif
        default: return NULL;
    }
}

void I2C_GPIO_ConfigAF(hwI2C_Index index)
{
    I2C_Pin_Def def = I2C_Pin_Def_Table[index][I2C_Index_Map_Alt[index]];

    SYS_UnlockReg();

    switch(index)
    {
#if defined(I2C0_BASE)
        case hwI2C_Index_0:
            if(def.scl_pin == hwGPIO_Pin_A10 && def.sda_pin == hwGPIO_Pin_A11)
            {
                SYS->GPA_MFPH =
                    (SYS->GPA_MFPH &
                    ~(SYS_GPA_MFPH_PA10MFP_Msk |
                      SYS_GPA_MFPH_PA11MFP_Msk)) |
                    (0x2UL << SYS_GPA_MFPH_PA10MFP_Pos) |
                    (0x2UL << SYS_GPA_MFPH_PA11MFP_Pos);
            }
            else if(def.scl_pin == hwGPIO_Pin_B4 && def.sda_pin == hwGPIO_Pin_B5)
            {
                SYS->GPB_MFPL =
                    (SYS->GPB_MFPL &
                    ~(SYS_GPB_MFPL_PB4MFP_Msk |
                      SYS_GPB_MFPL_PB5MFP_Msk)) |
                    (0x2UL << SYS_GPB_MFPL_PB4MFP_Pos) |
                    (0x2UL << SYS_GPB_MFPL_PB5MFP_Pos);
            }
            else if(def.scl_pin == hwGPIO_Pin_D14 && def.sda_pin == hwGPIO_Pin_D15)
            {
                SYS->GPD_MFPH =
                    (SYS->GPD_MFPH &
                    ~(SYS_GPD_MFPH_PD14MFP_Msk |
                      SYS_GPD_MFPH_PD15MFP_Msk)) |
                    (0x2UL << SYS_GPD_MFPH_PD14MFP_Pos) |
                    (0x2UL << SYS_GPD_MFPH_PD15MFP_Pos);
            }
            break;
#endif
#if defined(I2C1_BASE)
        case hwI2C_Index_1:
            if(def.scl_pin == hwGPIO_Pin_A12 && def.sda_pin == hwGPIO_Pin_A13)
            {
                SYS->GPA_MFPH =
                    (SYS->GPA_MFPH &
                    ~(SYS_GPA_MFPH_PA12MFP_Msk |
                      SYS_GPA_MFPH_PA13MFP_Msk)) |
                    (0x2UL << SYS_GPA_MFPH_PA12MFP_Pos) |
                    (0x2UL << SYS_GPA_MFPH_PA13MFP_Pos);
            }
            else if(def.scl_pin == hwGPIO_Pin_B6 && def.sda_pin == hwGPIO_Pin_B7)
            {
                SYS->GPB_MFPL =
                    (SYS->GPB_MFPL &
                    ~(SYS_GPB_MFPL_PB6MFP_Msk |
                      SYS_GPB_MFPL_PB7MFP_Msk)) |
                    (0x2UL << SYS_GPB_MFPL_PB6MFP_Pos) |
                    (0x2UL << SYS_GPB_MFPL_PB7MFP_Pos);
            }
            else if(def.scl_pin == hwGPIO_Pin_E0 && def.sda_pin == hwGPIO_Pin_E1)
            {
                SYS->GPE_MFPL =
                    (SYS->GPE_MFPL &
                    ~(SYS_GPE_MFPL_PE0MFP_Msk |
                      SYS_GPE_MFPL_PE1MFP_Msk)) |
                    (0x2UL << SYS_GPE_MFPL_PE0MFP_Pos) |
                    (0x2UL << SYS_GPE_MFPL_PE1MFP_Pos);
            }
            break;
#endif
#if defined(I2C2_BASE)
        case hwI2C_Index_2:
            if(def.scl_pin == hwGPIO_Pin_A14 && def.sda_pin == hwGPIO_Pin_A15)
            {
                SYS->GPA_MFPH =
                    (SYS->GPA_MFPH &
                    ~(SYS_GPA_MFPH_PA14MFP_Msk |
                      SYS_GPA_MFPH_PA15MFP_Msk)) |
                    (0x2UL << SYS_GPA_MFPH_PA14MFP_Pos) |
                    (0x2UL << SYS_GPA_MFPH_PA15MFP_Pos);
            }
            else if(def.scl_pin == hwGPIO_Pin_B8 && def.sda_pin == hwGPIO_Pin_B9)
            {
                SYS->GPB_MFPH =
                    (SYS->GPB_MFPH &
                    ~(SYS_GPB_MFPH_PB8MFP_Msk |
                      SYS_GPB_MFPH_PB9MFP_Msk)) |
                    (0x2UL << SYS_GPB_MFPH_PB8MFP_Pos) |
                    (0x2UL << SYS_GPB_MFPH_PB9MFP_Pos);
            }
            else if(def.scl_pin == hwGPIO_Pin_E2 && def.sda_pin == hwGPIO_Pin_E3)
            {
                SYS->GPE_MFPL =
                    (SYS->GPE_MFPL &
                    ~(SYS_GPE_MFPL_PE2MFP_Msk |
                      SYS_GPE_MFPL_PE3MFP_Msk)) |
                    (0x2UL << SYS_GPE_MFPL_PE2MFP_Pos) |
                    (0x2UL << SYS_GPE_MFPL_PE3MFP_Pos);
            }
            break;
#endif
#if defined(I2C3_BASE)
        case hwI2C_Index_3:
            if(def.scl_pin == hwGPIO_Pin_C0 && def.sda_pin == hwGPIO_Pin_C1)
            {
                SYS->GPC_MFPL =
                    (SYS->GPC_MFPL &
                    ~(SYS_GPC_MFPL_PC0MFP_Msk |
                      SYS_GPC_MFPL_PC1MFP_Msk)) |
                    (0x2UL << SYS_GPC_MFPL_PC0MFP_Pos) |
                    (0x2UL << SYS_GPC_MFPL_PC1MFP_Pos);
            }
            else if(def.scl_pin == hwGPIO_Pin_D0 && def.sda_pin == hwGPIO_Pin_D1)
            {
                SYS->GPD_MFPL =
                    (SYS->GPD_MFPL &
                    ~(SYS_GPD_MFPL_PD0MFP_Msk |
                      SYS_GPD_MFPL_PD1MFP_Msk)) |
                    (0x2UL << SYS_GPD_MFPL_PD0MFP_Pos) |
                    (0x2UL << SYS_GPD_MFPL_PD1MFP_Pos);
            }
            else if(def.scl_pin == hwGPIO_Pin_E4 && def.sda_pin == hwGPIO_Pin_E5)
            {
                SYS->GPE_MFPL =
                    (SYS->GPE_MFPL &
                    ~(SYS_GPE_MFPL_PE4MFP_Msk |
                      SYS_GPE_MFPL_PE5MFP_Msk)) |
                    (0x2UL << SYS_GPE_MFPL_PE4MFP_Pos) |
                    (0x2UL << SYS_GPE_MFPL_PE5MFP_Pos);
            }
            break;
#endif
#if defined(I2C4_BASE)
        case hwI2C_Index_4:
            if(def.scl_pin == hwGPIO_Pin_C2 && def.sda_pin == hwGPIO_Pin_C3)
            {
                SYS->GPC_MFPL =
                    (SYS->GPC_MFPL &
                    ~(SYS_GPC_MFPL_PC2MFP_Msk |
                      SYS_GPC_MFPL_PC3MFP_Msk)) |
                    (0x2UL << SYS_GPC_MFPL_PC2MFP_Pos) |
                    (0x2UL << SYS_GPC_MFPL_PC3MFP_Pos);
            }
            else if(def.scl_pin == hwGPIO_Pin_D2 && def.sda_pin == hwGPIO_Pin_D3)
            {
                SYS->GPD_MFPL =
                    (SYS->GPD_MFPL &
                    ~(SYS_GPD_MFPL_PD2MFP_Msk |
                      SYS_GPD_MFPL_PD3MFP_Msk)) |
                    (0x2UL << SYS_GPD_MFPL_PD2MFP_Pos) |
                    (0x2UL << SYS_GPD_MFPL_PD3MFP_Pos);
            }
            else if(def.scl_pin == hwGPIO_Pin_E6 && def.sda_pin == hwGPIO_Pin_E7)
            {
                SYS->GPE_MFPL =
                    (SYS->GPE_MFPL &
                    ~(SYS_GPE_MFPL_PE6MFP_Msk |
                      SYS_GPE_MFPL_PE7MFP_Msk)) |
                    (0x2UL << SYS_GPE_MFPL_PE6MFP_Pos) |
                    (0x2UL << SYS_GPE_MFPL_PE7MFP_Pos);
            }
            break;
#endif

        default:
            break;
    }

    SYS_LockReg();
}

void I2C_GPIO_DeConfigAF(hwI2C_Index index)
{
    I2C_Pin_Def def = I2C_Pin_Def_Table[index][I2C_Index_Map_Alt[index]];

    SYS_UnlockReg();

    GPIO_T *scl_port = GPIO_Map_Soc_Base(def.scl_pin);
    GPIO_T *sda_port = GPIO_Map_Soc_Base(def.sda_pin);
    uint16_t scl_mask = GPIO_Map_Soc_Pin(def.scl_pin);
    uint16_t sda_mask = GPIO_Map_Soc_Pin(def.sda_pin);

    if(scl_port != NULL && scl_mask != 0)
        GPIO_SetMode(scl_port, scl_mask, GPIO_MODE_INPUT);

    if(sda_port != NULL && sda_mask != 0)
        GPIO_SetMode(sda_port, sda_mask, GPIO_MODE_INPUT);

    /* 簡單做法：依 pin 清 MFP */
    switch(index)
    {
#if defined(I2C0_BASE)
        case hwI2C_Index_0:
            if(def.scl_pin == hwGPIO_Pin_A10 && def.sda_pin == hwGPIO_Pin_A11)
            {
                SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA10MFP_Msk;
                SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA11MFP_Msk;
            }
            else if(def.scl_pin == hwGPIO_Pin_B4 && def.sda_pin == hwGPIO_Pin_B5)
            {
                SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB4MFP_Msk;
                SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB5MFP_Msk;
            }
            else if(def.scl_pin == hwGPIO_Pin_D14 && def.sda_pin == hwGPIO_Pin_D15)
            {
                SYS->GPD_MFPH &= ~SYS_GPD_MFPH_PD14MFP_Msk;
                SYS->GPD_MFPH &= ~SYS_GPD_MFPH_PD15MFP_Msk;
            }
            break;
#endif
#if defined(I2C1_BASE)
        case hwI2C_Index_1:
            if(def.scl_pin == hwGPIO_Pin_A12 && def.sda_pin == hwGPIO_Pin_A13)
            {
                SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA12MFP_Msk;
                SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA13MFP_Msk;
            }
            else if(def.scl_pin == hwGPIO_Pin_B6 && def.sda_pin == hwGPIO_Pin_B7)
            {
                SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB6MFP_Msk;
                SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB7MFP_Msk;
            }
            else if(def.scl_pin == hwGPIO_Pin_E0 && def.sda_pin == hwGPIO_Pin_E1)
            {
                SYS->GPE_MFPL &= ~SYS_GPE_MFPL_PE0MFP_Msk;
                SYS->GPE_MFPL &= ~SYS_GPE_MFPL_PE1MFP_Msk;
            }
            break;
#endif
#if defined(I2C2_BASE)
        case hwI2C_Index_2:
            if(def.scl_pin == hwGPIO_Pin_A14 && def.sda_pin == hwGPIO_Pin_A15)
            {
                SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA14MFP_Msk;
                SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA15MFP_Msk;
            }
            else if(def.scl_pin == hwGPIO_Pin_B8 && def.sda_pin == hwGPIO_Pin_B9)
            {
                SYS->GPB_MFPH &= ~SYS_GPB_MFPH_PB8MFP_Msk;
                SYS->GPB_MFPH &= ~SYS_GPB_MFPH_PB9MFP_Msk;
            }
            else if(def.scl_pin == hwGPIO_Pin_E2 && def.sda_pin == hwGPIO_Pin_E3)
            {
                SYS->GPE_MFPL &= ~SYS_GPE_MFPL_PE2MFP_Msk;
                SYS->GPE_MFPL &= ~SYS_GPE_MFPL_PE3MFP_Msk;
            }
            break;
#endif
#if defined(I2C3_BASE)
        case hwI2C_Index_3:
            if(def.scl_pin == hwGPIO_Pin_C0 && def.sda_pin == hwGPIO_Pin_C1)
            {
                SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC0MFP_Msk;
                SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC1MFP_Msk;
            }
            else if(def.scl_pin == hwGPIO_Pin_D0 && def.sda_pin == hwGPIO_Pin_D1)
            {
                SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD0MFP_Msk;
                SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD1MFP_Msk;
            }
            else if(def.scl_pin == hwGPIO_Pin_E4 && def.sda_pin == hwGPIO_Pin_E5)
            {
                SYS->GPE_MFPL &= ~SYS_GPE_MFPL_PE4MFP_Msk;
                SYS->GPE_MFPL &= ~SYS_GPE_MFPL_PE5MFP_Msk;
            }
            break;
#endif
#if defined(I2C4_BASE)
        case hwI2C_Index_4:
            if(def.scl_pin == hwGPIO_Pin_C2 && def.sda_pin == hwGPIO_Pin_C3)
            {
                SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC2MFP_Msk;
                SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC3MFP_Msk;
            }
            else if(def.scl_pin == hwGPIO_Pin_D2 && def.sda_pin == hwGPIO_Pin_D3)
            {
                SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD2MFP_Msk;
                SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD3MFP_Msk;
            }
            else if(def.scl_pin == hwGPIO_Pin_E6 && def.sda_pin == hwGPIO_Pin_E7)
            {
                SYS->GPE_MFPL &= ~SYS_GPE_MFPL_PE6MFP_Msk;
                SYS->GPE_MFPL &= ~SYS_GPE_MFPL_PE7MFP_Msk;
            }
            break;
#endif

        default:
            break;
    }

    SYS_LockReg();
}

static void I2C_IRQ_Process(hwI2C_Index index)
{
    I2C_T *i2c = I2C_Map_Soc_Base(index);
    NUC4x2_I2C_Transfer *t = &i2c_xfer[index];

    uint32_t status = I2C_GET_STATUS(i2c);

    switch(status)
    {
        case 0x08: /* START transmitted */
            if(t->state == NUC4x2_I2C_TX)
                I2C_SET_DATA(i2c, (t->addr << 1) | 0);
            else
                I2C_SET_DATA(i2c, (t->addr << 1) | 1);

            I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI_Msk);
            break;

        case 0x18: /* SLA+W ACK */
            if(t->tx_pos < t->tx_len)
            {
                I2C_SET_DATA(i2c, t->tx_buf[t->tx_pos++]);
                I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI_Msk);
            }
            else
            {
                t->state = NUC4x2_I2C_DONE;

                if(t->stop)
                    I2C_STOP(i2c);

                //I2C_MasterTxCpltCallback(index);
            }
            break;

        case 0x28: /* DATA transmitted ACK */
            if(t->tx_pos < t->tx_len)
            {
                I2C_SET_DATA(i2c, t->tx_buf[t->tx_pos++]);
                I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI_Msk);
            }
            else
            {
                t->state = NUC4x2_I2C_DONE;

                if(t->stop)
                    I2C_STOP(i2c);

                I2C_MasterTxCpltCallback(index);
            }
            break;

        case 0x40: /* SLA+R ACK */
            if(t->rx_len > 1)
                I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI_Msk | I2C_CTL_AA_Msk);
            else
                I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI_Msk);
            break;

        case 0x50: /* DATA received ACK */
            t->rx_buf[t->rx_pos++] = I2C_GET_DATA(i2c);

            if(t->rx_pos < (t->rx_len - 1))
                I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI_Msk | I2C_CTL_AA_Msk);
            else
                I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI_Msk);
            break;

        case 0x58: /* DATA received NACK */
            t->rx_buf[t->rx_pos++] = I2C_GET_DATA(i2c);

            t->state = NUC4x2_I2C_DONE;

            if(t->stop)
                I2C_STOP(i2c);

            I2C_MasterRxCpltCallback(index);
            break;

        case 0x20: /* SLA+W NACK */
        case 0x30: /* DATA TX NACK */
        case 0x48: /* SLA+R NACK */
        case 0x38: /* arbitration lost */
        default:
            t->state = NUC4x2_I2C_ERROR;
            t->result = hwI2C_BusError;

            I2C_STOP(i2c);
            I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI_Msk);

            I2C_ErrorCallback(index);
            break;
    }
}

void I2C0_IRQHandler(void) { I2C_IRQ_Process(hwI2C_Index_0); }
void I2C1_IRQHandler(void) { I2C_IRQ_Process(hwI2C_Index_1); }
void I2C2_IRQHandler(void) { I2C_IRQ_Process(hwI2C_Index_2); }
void I2C3_IRQHandler(void) { I2C_IRQ_Process(hwI2C_Index_3); }
void I2C4_IRQHandler(void) { I2C_IRQ_Process(hwI2C_Index_4); }

static void I2C_Enable_Clock(hwI2C_Index index)
{
    switch(index)
    {
#if defined(I2C0_BASE)
        case hwI2C_Index_0:
            CLK_EnableModuleClock(I2C0_MODULE);
            break;
#endif
#if defined(I2C1_BASE)
        case hwI2C_Index_1:
            CLK_EnableModuleClock(I2C1_MODULE);
            break;
#endif
#if defined(I2C2_BASE)
        case hwI2C_Index_2:
            CLK_EnableModuleClock(I2C2_MODULE);
            break;
#endif
#if defined(I2C3_BASE)
        case hwI2C_Index_3:
            CLK_EnableModuleClock(I2C3_MODULE);
            break;
#endif
#if defined(I2C4_BASE)
        case hwI2C_Index_4:
            CLK_EnableModuleClock(I2C4_MODULE);
            break;
#endif
    }

    return;
}

static void I2C_Disable_Clock(hwI2C_Index index)
{
    switch(index)
    {
#if defined(I2C0_BASE)
        case hwI2C_Index_0:
            CLK_DisableModuleClock(I2C0_MODULE);
            break;
#endif
#if defined(I2C1_BASE)
        case hwI2C_Index_1:
            CLK_DisableModuleClock(I2C1_MODULE);
            break;
#endif
#if defined(I2C2_BASE)
        case hwI2C_Index_2:
            CLK_DisableModuleClock(I2C2_MODULE);
            break;
#endif
#if defined(I2C3_BASE)
        case hwI2C_Index_3:
            CLK_DisableModuleClock(I2C3_MODULE);
            break;
#endif
#if defined(I2C4_BASE)
        case hwI2C_Index_4:
            CLK_DisableModuleClock(I2C4_MODULE);
            break;
#endif

        default:
            break;
    }
}

hwI2C_OpResult I2C_Instance_Init(hwI2C_Index index, hwI2C_Speed_Mode speed_mode)
{
    if(index >= hwI2C_Index_MAX)
        return hwI2C_InvalidParameter;

    if(I2C_Master_Init_Status[index])
        return hwI2C_OK;

    if(speed_mode >= hwI2C_Speed_Mode_MAX)
        return hwI2C_InvalidParameter;

    I2C_T *i2c = I2C_Map_Soc_Base(index);

    if(i2c == NULL)
        return hwI2C_InvalidParameter;

    memset(&i2c_xfer[index], 0, sizeof(i2c_xfer[index]));
    i2c_xfer[index].state = NUC4x2_I2C_IDLE;

    I2C_Enable_Clock(index);

    switch(speed_mode)
    {
        case hwI2C_Standard_Mode:
            I2C_Open(i2c, I2C_MASTER_STANDARD_MODE_CLK_FREQUENCY);
            break;

        case hwI2C_Fast_Mode:
            I2C_Open(i2c, I2C_MASTER_FAST_MODE_CLK_FREQUENCY);
            break;

        case hwI2C_High_Speed_Mode:
            I2C_Open(i2c, I2C_MASTER_HIGH_SPEED_MODE_CLK_FREQUENCY);
            break;
    }

    I2C_DisableInt(i2c);
    I2C_EnableInt(i2c);

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Instance_DeInit(hwI2C_Index index)
{
    if(index >= hwI2C_Index_MAX)
        return hwI2C_InvalidParameter;

    if(!I2C_Master_Init_Status[index])
        return hwI2C_OK;

    I2C_T *i2c = I2C_Map_Soc_Base(index);

    if(i2c == NULL)
        return hwI2C_InvalidParameter;

    I2C_DisableInt(i2c);
    I2C_STOP(i2c);
    I2C_Close(i2c);

    I2C_Disable_Clock(index);

    memset(&i2c_xfer[index], 0, sizeof(i2c_xfer[index]));
    i2c_xfer[index].state = NUC4x2_I2C_IDLE;

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Transfer_Write(hwI2C_Index index, uint8_t address, uint8_t *write_dat, uint8_t write_len, bool stop, NeonRTOS_Time_t timeoutMs)
{
    if(index >= hwI2C_Index_MAX)
        return hwI2C_InvalidParameter;

    if(!I2C_Master_Init_Status[index])
        return hwI2C_NotInit;

    if(write_dat == NULL || write_len == 0)
        return hwI2C_InvalidParameter;

    I2C_T *i2c = I2C_Map_Soc_Base(index);

    if(i2c == NULL)
        return hwI2C_InvalidParameter;

    NUC4x2_I2C_Transfer *t = &i2c_xfer[index];

    if(t->state == NUC4x2_I2C_TX || t->state == NUC4x2_I2C_RX)
        return hwI2C_HwError;

    memset(t, 0, sizeof(*t));

    t->state  = NUC4x2_I2C_TX;
    t->addr   = address;
    t->tx_buf = write_dat;
    t->tx_len = write_len;
    t->stop   = stop;
    t->result = hwI2C_OK;

    I2C_START(i2c);

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Transfer_Read(hwI2C_Index index, uint8_t address, uint8_t *read_dat, uint8_t read_len, bool stop, NeonRTOS_Time_t timeoutMs)
{
    if(index >= hwI2C_Index_MAX)
        return hwI2C_InvalidParameter;

    if(!I2C_Master_Init_Status[index])
        return hwI2C_NotInit;

    if(read_dat == NULL || read_len == 0)
        return hwI2C_InvalidParameter;

    I2C_T *i2c = I2C_Map_Soc_Base(index);

    if(i2c == NULL)
        return hwI2C_InvalidParameter;

    NUC4x2_I2C_Transfer *t = &i2c_xfer[index];

    if(t->state == NUC4x2_I2C_TX || t->state == NUC4x2_I2C_RX)
        return hwI2C_HwError;

    memset(t, 0, sizeof(*t));

    t->state  = NUC4x2_I2C_RX;
    t->addr   = address;
    t->rx_buf = read_dat;
    t->rx_len = read_len;
    t->stop   = stop;
    t->result = hwI2C_OK;

    I2C_START(i2c);

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Transfer_Get_Status(hwI2C_Index index)
{
    if(index >= hwI2C_Index_MAX)
        return hwI2C_InvalidParameter;

    if(!I2C_Master_Init_Status[index])
        return hwI2C_NotInit;

    NUC4x2_I2C_Transfer *t = &i2c_xfer[index];

    return (t->state == NUC4x2_I2C_DONE) ? hwI2C_OK : t->result;;
}

hwI2C_OpResult I2C_Transfer_Stop(hwI2C_Index index)
{
    if(index >= hwI2C_Index_MAX)
        return hwI2C_InvalidParameter;

    if(!I2C_Master_Init_Status[index])
        return hwI2C_NotInit;

    I2C_T *i2c = I2C_Map_Soc_Base(index);

    if(i2c == NULL)
        return hwI2C_InvalidParameter;

    NUC4x2_I2C_Transfer *t = &i2c_xfer[index];

    I2C_STOP(i2c);
    t->state = NUC4x2_I2C_ERROR;

    return hwI2C_OK;
}

void I2C_NVIC_Init(hwI2C_Index index)
{
    switch(index)
    {
#if defined(I2C0_BASE)
        case hwI2C_Index_0:
            NVIC_ClearPendingIRQ(I2C0_IRQn);
            NVIC_EnableIRQ(I2C0_IRQn);
            break;
#endif
#if defined(I2C1_BASE)
        case hwI2C_Index_1:
            NVIC_ClearPendingIRQ(I2C1_IRQn);
            NVIC_EnableIRQ(I2C1_IRQn);
            break;
#endif
#if defined(I2C2_BASE)
        case hwI2C_Index_2:
            NVIC_ClearPendingIRQ(I2C2_IRQn);
            NVIC_EnableIRQ(I2C2_IRQn);
            break;
#endif
#if defined(I2C3_BASE)
        case hwI2C_Index_3:
            NVIC_ClearPendingIRQ(I2C3_IRQn);
            NVIC_EnableIRQ(I2C3_IRQn);
            break;
#endif
#if defined(I2C4_BASE)
        case hwI2C_Index_4:
            NVIC_ClearPendingIRQ(I2C4_IRQn);
            NVIC_EnableIRQ(I2C4_IRQn);
            break;
#endif

        default:
            break;
    }
}

void I2C_NVIC_DeInit(hwI2C_Index index)
{
    switch(index)
    {
#if defined(I2C0_BASE)
        case hwI2C_Index_0:
            NVIC_DisableIRQ(I2C0_IRQn);
            NVIC_ClearPendingIRQ(I2C0_IRQn);
            break;
#endif
#if defined(I2C1_BASE)
        case hwI2C_Index_1:
            NVIC_DisableIRQ(I2C1_IRQn);
            NVIC_ClearPendingIRQ(I2C1_IRQn);
            break;
#endif
#if defined(I2C2_BASE)
        case hwI2C_Index_2:
            NVIC_DisableIRQ(I2C2_IRQn);
            NVIC_ClearPendingIRQ(I2C2_IRQn);
            break;
#endif
#if defined(I2C3_BASE)
        case hwI2C_Index_3:
            NVIC_DisableIRQ(I2C3_IRQn);
            NVIC_ClearPendingIRQ(I2C3_IRQn);
            break;
#endif
#if defined(I2C4_BASE)
        case hwI2C_Index_4:
            NVIC_DisableIRQ(I2C4_IRQn);
            NVIC_ClearPendingIRQ(I2C4_IRQn);
            break;
#endif

        default:
            break;
    }
}

#endif // NUC442 || NUC472