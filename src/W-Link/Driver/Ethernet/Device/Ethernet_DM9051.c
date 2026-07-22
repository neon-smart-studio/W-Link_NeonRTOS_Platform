#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "Ethernet/Ethernet.h"

#include "Ethernet/Ethernet_Def.h"

#include "GPIO/GPIO.h"

#include "SPI/SPI_Master.h"

#ifdef CONFIG_ETHERNET_DM9051

#include "GPIO/Device/STM32/GPIO_STM32.h"

#define LAN8742A_PHY_ADDRESS            0x00U

/* Definition of the Ethernet driver buffers size and count */
#define ETH_RX_BUF_SIZE     ETH_MAX_PACKET_SIZE /* buffer size for receive               */
#define ETH_TX_BUF_SIZE     ETH_MAX_PACKET_SIZE /* buffer size for transmit              */

#ifndef ETH_RX_DESC_CNT
#define ETH_RX_DESC_CNT 4U
#endif

#ifndef ETH_TX_DESC_CNT
#define ETH_TX_DESC_CNT 4U
#endif

#define ETH_RXBUFNB ETH_RX_DESC_CNT
#define ETH_TXBUFNB ETH_TX_DESC_CNT

#ifndef ETH_DMA_TRANSMIT_TIMEOUT
#define ETH_DMA_TRANSMIT_TIMEOUT     100U
#endif

#define DM9051_ID 0x90510A46

#define DM9051_NCR             	(0x00)
#define DM9051_NSR             	(0x01)
#define DM9051_TCR              (0x02)
#define DM9051_RCR              (0x05)
#define DM9051_BPTR             (0x08)
#define DM9051_FCTR             (0x09)
#define DM9051_FCR              (0x0A)
#define DM9051_EPCR             (0x0B)
#define DM9051_EPAR             (0x0C)
#define DM9051_EPDRL            (0x0D)
#define DM9051_EPDRH            (0x0E)
#define DM9051_PAR              (0x10)
#define DM9051_MAR              (0x16)
#define DM9051_GPCR	       		(0x1E)
#define DM9051_GPR              (0x1f)
#define DM9051_TRPAL            (0x22)
#define DM9051_TRPAH            (0x23)
#define DM9051_RWPAL            (0x24)
#define DM9051_RWPAH            (0x25)

#define DM9051_VIDL             (0x28)
#define DM9051_VIDH             (0x29)
#define DM9051_PIDL             (0x2A)
#define DM9051_PIDH             (0x2B)

#define DM9051_CHIPR            (0x2C)
#define DM9051_TCR2             (0x2D)
#define DM9051_OTCR             (0x2E)
#define DM9051_SMCR             (0x2F)

#define DM9051_INTCR			0x39
#define DM9051_PPCR				0x3D

#define DM9051_INTR			    (0x39)
#define DM9051_MPCR	       		(0x55)

#define DM9051_MRRL             0x74 //0xF4
#define DM9051_MRRH             0x75 //0xF5
#define DM9051_MWRL             0x7A //0xFA
#define DM9051_MWRH             0x7B //0xFB
#define DM9051_TXPLL            0x7C //0xFC
#define DM9051_TXPLH            0x7D //0xFD
#define DM9051_ISR             	0x7E //0xFE
#define DM9051_IMR             	0x7F //0xFF

#define DM9051_MRCMDX           (0x70)
#define DM9051_MRCMDX1          (0x71)
#define DM_SPI_MRCMDX			(0x70)
#define DM_SPI_MRCMD			(0x72)
#define DM_SPI_MWCMD			(0x78)

#define DM_SPI_RD				(0x00)
#define DM_SPI_WR				(0x80)

#define DM9051_ATCR             (0x30)
#define DM9051_NLEDCR           (0x57)
#define DM9051_BCASTCR          (0X53)
#define DM9051_INTCKCR          (0x54)

/* DM9051 PHY register list */
#define DM9051_PHY              (0x40)    /* PHY address 0x01 */
#define DM9051_PHY_REG_BMCR     (0x00) /* Basic Mode Control Register */
#define DM9051_PHY_REG_BMSR     (0x01) /* Basic Mode Status Register */
#define DM9051_PHY_REG_PHYID1   (0x02) /* PHY ID Identifier Register #1 */
#define DM9051_PHY_REG_PHYID2   (0x03) /* PHY ID Identifier Register #2 */
#define DM9051_PHY_REG_ANAR     (0x04) /* Auto-Negotiation Advertisement Register */
#define DM9051_PHY_REG_ANLPAR   (0x05) /* Auto-Negotiation Link Partner Ability Register */
#define DM9051_PHY_REG_ANER     (0x06) /* Auto-Negotiation Expansion Register */
#define DM9051_PHY_REG_DSCR     (0x10) /* DAVICOM Specified Configuration Register  */
#define DM9051_PHY_REG_DSCSR    (0x11) /* DAVICOM Specified Configuration and Status Register  */
#define DM9051_PHY_REG_10BTCSR  (0x12) /* 10BASE-T Configuration/Status */
#define DM9051_PHY_REG_PWDOR    (0x13) /* Power Down Control Register */
#define DM9051_PHY_REG_SCR      (0x14) /* Specified Config Register */
#define DM9051_PHY_REG_PSCR     (0x1D) /* Power Saving Control Register */

