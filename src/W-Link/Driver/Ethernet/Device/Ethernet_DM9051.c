
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "soc.h"

#include <string.h>

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

static bool ETH_Init = false;

static onLinkUpCallback onLinkUpCB = NULL;
static onLinkDownCallback onLinkDownCB = NULL;

void DM9051_Hardware_Reset()
{
    GPIO_Pin_Write(DM9051_RST_Pin, 0);
    NeonRTOS_Sleep(2);
    GPIO_Pin_Write(DM9051_RST_Pin, 1);
    NeonRTOS_Sleep(2);
}

static uint8_t DM9051_Read_Reg(uint8_t reg)
{
    uint8_t rx = 0;
    uint8_t dummy = 0xFF;

    DM9051_CS_LOW();
    SPI_Master_TransferByte(DM9051_SPI_INDEX, reg, &rx);
    SPI_Master_TransferByte(DM9051_SPI_INDEX, dummy, &rx);
    DM9051_CS_HIGH();

    return rx;
}

static void DM9051_Write_Reg(uint8_t reg, uint8_t val)
{
    DM9051_CS_LOW();
    reg |= DM_SPI_WR;
    SPI_Master_WriteByte(DM9051_SPI_INDEX, reg);
    SPI_Master_WriteByte(DM9051_SPI_INDEX, val);
    DM9051_CS_HIGH();
}

/*------------------------- Read FIFO -------------------------*/
static void DM9051_Read_Mem(uint8_t *buf, uint16_t len)
{
    const uint8_t cmd = DM_SPI_MRCMD;            /* 0x72 */

    DM9051_CS_LOW();

    SPI_Master_WriteByte(DM9051_SPI_INDEX, cmd);

    if (len <= 4) {
        /* ─────≤ FIFO 深度：改走 blocking───── */
        static uint8_t dummy = 0xFF;
        for (uint16_t i = 0; i < len; i++)          /* clock out len 個 0xFF */
        {
            SPI_Master_TransferByte(DM9051_SPI_INDEX, dummy, &buf[i]);
        }
    } else {
        SPI_Master_Burst_Read(DM9051_SPI_INDEX, buf, len);
    }

    DM9051_CS_HIGH();
}

static void DM9051_Write_Mem(const uint8_t *buf, uint16_t len)
{
    const uint8_t cmd = DM_SPI_MWCMD | DM_SPI_WR;/* 0x7A */

    DM9051_CS_LOW();
    
    SPI_Master_WriteByte(DM9051_SPI_INDEX, cmd);

    if (len <= 4) {
        /* ─────≤ FIFO 深度：改走 blocking───── */
        for (uint16_t i = 0; i < len; i++)          /* clock out len 個 0xFF */
        {
            SPI_Master_WriteByte(DM9051_SPI_INDEX, buf[i]);
        }
    } else {
        SPI_Master_Burst_Write(DM9051_SPI_INDEX, buf, len);
    }

    DM9051_CS_HIGH();
}