/********* register define *********/
#define DM9051_NCR_REG_RESET    (0x01)
#define NCR_DEFAULT		        (0x00)						// Disable Wakeup

//0x01
#define NSR_SPEED           (1 << 7)
#define NSR_LINKST          (1 << 6)
#define NSR_WAKEST          (1 << 5)
#define NSR_TX2END          (1 << 3)
#define NSR_TX1END          (1 << 2)
#define NSR_RXOV            (1 << 1)
#define NSR_RXRDY           (1 << 0)
//#define NSR_CLR_STATUS        (NSR_WAKEST | NSR_TX2END | NSR_TX1END)
#define NSR_CLR_STATUS      (NSR_WAKEST)

/* 0x02 */
#define TCR_TJDIS           (1 << 6)
#define TCR_EXCECM          (1 << 5)
#define TCR_PAD_DIS2        (1 << 4)
#define TCR_CRC_DIS2        (1 << 3)
#define TCR_PAD_DIS1        (1 << 2)
#define TCR_CRC_DIS1        (1 << 1)
#define TCR_TXREQ           (1 << 0)		//Start TX
#define TCR_DEFAULT		    (0x00)

//0x05
#define RCR_WTDIS           (1 << 6)
#define RCR_DIS_LONG        (1 << 5)
#define RCR_DIS_CRC         (1 << 4)
#define RCR_ALL	            (1 << 3)
#define RCR_RUNT            (1 << 2)
#define RCR_PRMSC           (1 << 1)
#define RCR_RXEN            (1 << 0)
#define RCR_RX_DISABLE      (RCR_DIS_LONG | RCR_DIS_CRC) // #define RCR_RX_DISABLE 0x30
#define RCR_DEFAULT		    (RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN)

#define BPTR_DEFAULT	    (0x3f)
#define FCTR_DEAFULT	    (0x38)
#define FCR_DEFAULT		    (0x28)
#define SMCR_DEFAULT	    (0x00)

//0x0A
#define FCR_FLOW_ENABLE		 0x29
//0x1E
#define GPCR_GEP_CNTL       (1<<0)
//0x39
#define INTCR_POL       	(1<<0)
//0x3D
//#define PPCR_SETTING		 0x00 (Trouble in the way)
//#define PPCR_SETTING		 0x01 (default)
//#define PPCR_SETTING		 0x02 (TO BE TRY ONCE LATER)
//#define PPCR_SETTING		 0x08 (Using now, To work to)
#define PPCR_SETTING		 0x08
//0x55
#define MPCR_RSTTX          (1<<1)
#define MPCR_RSTRX          (1<<0)
//0xFE
#define ISR_LNKCHGS         (1<<5)
#define ISR_ROOS            (1<<3)
#define ISR_ROS             (1<<2)
#define ISR_PTS             (1<<1)
#define ISR_PRS             (1<<0)
#define ISR_CLR_STATUS      (ISR_LNKCHGS | ISR_ROOS | ISR_ROS | ISR_PTS)
#define ISR_CLR_RX_STATUS   (ISR_PRS)

//0xFF
#define IMR_PAR             (1<<7)
#define IMR_LNKCHGI         (1<<5)
#define IMR_ROOI            (1<<3)
#define IMR_ROI             (1<<2)
#define IMR_PTM             (1<<1)
#define IMR_PRM             (1<<0)
#define DM9051_IMR_OFF      (IMR_PAR)
#define DM9051_IMR_SET      (IMR_PAR | IMR_ROOI | IMR_ROI | IMR_PRM | IMR_LNKCHGI)

//Const
#define DM9051_PKT_RDY		0x01	/* Packet ready to receive */
#define DM9051_PKT_MAX		1536	/* Received packet max size */

#define DM9051_REG_RESET     (0x01)
#define DM9051_TCR2_SET      (0x90)	//one packet
#define DM9051_BPTR_SET      (0x37)
#define DM9051_FCTR_SET      (0x38)
#define DM9051_FCR_SET       (0x28)
#define DM9051_TCR_SET       (0x01)

/*
 * dm9000 Ethernet
 */
//#define DM9000_NSR             0x01
#define DM9000_TCR             0x02
#define DM9000_RSR             0x06
#define DM9000_BPTR            0x08
#define DM9000_EPCR            0x0B
#define DM9000_EPAR            0x0C
#define DM9000_EPDRL           0x0D
#define DM9000_EPDRH           0x0E
#define DM9000_MAR             0x16
#define DM9000_GPR             0x1F
#define DM9000_SMCR            0x2F

//0x00
#define NCR_WAKEEN          (1<<6)
#define NCR_FDX             (1<<3)
#define NCR_RST	            (1<<0)

//0x06
#define RSR_RF              (1<<7)
#define RSR_MF              (1<<6)
#define RSR_LCS             (1<<5)
#define RSR_RWTO            (1<<4)
#define RSR_PLE             (1<<3)
#define RSR_AE              (1<<2)
#define RSR_CE              (1<<1)
#define RSR_FOE             (1<<0)
//0x0B
#define EPCR_WEP			(1<<4) //=0x10
#define EPCR_EPOS           (1<<3)
#define EPCR_ERPRR          (1<<2)
#define EPCR_ERPRW          (1<<1)
#define EPCR_ERRE           (1<<0)

#ifndef CONFIG_DM9051_IRQ_PIN
#define DM9051_IRQ_Pin hwGPIO_Int_Pin_C4
#else
#define DM9051_IRQ_Pin CONFIG_DM9051_IRQ_PIN
#endif

#ifndef CONFIG_DM9051_RST_PIN
#define DM9051_RST_Pin hwGPIO_Pin_A9
#else
#define DM9051_RST_Pin CONFIG_DM9051_RST_PIN
#endif

#ifndef CONFIG_DM9051_CS_PIN
#define DM9051_CS_Pin hwGPIO_Pin_A4
#else
#define DM9051_CS_Pin CONFIG_DM9051_CS_PIN
#endif

#ifndef CONFIG_DM9051_SPI_INDEX
#define DM9051_SPI_INDEX hwSPI_Index_0
#else
#define DM9051_SPI_INDEX CONFIG_DM9051_SPI_INDEX
#endif

#define DM9051_CS_LOW()     GPIO_Pin_Write(DM9051_CS_Pin, 0)
#define DM9051_CS_HIGH()    GPIO_Pin_Write(DM9051_CS_Pin, 1)

/* The Ethernet stack can call RX and TX from different tasks.  Keep every
 * complete DM9051 SPI transaction serialized; otherwise CS can be toggled by
 * another task in the middle of a register/FIFO access. */
static NeonRTOS_LockObj_t dm9051_spi_mutex;
static bool dm9051_spi_mutex_ready = false;

static bool ETH_Init = false;
static onLinkUpCallback onLinkUpCB = NULL;
static onLinkDownCallback onLinkDownCB = NULL;

static uint8_t saved_mac[6] = {0, 0, 0, 0, 0, 0};
static uint32_t saved_ETH_HashTableLow = 0;
static uint32_t saved_ETH_HashTableHigh = 0;

static uint16_t tx_calc_MWR = 0;
static uint16_t rx_calc_MRR = 0;
static uint16_t rx_pending_len = 0;
static bool rx_frame_pending = false;

static uint32_t dm9051_rx_err_count = 0;
static uint32_t dm9051_rx_err_total = 0;

#define DM9051_INIT_RETRY_COUNT       5U
#define DM9051_INIT_RETRY_DELAY_MS  200U
#define DM9051_PHY_TIMEOUT_MS       1000U
#define DM9051_RX_STATUS_ERROR_MASK 0x3900U

static inline void DM9051_SPI_Lock(void)
{
    if (dm9051_spi_mutex_ready) {
        NeonRTOS_LockObjLock(&dm9051_spi_mutex, NEONRT_WAIT_FOREVER);
    }
}

static inline void DM9051_SPI_Unlock(void)
{
    if (dm9051_spi_mutex_ready) {
        NeonRTOS_LockObjUnlock(&dm9051_spi_mutex);
    }
}

static void DM9051_Hardware_Reset_NoLock(void)
{
    GPIO_Pin_Write(DM9051_RST_Pin, 0);
    NeonRTOS_Sleep(2);
    GPIO_Pin_Write(DM9051_RST_Pin, 1);
    NeonRTOS_Sleep(2);
}

void DM9051_Hardware_Reset(void)
{
    DM9051_SPI_Lock();
    DM9051_Hardware_Reset_NoLock();
    rx_frame_pending = false;
    rx_pending_len = 0;
    DM9051_SPI_Unlock();
}

static uint8_t DM9051_Read_Reg_NoLock(uint8_t reg)
{
    uint8_t rx = 0;
    uint8_t dummy = 0xFF;

    DM9051_CS_LOW();
    SPI_Master_TransferByte(DM9051_SPI_INDEX, reg, &rx);
    SPI_Master_TransferByte(DM9051_SPI_INDEX, dummy, &rx);
    DM9051_CS_HIGH();

    return rx;
}