static void DM9051_Soft_Reset(uint8_t mac[6])
{
    NeonRTOS_Sleep(2); // delay 2 ms any need before NCR_RST (20170510)
    DM9051_Write_Reg(DM9051_NCR, DM9051_NCR_REG_RESET);
    NeonRTOS_Sleep(2);
    DM9051_Write_Reg(DM9051_NCR, 0);

    /* Setup DM9051 Registers */
    DM9051_Write_Reg(DM9051_NCR, NCR_DEFAULT);
    DM9051_Write_Reg(DM9051_IMR, DM9051_IMR_OFF);
    DM9051_Write_Reg(DM9051_TCR, TCR_DEFAULT);
    DM9051_Write_Reg(DM9051_BPTR, BPTR_DEFAULT);
    DM9051_Write_Reg(DM9051_FCTR, FCTR_DEAFULT);
    DM9051_Write_Reg(DM9051_FCR, FCR_DEFAULT);

    //DM9051_Write_Reg(DM9051_INTCR, (0<<1) | (1<<0)); /* [1] 0:push-pull. [0] 0:active high, 1:active low. */
    DM9051_Write_Reg(DM9051_INTCR, 0x00);
    DM9051_Write_Reg(DM9051_INTCKCR, 0x81);

    /* Clear status */
    DM9051_Write_Reg(DM9051_NSR, NSR_CLR_STATUS);
    DM9051_Write_Reg(DM9051_ISR, ISR_CLR_STATUS);

    /* edit */
#ifdef DM9051_FLOWCONTROL_EN
    DM9051_Write_Reg(DM9051_FCR, FCR_FLOW_ENABLE); /* Flow Control */
#else
    DM9051_Write_Reg(DM9051_FCR, 0x00); /* Flow Control */
#endif
    DM9051_Write_Reg(DM9051_PPCR, PPCR_SETTING); /* Fully Pause Packet Count */
    DM9051_Write_Reg(DM9051_NLEDCR, 0x81);       //set led model
    DM9051_Write_Reg(DM9051_ATCR, 0x80);         //set TX auto_send
    DM9051_Write_Reg(DM9051_BCASTCR, 0xC0);      //set rec broadcast packet

    for (int i = 0; i < 6; i++) {
        DM9051_Write_Reg(DM9051_PAR + i, mac[i]);
    }
    
    DM9051_Write_Reg(DM9051_RCR, RCR_DEFAULT);
}

static void DM9051_PHY_Write(uint16_t reg, uint16_t value)
{
    /* Fill the phyxcer register into REG_0C */
    DM9051_Write_Reg(DM9051_EPAR, DM9051_PHY | reg);

    /* Fill the written data into REG_0D & REG_0E */
    DM9051_Write_Reg(DM9051_EPDRL, (value & 0xff));
    DM9051_Write_Reg(DM9051_EPDRH, ((value >> 8) & 0xff));
    DM9051_Write_Reg(DM9051_EPCR, 0xa); /* Issue phyxcer write command */

    while (DM9051_Read_Reg(DM9051_EPCR) & 0x1)
    {
        NeonRTOS_Sleep(1);
    }; //Wait complete

    DM9051_Write_Reg(DM9051_EPCR, 0x0); /* Clear phyxcer write command */
}

static uint16_t DM9051_PHY_Read(uint32_t reg)
{
    uint16_t value;

    /* Fill the phyxcer register into REG_0C */
    DM9051_Write_Reg(DM9051_EPAR, DM9051_PHY | reg);
    DM9051_Write_Reg(DM9051_EPCR, 0xc); /* Issue phyxcer read command */

    while (DM9051_Read_Reg(DM9051_EPCR) & 0x1)
    {
        NeonRTOS_Sleep(1);
    }; //Wait complete

    DM9051_Write_Reg(DM9051_EPCR, 0x0); /* Clear phyxcer read command */
    value = (DM9051_Read_Reg(DM9051_EPDRH) << 8) | DM9051_Read_Reg(DM9051_EPDRL);

    return value;
}