static void DM9051_Write_Reg_NoLock(uint8_t reg, uint8_t val)
{
    DM9051_CS_LOW();
    reg |= DM_SPI_WR;
    SPI_Master_WriteByte(DM9051_SPI_INDEX, reg);
    SPI_Master_WriteByte(DM9051_SPI_INDEX, val);
    DM9051_CS_HIGH();
}

/*------------------------- Read FIFO -------------------------*/
static void DM9051_Read_Mem_NoLock(uint8_t *buf, uint16_t len)
{
    const uint8_t cmd = DM_SPI_MRCMD;

    DM9051_CS_LOW();
    SPI_Master_WriteByte(DM9051_SPI_INDEX, cmd);

    if (len <= 4U) {
        uint8_t dummy = 0xFF;
        for (uint16_t i = 0; i < len; i++) {
            SPI_Master_TransferByte(DM9051_SPI_INDEX, dummy, &buf[i]);
        }
    } else {
        SPI_Master_Burst_Read(DM9051_SPI_INDEX, buf, len);
    }

    DM9051_CS_HIGH();
}

static void DM9051_Write_Mem_NoLock(const uint8_t *buf, uint16_t len)
{
    const uint8_t cmd = DM_SPI_MWCMD | DM_SPI_WR;

    DM9051_CS_LOW();
    SPI_Master_WriteByte(DM9051_SPI_INDEX, cmd);

    if (len <= 4U) {
        for (uint16_t i = 0; i < len; i++) {
            SPI_Master_WriteByte(DM9051_SPI_INDEX, buf[i]);
        }
    } else {
        SPI_Master_Burst_Write(DM9051_SPI_INDEX, buf, len);
    }

    DM9051_CS_HIGH();
}

static void DM9051_Soft_Reset_NoLock(const uint8_t mac[6])
{
    NeonRTOS_Sleep(2);
    DM9051_Write_Reg_NoLock(DM9051_NCR, DM9051_NCR_REG_RESET);
    NeonRTOS_Sleep(2);
    DM9051_Write_Reg_NoLock(DM9051_NCR, 0);

    DM9051_Write_Reg_NoLock(DM9051_NCR, NCR_DEFAULT);
    DM9051_Write_Reg_NoLock(DM9051_IMR, DM9051_IMR_OFF);
    DM9051_Write_Reg_NoLock(DM9051_TCR, TCR_DEFAULT);
    DM9051_Write_Reg_NoLock(DM9051_BPTR, BPTR_DEFAULT);
    DM9051_Write_Reg_NoLock(DM9051_FCTR, FCTR_DEAFULT);
    DM9051_Write_Reg_NoLock(DM9051_FCR, FCR_DEFAULT);
    DM9051_Write_Reg_NoLock(DM9051_INTCR, 0x00);
    DM9051_Write_Reg_NoLock(DM9051_INTCKCR, 0x81);

    DM9051_Write_Reg_NoLock(DM9051_NSR, NSR_CLR_STATUS);
    DM9051_Write_Reg_NoLock(DM9051_ISR, ISR_CLR_STATUS | ISR_PRS);

#ifdef DM9051_FLOWCONTROL_EN
    DM9051_Write_Reg_NoLock(DM9051_FCR, FCR_FLOW_ENABLE);
#else
    DM9051_Write_Reg_NoLock(DM9051_FCR, 0x00);
#endif
    DM9051_Write_Reg_NoLock(DM9051_PPCR, PPCR_SETTING);
    DM9051_Write_Reg_NoLock(DM9051_NLEDCR, 0x81);
    DM9051_Write_Reg_NoLock(DM9051_ATCR, 0x80);
    DM9051_Write_Reg_NoLock(DM9051_BCASTCR, 0xC0);

    for (int i = 0; i < 6; i++) {
        DM9051_Write_Reg_NoLock((uint8_t)(DM9051_PAR + i), mac[i]);
    }

    DM9051_Write_Reg_NoLock(DM9051_RCR, RCR_DEFAULT);
    rx_frame_pending = false;
    rx_pending_len = 0;
}

static bool DM9051_PHY_Write_NoLock(uint16_t reg, uint16_t value)
{
    uint32_t timeout = DM9051_PHY_TIMEOUT_MS;

    DM9051_Write_Reg_NoLock(DM9051_EPAR, (uint8_t)(DM9051_PHY | reg));
    DM9051_Write_Reg_NoLock(DM9051_EPDRL, value & 0xffU);
    DM9051_Write_Reg_NoLock(DM9051_EPDRH, (value >> 8) & 0xffU);
    DM9051_Write_Reg_NoLock(DM9051_EPCR, 0x0a);

    while ((DM9051_Read_Reg_NoLock(DM9051_EPCR) & 0x01U) != 0U) {
        if (timeout-- == 0U) {
            DM9051_Write_Reg_NoLock(DM9051_EPCR, 0x00);
            UART_Printf("[DM9051] PHY write timeout, reg=%u\n", reg);
            return false;
        }
        NeonRTOS_Sleep(1);
    }

    DM9051_Write_Reg_NoLock(DM9051_EPCR, 0x00);
    return true;
}

static bool DM9051_PHY_Mode_Set_NoLock(void)
{
    bool ok = true;
    uint16_t phy_reg4 = 0x01e1;
    uint16_t phy_reg0 = 0x1200;

    ok = DM9051_PHY_Write_NoLock(20, 0x0200) && ok;
#ifdef DM9051_FLOWCONTROL_EN
    ok = DM9051_PHY_Write_NoLock(4, phy_reg4 | 0x0400) && ok;
#else
    ok = DM9051_PHY_Write_NoLock(4, phy_reg4) && ok;
#endif
    ok = DM9051_PHY_Write_NoLock(0, phy_reg0) && ok;
    return ok;
}

static void DM9051_Record_RX_Error(void)
{
    dm9051_rx_err_count++;
    dm9051_rx_err_total++;
}

/* Reset only the RX path.  A full chip reset on a single bad ready byte can
 * restart auto-negotiation and turn a recoverable FIFO error into a long or
 * permanent link outage. */
static void DM9051_RX_Soft_Recover_NoLock(void)
{
    DM9051_Write_Reg_NoLock(DM9051_RCR, 0x00);
    DM9051_Write_Reg_NoLock(DM9051_MPCR, MPCR_RSTRX);
    NeonRTOS_Sleep(2);
    DM9051_Write_Reg_NoLock(DM9051_ISR, ISR_CLR_STATUS | ISR_PRS);
    DM9051_Write_Reg_NoLock(DM9051_RCR, RCR_DEFAULT);
    DM9051_Write_Reg_NoLock(DM9051_IMR, DM9051_IMR_SET);
    rx_frame_pending = false;
    rx_pending_len = 0;
}

/* polynomial: 0xEDB88320L */
static uint32_t DM9051_CRC32_LE(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xffffffff;

    int i;
    while (length--)
    {
        crc ^= *data++;
        for (i = 0; i < 8; i++)
        {
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320L : 0);
        }
    }
    return crc;
}