static void DM9051_PHY_Mode_Set()
{
    uint16_t phy_reg4 = 0x01e1, phy_reg0 = 0x1200;

    DM9051_PHY_Write(20, 0x0200); /* Disable NWAY powersaver */

#ifdef DM9051_FLOWCONTROL_EN
    DM9051_PHY_Write(4, phy_reg4 | 0x0400); /* Set PHY media mode */
#else
    DM9051_PHY_Write(4, phy_reg4); /* Set PHY media mode */
#endif /* DM9051_FLOWCONTROL_EN */
    DM9051_PHY_Write(0, phy_reg0); /* RE_START NWAY */
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

static void DM9051_Chip_Reset(uint8_t mac[6])
{
    DM9051_Soft_Reset(mac);

    DM9051_Write_Reg(DM9051_IMR, DM9051_IMR_SET);
}

hwEthernet_OpResult Ethernet_Init(const uint8_t mac[6], onLinkUpCallback link_up_cb, onLinkDownCallback link_down_cb)
{
    int i, oft;
    uint32_t device_id;

    GPIO_Pin_Init(DM9051_RST_Pin, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    GPIO_Pin_Init(DM9051_CS_Pin, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);

    SPI_Master_Init(DM9051_SPI_INDEX, 10000000, hwSPI_OpMode_Polarity0_Phase0, false);

    DM9051_Hardware_Reset();

    NeonRTOS_Sleep(100);

    /* RESET device */
    DM9051_Write_Reg(DM9051_NCR, DM9051_NCR_REG_RESET);
    NeonRTOS_Sleep(2);
    DM9051_Write_Reg(DM9051_NCR, 0);

    DM9051_Write_Reg(DM9051_GPCR, GPCR_GEP_CNTL);
    DM9051_Write_Reg(DM9051_GPR, 0x00); //Power on PHY

    NeonRTOS_Sleep(100);

    device_id  = DM9051_Read_Reg(DM9051_VIDL);
    device_id |= DM9051_Read_Reg(DM9051_VIDH) << 8;
    device_id |= DM9051_Read_Reg(DM9051_PIDL) << 16;
    device_id |= DM9051_Read_Reg(DM9051_PIDH) << 24;
    UART_Printf("[%s L%d] device_id: %08X\n", __FUNCTION__, __LINE__, device_id);

    if(device_id != DM9051_ID)
    {
        return hwEthernet_HwError;
    }

    device_id  = DM9051_Read_Reg(DM9051_CHIPR);
    UART_Printf("[%s L%d] CHIP Revision: %02X\n", __FUNCTION__, __LINE__, device_id);

    /* Set PHY */
    DM9051_PHY_Mode_Set();

    /* set mac address */
    for (i = 0, oft = DM9051_PAR; i < 6; i++, oft++)
    {
        DM9051_Write_Reg(oft, mac[i]);
    }

    for (i = 0, oft = DM9051_MAR; i < 8; i++, oft++)
    {
        DM9051_Write_Reg(oft, 0x00);
    }
    UART_Printf("Clean Multicast Address Hash Table\n");

    /* Activate DM9051 */
    DM9051_Soft_Reset(mac);

    //DM9051_Write_Reg(DM9051_IMR, DM9051_IMR_SET); // Re-enable interrupt mask
    DM9051_Write_Reg(DM9051_IMR, DM9051_IMR_OFF); // Disable all interrupts
    
    onLinkUpCB = link_up_cb;
    onLinkDownCB = link_down_cb;
    
    ETH_Init = true;

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

    uint32_t retry = 0;
    uint8_t nsr_reg = 0;

    // wait for sending complete
    while (1)
    {
        nsr_reg = DM9051_Read_Reg(DM9051_NSR) & (NSR_TX1END | NSR_TX2END);
        if(nsr_reg != 0)
        {
            break;
        }

        retry++;
        if (retry > 10)
        {
            //UART_Printf("wait for send complete timeout, retry=%d, abort!\n", retry);
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
        DM9051_Write_Reg(DM9051_MPCR, 0x02); //reset tx point
    }

    //Write data to FIFO
    DM9051_Write_Reg(DM9051_TXPLL, out_len & 0xff);
    DM9051_Write_Reg(DM9051_TXPLH, (out_len >> 8) & 0xff);

    DM9051_Write_Mem(out_data, out_len);

    // start send cmd
    DM9051_Write_Reg(DM9051_TCR, TCR_TXREQ);

    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Get_Input_Frame_Length(uint32_t *frame_len)
{
    uint8_t isr_reg;
    uint8_t nsr_reg;

    isr_reg = DM9051_Read_Reg(DM9051_ISR);
    DM9051_Write_Reg(DM9051_ISR, (isr_reg & ISR_CLR_STATUS));  // Clear ISR status
    //UART_Printf("isr_reg=0x%x", isr_reg);

    // Receive Overflow Counter Overflow
    if (isr_reg & ISR_ROOS)
    {
        UART_Printf("dm9051_chip_reset Receive Overflow Counter Overflow\n");
        return hwEthernet_BufferError;
    }

    // Receive Overflow
    if (isr_reg & ISR_ROS)
    {
        UART_Printf("Receive_FIFO Overflow\n");
        DM9051_Write_Reg(DM9051_MPCR, 0x01); //reset rx point
    }

    // transmit
    if (isr_reg & ISR_PTS)
    {
        //UART_Printf("ISR_PTS\n");
    }

    uint16_t rx_status, rx_len;
    uint8_t ReceiveData[4];
    uint8_t rx_bytes[2];

    /* Check packet ready or not                                                                              */
    nsr_reg = DM9051_Read_Reg(DM9051_NSR) & NSR_RXRDY;
    if (nsr_reg)
    {
        rx_bytes[0] = DM9051_Read_Reg(DM9051_MRCMDX);
        rx_bytes[1] = DM9051_Read_Reg(DM9051_MRCMDX1);

        if (rx_bytes[1] != DM9051_PKT_RDY)
        {
            UART_Printf("NSR %02X, RCMDX %02X: rx error, dm9051_chip_rx_fifo_reset\n", nsr_reg, rx_bytes[1]);
            DM9051_Write_Reg(DM9051_MPCR, 0x01); //reset rx point
            DM9051_Write_Reg(DM9051_ISR, ISR_CLR_RX_STATUS);
            return hwEthernet_BufferError;
        }
    }
    else
    {
        DM9051_Write_Reg(DM9051_ISR, ISR_CLR_RX_STATUS);
        return hwEthernet_BufferError;
    }

    DM9051_Read_Mem(ReceiveData, 4);

    rx_status = ReceiveData[0] + (ReceiveData[1] << 8);
    rx_len = ReceiveData[2] + (ReceiveData[3] << 8);

    if ((rx_len < 14) || (rx_len > DM9051_PKT_MAX)) {
        UART_Printf("bad rx len=%u status=%04X, reset rx fifo\n", rx_len, rx_status);
        DM9051_Write_Reg(DM9051_MPCR, MPCR_RSTRX);
        DM9051_Write_Reg(DM9051_ISR, ISR_CLR_RX_STATUS);
        return hwEthernet_BufferError;
    }

    //UART_Printf("RX header status=%04X len=%u\n", rx_status, rx_len);

    *frame_len = rx_len;

    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Input(uint8_t *in_data, uint32_t in_len)
{
    if (in_data == NULL || in_len == 0U) {
        return hwEthernet_InvalidParameter;
    }

    DM9051_Read_Mem(in_data, in_len);

    //DM9051_Write_Reg(DM9051_IMR, DM9051_IMR_SET); // Re-enable interrupt mask
    /*
    UART_Printf("RX frame len=%lu %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
            in_len,
            in_data[0], in_data[1], in_data[2], in_data[3],
            in_data[4], in_data[5], in_data[6], in_data[7],
            in_data[8], in_data[9], in_data[10], in_data[11],
            in_data[12], in_data[13]);*/

    return hwEthernet_OK;
}

bool Ethernet_isInit(void)
{
    return ETH_Init;
}

void Ethernet_Set_Link(void)
{
    uint8_t nsr_reg;
    nsr_reg = DM9051_Read_Reg(DM9051_NSR);
    if (nsr_reg & NSR_LINKST)
    {
        uint8_t ncr_reg;

        ncr_reg = DM9051_Read_Reg(DM9051_NCR) & NCR_FDX;
        nsr_reg = DM9051_Read_Reg(DM9051_NSR) & (NSR_SPEED | NSR_LINKST);

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
  if (isLinkUp) {
  } else {
  }

}

uint32_t Ethernet_Get_Tick(void)
{
  return HAL_GetTick();
}

void Ethernet_Get_Hardware_Mac(uint8_t mac[6])
{
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

    /* calculate crc32 value of mac address */
    crc = DM9051_CRC32_LE(mac, 6);

    hash = crc & 0x3F;

    hash_group = hash / 8;

    if (hash > 31)
    {
        *eth_HashTableHigh |= 1 << (hash - 32);
        DM9051_Write_Reg(DM9051_MAR + hash_group, (*eth_HashTableHigh >> (hash_group - 4) * 8) & 0xff);
    }
    else
    {
        *eth_HashTableLow |= 1 << hash;
        DM9051_Write_Reg(DM9051_MAR + hash_group, (*eth_HashTableLow >> (hash_group * 8)) & 0xff);
    }
}

#endif //CONFIG_ETHERNET_ONBOARD