hwEthernet_OpResult Ethernet_Init(const uint8_t mac[6], onLinkUpCallback link_up_cb, onLinkDownCallback link_down_cb)
{
    int i, oft;
    uint32_t device_id = 0;

    if (mac == NULL) {
        return hwEthernet_InvalidParameter;
    }

    GPIO_Pin_Init(DM9051_RST_Pin, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    GPIO_Pin_Init(DM9051_CS_Pin, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    SPI_Master_Init(DM9051_SPI_INDEX, 20000000, hwSPI_OpMode_Polarity0_Phase0, false);

    if (!dm9051_spi_mutex_ready) {
        if (NeonRTOS_LockObjCreate(&dm9051_spi_mutex) != NeonRTOS_OK) {
            UART_Printf("[DM9051] Failed to create SPI mutex\n");
            return hwEthernet_HwError;
        }
        dm9051_spi_mutex_ready = true;
    }

    ETH_Init = false;
    DM9051_SPI_Lock();

    for (uint32_t retry = 0; retry < DM9051_INIT_RETRY_COUNT; retry++) {
        if (retry != 0U) {
            UART_Printf("[DM9051] Init retry %u/%u\n",
                        (unsigned)(retry + 1U),
                        (unsigned)DM9051_INIT_RETRY_COUNT);
            NeonRTOS_Sleep(DM9051_INIT_RETRY_DELAY_MS);
        }

        DM9051_Hardware_Reset_NoLock();
        NeonRTOS_Sleep(100);

        DM9051_Write_Reg_NoLock(DM9051_NCR, DM9051_NCR_REG_RESET);
        NeonRTOS_Sleep(2);
        DM9051_Write_Reg_NoLock(DM9051_NCR, 0);
        DM9051_Write_Reg_NoLock(DM9051_GPCR, GPCR_GEP_CNTL);
        DM9051_Write_Reg_NoLock(DM9051_GPR, 0x00);
        NeonRTOS_Sleep(100);

        device_id  = DM9051_Read_Reg_NoLock(DM9051_VIDL);
        device_id |= (uint32_t)DM9051_Read_Reg_NoLock(DM9051_VIDH) << 8;
        device_id |= (uint32_t)DM9051_Read_Reg_NoLock(DM9051_PIDL) << 16;
        device_id |= (uint32_t)DM9051_Read_Reg_NoLock(DM9051_PIDH) << 24;
        UART_Printf("[%s L%d] device_id: %08X\n", __FUNCTION__, __LINE__, device_id);

        if (device_id == DM9051_ID) {
            break;
        }
    }

    if (device_id != DM9051_ID) {
        UART_Printf("[DM9051] Init failed after %u retries\n",
                    (unsigned)DM9051_INIT_RETRY_COUNT);
        DM9051_SPI_Unlock();
        return hwEthernet_HwError;
    }

    device_id = DM9051_Read_Reg_NoLock(DM9051_CHIPR);
    UART_Printf("[%s L%d] CHIP Revision: %02X\n", __FUNCTION__, __LINE__, device_id);

    memcpy(saved_mac, mac, 6);
    saved_ETH_HashTableLow = 0;
    saved_ETH_HashTableHigh = 0;

    if (!DM9051_PHY_Mode_Set_NoLock()) {
        UART_Printf("[DM9051] PHY configuration did not complete cleanly\n");
    }

    for (i = 0, oft = DM9051_PAR; i < 6; i++, oft++)
    {
        DM9051_Write_Reg_NoLock((uint8_t)oft, mac[i]);
    }

    for (i = 0, oft = DM9051_MAR; i < 8; i++, oft++)
    {
        DM9051_Write_Reg_NoLock((uint8_t)oft, 0x00);
    }

    UART_Printf("Clean Multicast Address Hash Table\n");

    DM9051_Soft_Reset_NoLock(mac);
    DM9051_Write_Reg_NoLock(DM9051_IMR, DM9051_IMR_SET);

    onLinkUpCB = link_up_cb;
    onLinkDownCB = link_down_cb;
    tx_calc_MWR = 0;
    rx_calc_MRR = 0;
    rx_pending_len = 0;
    rx_frame_pending = false;
    dm9051_rx_err_count = 0;
    dm9051_rx_err_total = 0;
    ETH_Init = true;

    DM9051_SPI_Unlock();
    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Output(const uint8_t *out_data, uint16_t out_len)
{
    if ((out_data == NULL) || (out_len == 0U)) {
        return hwEthernet_InvalidParameter;
    }

    if ((out_len > ETH_MAX_PACKET_SIZE) || (out_len > ETH_TX_BUF_SIZE)) {
        return hwEthernet_BufferError;
    }

    if (!ETH_Init) {
        return hwEthernet_HwError;
    }

    uint32_t retry = 0;
    uint8_t nsr_reg = 0;

    DM9051_SPI_Lock();

    while (1) {
        nsr_reg = DM9051_Read_Reg_NoLock(DM9051_NSR) & (NSR_TX1END | NSR_TX2END);
        if(nsr_reg != 0)
        {
            break;
        }

        retry++;
        if (retry > 10)
        {
            DM9051_SPI_Unlock();
            return hwEthernet_Busy;
        }

        NeonRTOS_Sleep(1);
    }

    if (retry > 2)
    {
        //UART_Printf("TX wait %d.", retry);
    }

    if ((NSR_TX1END | NSR_TX2END) == nsr_reg)
    {
        DM9051_Write_Reg_NoLock(DM9051_MPCR, MPCR_RSTTX);
    }

    tx_calc_MWR = ((uint16_t)DM9051_Read_Reg_NoLock(DM9051_MWRH) << 8) |
                  DM9051_Read_Reg_NoLock(DM9051_MWRL);

    DM9051_Write_Reg_NoLock(DM9051_TXPLL, out_len & 0xffU);
    DM9051_Write_Reg_NoLock(DM9051_TXPLH, (out_len >> 8) & 0xffU);
    DM9051_Write_Mem_NoLock(out_data, out_len);

    tx_calc_MWR += out_len;
    if (tx_calc_MWR > 0x0bff) {
        tx_calc_MWR -= 0x0c00;
    }
    {
        uint16_t actual_MWR = ((uint16_t)DM9051_Read_Reg_NoLock(DM9051_MWRH) << 8) |
                              DM9051_Read_Reg_NoLock(DM9051_MWRL);
        if (tx_calc_MWR != actual_MWR) {
            DM9051_Write_Reg_NoLock(DM9051_MWRH, (tx_calc_MWR >> 8) & 0xffU);
            DM9051_Write_Reg_NoLock(DM9051_MWRL, tx_calc_MWR & 0xffU);
        }
    }

    DM9051_Write_Reg_NoLock(DM9051_TCR, TCR_TXREQ);

    DM9051_SPI_Unlock();
    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Get_Input_Frame_Length(uint32_t *frame_len)
{
    uint8_t isr_reg;
    uint8_t nsr_reg;
    uint16_t rx_status;
    uint16_t rx_len;
    uint8_t receive_data[4];
    uint8_t rx_bytes[2];

    if (frame_len == NULL) {
        return hwEthernet_InvalidParameter;
    }

    if (!ETH_Init) {
        return hwEthernet_HwError;
    }

    DM9051_SPI_Lock();

    /* The API separates header and payload reads.  If the caller asks again
     * before consuming the payload, return the same length instead of moving
     * the FIFO pointer into the middle of the frame. */
    if (rx_frame_pending) {
        *frame_len = rx_pending_len;
        DM9051_SPI_Unlock();
        return hwEthernet_OK;
    }

    DM9051_Write_Reg_NoLock(DM9051_IMR, DM9051_IMR_OFF);
    isr_reg = DM9051_Read_Reg_NoLock(DM9051_ISR);
    DM9051_Write_Reg_NoLock(DM9051_ISR, isr_reg); /* W1C every latched bit */

    if (isr_reg & ISR_ROOS)
    {
        UART_Printf("dm9051 ROOS, soft recover\n");
        DM9051_Record_RX_Error();
        DM9051_RX_Soft_Recover_NoLock();
        DM9051_SPI_Unlock();
        return hwEthernet_BufferError;
    }

    if (isr_reg & ISR_ROS)
    {
        DM9051_Record_RX_Error();
        UART_Printf("Receive_FIFO Overflow #%lu (total=%lu)\n",
                    (unsigned long)dm9051_rx_err_count,
                    (unsigned long)dm9051_rx_err_total);
        DM9051_RX_Soft_Recover_NoLock();
        DM9051_SPI_Unlock();
        return hwEthernet_BufferError;
    }

    nsr_reg = DM9051_Read_Reg_NoLock(DM9051_NSR) & NSR_RXRDY;
    if (nsr_reg)
    {
        rx_bytes[0] = DM9051_Read_Reg_NoLock(DM9051_MRCMDX);  /* dummy */
        rx_bytes[1] = DM9051_Read_Reg_NoLock(DM9051_MRCMDX1); /* ready */

        if (rx_bytes[1] != DM9051_PKT_RDY)
        {
            DM9051_Record_RX_Error();
            UART_Printf("NSR %02X, RCMDX %02X: rx err #%lu (total=%lu), soft recover\n",
                        nsr_reg, rx_bytes[1],
                        (unsigned long)dm9051_rx_err_count,
                        (unsigned long)dm9051_rx_err_total);
            DM9051_RX_Soft_Recover_NoLock();
            DM9051_SPI_Unlock();
            return hwEthernet_BufferError;
        }
    }
    else
    {
        DM9051_Write_Reg_NoLock(DM9051_ISR, ISR_CLR_RX_STATUS);
        DM9051_Write_Reg_NoLock(DM9051_IMR, DM9051_IMR_SET);
        DM9051_SPI_Unlock();
        return hwEthernet_BufferError;
    }

    rx_calc_MRR = ((uint16_t)DM9051_Read_Reg_NoLock(DM9051_MRRH) << 8) |
                  DM9051_Read_Reg_NoLock(DM9051_MRRL);
    DM9051_Read_Mem_NoLock(receive_data, sizeof(receive_data));

    rx_status = (uint16_t)receive_data[0] | ((uint16_t)receive_data[1] << 8);
    rx_len = (uint16_t)receive_data[2] | ((uint16_t)receive_data[3] << 8);

    if (((rx_status & DM9051_RX_STATUS_ERROR_MASK) != 0U) ||
        (rx_len < 14U) || (rx_len > DM9051_PKT_MAX) ||
        (rx_len > ETH_RX_BUF_SIZE)) {
        DM9051_Record_RX_Error();
        UART_Printf("DM9051 bad rx: status=%04X len=%u #%lu (total=%lu), soft recover\n",
                    rx_status, rx_len,
                    (unsigned long)dm9051_rx_err_count,
                    (unsigned long)dm9051_rx_err_total);
        DM9051_RX_Soft_Recover_NoLock();
        DM9051_SPI_Unlock();
        return hwEthernet_BufferError;
    }

    rx_pending_len = rx_len;
    rx_frame_pending = true;
    *frame_len = rx_len;

    /* Keep IMR masked until Ethernet_Input() consumes this payload. */
    DM9051_SPI_Unlock();
    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Input(uint8_t *in_data, uint32_t in_len)
{
    hwEthernet_OpResult result = hwEthernet_OK;

    if (!ETH_Init) {
        return hwEthernet_HwError;
    }

    DM9051_SPI_Lock();

    if (in_data == NULL || in_len == 0U) {
        if (rx_frame_pending) {
            DM9051_RX_Soft_Recover_NoLock();
        }
        DM9051_SPI_Unlock();
        return hwEthernet_InvalidParameter;
    }

    if (!rx_frame_pending || (in_len != rx_pending_len) ||
        (in_len > DM9051_PKT_MAX) || (in_len > ETH_RX_BUF_SIZE)) {
        UART_Printf("[DM9051] RX payload length mismatch: expected=%u actual=%lu\n",
                    rx_pending_len, (unsigned long)in_len);
        if (rx_frame_pending) {
            DM9051_Record_RX_Error();
            DM9051_RX_Soft_Recover_NoLock();
        }
        DM9051_SPI_Unlock();
        return hwEthernet_BufferError;
    }

    DM9051_Read_Mem_NoLock(in_data, (uint16_t)in_len);

    rx_calc_MRR += (uint16_t)(in_len + 4U);
    if (rx_calc_MRR > 0x3fff) {
        rx_calc_MRR -= 0x3400;
    }
    {
        uint16_t actual_MRR = ((uint16_t)DM9051_Read_Reg_NoLock(DM9051_MRRH) << 8) |
                              DM9051_Read_Reg_NoLock(DM9051_MRRL);
        if (rx_calc_MRR != actual_MRR) {
            DM9051_Write_Reg_NoLock(DM9051_MRRH, (rx_calc_MRR >> 8) & 0xffU);
            DM9051_Write_Reg_NoLock(DM9051_MRRL, rx_calc_MRR & 0xffU);
        }
    }

    rx_frame_pending = false;
    rx_pending_len = 0;
    dm9051_rx_err_count = 0;
    if (dm9051_rx_err_total > 0U) {
        dm9051_rx_err_total--;
    }
    DM9051_Write_Reg_NoLock(DM9051_IMR, DM9051_IMR_SET);

    DM9051_SPI_Unlock();
    return result;
}

bool Ethernet_isInit(void)
{
    return ETH_Init;
}

void Ethernet_Set_Link(void)
{
    uint8_t nsr_reg;
    bool link_up;

    if (!ETH_Init) {
        return;
    }

    DM9051_SPI_Lock();
    nsr_reg = DM9051_Read_Reg_NoLock(DM9051_NSR);
    link_up = (nsr_reg & NSR_LINKST) != 0U;
    DM9051_SPI_Unlock();

    if (link_up)
    {
        if (onLinkUpCB != NULL) {
            onLinkUpCB();
        }
    }
    else
    {
        if (onLinkDownCB != NULL) {
            onLinkDownCB();
        }
    }
}

void Ethernet_Update_Config(bool isLinkUp)
{
    (void)isLinkUp;
}

uint32_t Ethernet_Get_Tick(void)
{
  return HAL_GetTick();
}

void Ethernet_Get_Hardware_Mac(uint8_t mac[6])
{
    if (mac == NULL) {
        return;
    }

    // 使用 STM32 的唯一 ID 生成 MAC 地址
    uint32_t baseUID = *(uint32_t *)UID_BASE;
    mac[0] = 0x00;
    mac[1] = 0x60;
    mac[2] = 0x6e;
    mac[3] = (baseUID & 0x00FF0000) >> 16;
    mac[4] = (baseUID & 0x0000FF00) >> 8;
    mac[5] = (baseUID & 0x000000FF);
}

hwEthernet_OpResult Ethernet_Register_Multicast_Address(const uint8_t *mac, uint32_t *eth_HashTableHigh, uint32_t *eth_HashTableLow)
{
    uint32_t crc;
    uint8_t hash;
    uint8_t hash_group;

    if ((mac == NULL) || (eth_HashTableHigh == NULL) ||
        (eth_HashTableLow == NULL)) {
        return hwEthernet_InvalidParameter;
    }

    if (!ETH_Init) {
        return hwEthernet_HwError;
    }

    crc = DM9051_CRC32_LE(mac, 6);

    hash = crc & 0x3F;

    hash_group = hash / 8;

    DM9051_SPI_Lock();

    if (hash > 31)
    {
        saved_ETH_HashTableHigh |= UINT32_C(1) << (hash - 32);
        DM9051_Write_Reg_NoLock(DM9051_MAR + hash_group,
                                (saved_ETH_HashTableHigh >> ((hash_group - 4) * 8)) & 0xffU);
        *eth_HashTableHigh = saved_ETH_HashTableHigh;
    }
    else
    {
        saved_ETH_HashTableLow |= UINT32_C(1) << hash;
        DM9051_Write_Reg_NoLock(DM9051_MAR + hash_group,
                                (saved_ETH_HashTableLow >> (hash_group * 8)) & 0xffU);
        *eth_HashTableLow = saved_ETH_HashTableLow;
    }

    DM9051_SPI_Unlock();
    return hwEthernet_OK;
}

#endif /* CONFIG_ETHERNET_DM9051 */