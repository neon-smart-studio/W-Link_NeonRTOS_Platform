
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "soc.h"

#include <string.h>

#include "NeonRTOS.h"

#include "Ethernet/Ethernet.h"

#include "Ethernet/Ethernet_Def.h"

#include "GPIO/GPIO.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSP432E

#ifdef CONFIG_ETHERNET_ONBOARD

#include "GPIO/Device/TIMSP432/GPIO_TIMSP432E.h"

#define EMAC_PHY_CONFIG  (EMAC_PHY_TYPE_INTERNAL | EMAC_PHY_INT_MDIX_EN | \
                          EMAC_PHY_AN_100B_T_FULL_DUPLEX)
#define EMAC_PHY_ADDR    0U

#define ETH_RX_DESC_NUM  20U
#define ETH_TX_DESC_NUM  20U

#ifndef ETH_MAX_PACKET_SIZE
#define ETH_MAX_PACKET_SIZE 1522U
#endif

#define ETH_DMA_BUF_SIZE ((ETH_MAX_PACKET_SIZE + 3U) & ~3U)

#define ETH_DMA_ALIGN __attribute__((aligned(4)))

typedef struct {
    tEMACDMADescriptor desc;
    uint8_t *buf;
    uint32_t len;
    bool used;
} Ethernet_DMA_Desc_t;

typedef struct {
    Ethernet_DMA_Desc_t *items;
    uint32_t count;
    uint32_t read;
    uint32_t write;
} Ethernet_DMA_Ring_t;

static Ethernet_DMA_Desc_t g_tx_desc[ETH_TX_DESC_NUM] ETH_DMA_ALIGN;
static Ethernet_DMA_Desc_t g_rx_desc[ETH_RX_DESC_NUM] ETH_DMA_ALIGN;

static uint8_t g_tx_buf[ETH_TX_DESC_NUM][ETH_DMA_BUF_SIZE] ETH_DMA_ALIGN;
static uint8_t g_rx_buf[ETH_RX_DESC_NUM][ETH_DMA_BUF_SIZE] ETH_DMA_ALIGN;

static Ethernet_DMA_Ring_t g_tx_ring = { g_tx_desc, ETH_TX_DESC_NUM, 0U, 0U };
static Ethernet_DMA_Ring_t g_rx_ring = { g_rx_desc, ETH_RX_DESC_NUM, 0U, 0U };

static bool ETH_Init = false;

static onLinkUpCallback onLinkUpCB = NULL;
static onLinkDownCallback onLinkDownCB = NULL;

static uint32_t g_eth_hash_high = 0U;
static uint32_t g_eth_hash_low  = 0U;

static void Ethernet_Release_Rx(Ethernet_DMA_Desc_t *d)
{
    d->len = 0U;
    d->used = false;
    d->desc.ui32Count = DES1_RX_CTRL_CHAINED |
                        (ETH_DMA_BUF_SIZE << DES1_RX_CTRL_BUFF1_SIZE_S);
    d->desc.pvBuffer1 = d->buf;
    d->desc.ui32CtrlStatus = DES0_RX_CTRL_OWN;
}

void Ethernet_IRQHandler(void)
{
    if(!ETH_Init) return;

    uint32_t status = MAP_EMACIntStatus(EMAC0_BASE, true);
    MAP_EMACIntClear(EMAC0_BASE, status);

    if(status & EMAC_INT_PHY) {
        Ethernet_Set_Link();
    }

    if(status & (EMAC_INT_TRANSMIT | EMAC_INT_TX_STOPPED)) {
        Ethernet_DMA_Desc_t *d;

        for(uint32_t i = 0U; i < g_tx_ring.count; i++) {
            d = &g_tx_ring.items[g_tx_ring.read];

            if(!d->used) {
                break;
            }

            if(d->desc.ui32CtrlStatus & DES0_TX_CTRL_OWN) {
                break;
            }

            d->used = false;
            d->len = 0U;
            d->desc.ui32Count = 0U;
            d->desc.pvBuffer1 = d->buf;
            d->desc.ui32CtrlStatus = DES0_TX_CTRL_CHAINED;

            g_tx_ring.read++;
            if(g_tx_ring.read >= g_tx_ring.count) {
                g_tx_ring.read = 0U;
            }
        }
    }

    if(status & EMAC_INT_RX_STOPPED) {
        MAP_EMACRxDMAPollDemand(EMAC0_BASE);
    }

    if(status & EMAC_INT_TX_STOPPED) {
        MAP_EMACTxDMAPollDemand(EMAC0_BASE);
    }
}

hwEthernet_OpResult Ethernet_Init(const uint8_t mac[6], onLinkUpCallback link_up_cb, onLinkDownCallback link_down_cb)
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    MAP_GPIOPinConfigure(GPIO_PF0_EN0LED0);
    MAP_GPIOPinConfigure(GPIO_PF4_EN0LED1);

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EMAC0);
    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_EMAC0);

    if((EMAC_PHY_CONFIG & EMAC_PHY_TYPE_MASK) == EMAC_PHY_TYPE_INTERNAL) {
        if(!MAP_SysCtlPeripheralPresent(SYSCTL_PERIPH_EPHY0)) {
            return hwEthernet_HwError;
        }

        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EPHY0);
        MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_EPHY0);
    }

    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_EMAC0)) {
    }

    MAP_EMACPHYConfigSet(EMAC0_BASE, EMAC_PHY_CONFIG);

    MAP_EMACInit(EMAC0_BASE,
                 g_sys_clock_hz,
                 EMAC_BCONFIG_MIXED_BURST | EMAC_BCONFIG_PRIORITY_FIXED,
                 4,
                 4,
                 0);

    MAP_EMACConfigSet(EMAC0_BASE,
                      (EMAC_CONFIG_FULL_DUPLEX |
                       EMAC_CONFIG_CHECKSUM_OFFLOAD |
                       EMAC_CONFIG_7BYTE_PREAMBLE |
                       EMAC_CONFIG_IF_GAP_96BITS |
                       EMAC_CONFIG_USE_MACADDR0 |
                       EMAC_CONFIG_SA_FROM_DESCRIPTOR |
                       EMAC_CONFIG_BO_LIMIT_1024),
                      (EMAC_MODE_RX_STORE_FORWARD |
                       EMAC_MODE_TX_STORE_FORWARD |
                       EMAC_MODE_TX_THRESHOLD_64_BYTES |
                       EMAC_MODE_RX_THRESHOLD_64_BYTES),
                      ETH_MAX_PACKET_SIZE);

    MAP_EMACAddrSet(EMAC0_BASE, 0U, mac);
    
    for(uint32_t i = 0U; i < ETH_TX_DESC_NUM; i++) {
        g_tx_desc[i].buf = g_tx_buf[i];
        g_tx_desc[i].len = 0U;
        g_tx_desc[i].used = false;
        g_tx_desc[i].desc.ui32Count = 0U;
        g_tx_desc[i].desc.pvBuffer1 = g_tx_desc[i].buf;
        g_tx_desc[i].desc.DES3.pLink = (i == (ETH_TX_DESC_NUM - 1U)) ?
                                      &g_tx_desc[0].desc : &g_tx_desc[i + 1U].desc;
        g_tx_desc[i].desc.ui32CtrlStatus = DES0_TX_CTRL_CHAINED;
    }

    g_tx_ring.read = 0U;
    g_tx_ring.write = 0U;

    for(uint32_t i = 0U; i < ETH_RX_DESC_NUM; i++) {
        g_rx_desc[i].buf = g_rx_buf[i];
        g_rx_desc[i].desc.DES3.pLink = (i == (ETH_RX_DESC_NUM - 1U)) ?
                                      &g_rx_desc[0].desc : &g_rx_desc[i + 1U].desc;
        Ethernet_Release_Rx(&g_rx_desc[i]);
    }

    g_rx_ring.read = 0U;
    g_rx_ring.write = 0U;

    MAP_EMACRxDMADescriptorListSet(EMAC0_BASE, &g_rx_desc[0].desc);
    MAP_EMACTxDMADescriptorListSet(EMAC0_BASE, &g_tx_desc[0].desc);

    (void)MAP_EMACPHYRead(EMAC0_BASE, EMAC_PHY_ADDR, EPHY_MISR1);
    (void)MAP_EMACPHYRead(EMAC0_BASE, EMAC_PHY_ADDR, EPHY_MISR2);

    uint16_t scr = MAP_EMACPHYRead(EMAC0_BASE, EMAC_PHY_ADDR, EPHY_SCR);
    scr |= (EPHY_SCR_INTEN_EXT | EPHY_SCR_INTOE_EXT);
    MAP_EMACPHYWrite(EMAC0_BASE, EMAC_PHY_ADDR, EPHY_SCR, scr);

    MAP_EMACPHYWrite(EMAC0_BASE,
                     EMAC_PHY_ADDR,
                     EPHY_MISR1,
                     EPHY_MISR1_LINKSTATEN |
                     EPHY_MISR1_SPEEDEN |
                     EPHY_MISR1_DUPLEXMEN |
                     EPHY_MISR1_ANCEN);

    MAP_EMACFrameFilterSet(EMAC0_BASE,
                           EMAC_FRMFILTER_HASH_AND_PERFECT |
                           EMAC_FRMFILTER_PASS_MULTICAST |
                           EMAC_FRMFILTER_PASS_NO_CTRL);

    MAP_EMACHashFilterSet(EMAC0_BASE, g_eth_hash_high, g_eth_hash_low);

    MAP_EMACIntClear(EMAC0_BASE, MAP_EMACIntStatus(EMAC0_BASE, false));

    MAP_EMACTxEnable(EMAC0_BASE);
    MAP_EMACRxEnable(EMAC0_BASE);

    MAP_EMACIntEnable(EMAC0_BASE,
                      EMAC_INT_RECEIVE |
                      EMAC_INT_TRANSMIT |
                      EMAC_INT_TX_STOPPED |
                      EMAC_INT_RX_NO_BUFFER |
                      EMAC_INT_RX_STOPPED |
                      EMAC_INT_PHY);

    MAP_EMACPHYWrite(EMAC0_BASE,
                     EMAC_PHY_ADDR,
                     EPHY_BMCR,
                     EPHY_BMCR_ANEN | EPHY_BMCR_RESTARTAN);

    onLinkUpCB = link_up_cb;
    onLinkDownCB = link_down_cb;
    
    gpio_pin_init_status[hwGPIO_Pin_F0] = true;
    gpio_pin_init_status[hwGPIO_Pin_F4] = true;

    ETH_Init = true;

    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Output(const uint8_t *out_data, uint16_t out_len)
{
    if ((out_data == NULL) || (out_len == 0U)) {
        return hwEthernet_InvalidParameter;
    }

    if (out_len > ETH_MAX_PACKET_SIZE) {
        return hwEthernet_BufferError;
    }

    Ethernet_DMA_Desc_t *d;

    for(uint32_t i = 0U; i < g_tx_ring.count; i++) {
        d = &g_tx_ring.items[g_tx_ring.read];

        if(!d->used) {
            break;
        }

        if(d->desc.ui32CtrlStatus & DES0_TX_CTRL_OWN) {
            break;
        }

        d->used = false;
        d->len = 0U;
        d->desc.ui32Count = 0U;
        d->desc.pvBuffer1 = d->buf;
        d->desc.ui32CtrlStatus = DES0_TX_CTRL_CHAINED;

        g_tx_ring.read++;
        if(g_tx_ring.read >= g_tx_ring.count) {
            g_tx_ring.read = 0U;
        }
    }

    d = &g_tx_ring.items[g_tx_ring.write];
    if(d->used || (d->desc.ui32CtrlStatus & DES0_TX_CTRL_OWN)) {
        return hwEthernet_Busy;
    }

    memcpy(d->buf, out_data, out_len);
    d->len = out_len;
    d->used = true;

    d->desc.ui32Count = out_len;
    d->desc.pvBuffer1 = d->buf;
    d->desc.ui32CtrlStatus = DES0_TX_CTRL_FIRST_SEG |
                             DES0_TX_CTRL_LAST_SEG |
                             DES0_TX_CTRL_INTERRUPT |
                             DES0_TX_CTRL_CHAINED |
                             DES0_TX_CTRL_IP_ALL_CKHSUMS |
                             DES0_TX_CTRL_OWN;

    g_tx_ring.write++;
    if(g_tx_ring.write >= g_tx_ring.count) {
        g_tx_ring.write = 0U;
    }

    MAP_EMACTxDMAPollDemand(EMAC0_BASE);

    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Get_Input_Frame_Length(uint32_t *frame_len)
{
    if(frame_len == NULL) {
        return hwEthernet_InvalidParameter;
    }

    *frame_len = 0U;

    if(!ETH_Init) {
        return hwEthernet_NotInit;
    }

    Ethernet_DMA_Desc_t *d = &g_rx_ring.items[g_rx_ring.read];

    /* DMA 還沒交給 CPU */
    if(d->desc.ui32CtrlStatus & DES0_RX_CTRL_OWN) {
        return hwEthernet_OK;
    }

    /* Frame Error */
    if(d->desc.ui32CtrlStatus & DES0_RX_STAT_ERR) {
        Ethernet_Release_Rx(d);

        g_rx_ring.read++;
        if(g_rx_ring.read >= g_rx_ring.count) {
            g_rx_ring.read = 0U;
        }

        MAP_EMACRxDMAPollDemand(EMAC0_BASE);

        return hwEthernet_BufferError;
    }

    /* 目前只支援一個 Descriptor 一個 Frame */
    if((d->desc.ui32CtrlStatus & DES0_RX_STAT_LAST_DESC) == 0U) {
        return hwEthernet_BufferError;
    }

    uint32_t rx_len =
        (d->desc.ui32CtrlStatus & DES0_RX_STAT_FRAME_LENGTH_M) >>
        DES0_RX_STAT_FRAME_LENGTH_S;

    if(rx_len > ETH_MAX_PACKET_SIZE) {
        Ethernet_Release_Rx(d);

        g_rx_ring.read++;
        if(g_rx_ring.read >= g_rx_ring.count) {
            g_rx_ring.read = 0U;
        }

        MAP_EMACRxDMAPollDemand(EMAC0_BASE);

        return hwEthernet_BufferError;
    }

    d->len = rx_len;
    d->used = true;

    *frame_len = rx_len;

    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Input(uint8_t *in_data, uint32_t in_len)
{
    if (in_data == NULL || in_len == 0U) {
        return hwEthernet_InvalidParameter;
    }

    if((in_data == NULL) || (in_len == 0U)) {
        return hwEthernet_InvalidParameter;
    }

    if(!ETH_Init) {
        return hwEthernet_NotInit;
    }

    Ethernet_DMA_Desc_t *d = &g_rx_ring.items[g_rx_ring.read];

    if(d->desc.ui32CtrlStatus & DES0_RX_CTRL_OWN)
    {
        return hwEthernet_Busy;
    }

    if(!d->used || (d->len == 0U))
    {
        uint32_t status = d->desc.ui32CtrlStatus;

        if(status & DES0_RX_STAT_ERR)
        {
            Ethernet_Release_Rx(d);
            g_rx_ring.read++;
            if(g_rx_ring.read >= g_rx_ring.count) {
                g_rx_ring.read = 0U;
            }
            MAP_EMACRxDMAPollDemand(EMAC0_BASE);
            return hwEthernet_BufferError;
        }

        if((status & DES0_RX_STAT_LAST_DESC) == 0U)
        {
            return hwEthernet_BufferError;
        }

        uint32_t rx_len =
            (status & DES0_RX_STAT_FRAME_LENGTH_M) >>
            DES0_RX_STAT_FRAME_LENGTH_S;

        if(rx_len == 0U || rx_len > ETH_MAX_PACKET_SIZE)
        {
            Ethernet_Release_Rx(d);
            g_rx_ring.read++;
            if(g_rx_ring.read >= g_rx_ring.count) {
                g_rx_ring.read = 0U;
            }
            MAP_EMACRxDMAPollDemand(EMAC0_BASE);
            return hwEthernet_BufferError;
        }

        d->len = rx_len;
        d->used = true;
    }

    if(in_len < d->len) {
        return hwEthernet_BufferError;
    }

    memcpy(in_data, d->buf, d->len);

    Ethernet_Release_Rx(d);

    g_rx_ring.read++;
    if(g_rx_ring.read >= g_rx_ring.count) {
        g_rx_ring.read = 0U;
    }

    MAP_EMACRxDMAPollDemand(EMAC0_BASE);

    return hwEthernet_OK;
}

bool Ethernet_isInit(void)
{
    return ETH_Init;
}

void Ethernet_Set_Link(void)
{
    if(!ETH_Init) {
        return;
    }

    (void)MAP_EMACPHYRead(EMAC0_BASE, EMAC_PHY_ADDR, EPHY_MISR1);

    (void)MAP_EMACPHYRead(EMAC0_BASE, EMAC_PHY_ADDR, EPHY_BMSR);
    uint16_t bmsr = MAP_EMACPHYRead(EMAC0_BASE, EMAC_PHY_ADDR, EPHY_BMSR);
    bool link_now = ((bmsr & EPHY_BMSR_LINKSTAT) != 0U);

    if(link_now)
    {
        if(onLinkUpCB != NULL)
        {
            onLinkUpCB();
        }
    }
    else
    {
        if(onLinkDownCB != NULL)
        {
            onLinkDownCB();
        }
    }
}

void Ethernet_Update_Config(bool isLinkUp)
{
    if(!ETH_Init) {
        return;
    }

    if(!isLinkUp) {
        MAP_EMACTxDisable(EMAC0_BASE);
        MAP_EMACRxDisable(EMAC0_BASE);
        return;
    }

    uint16_t sts = MAP_EMACPHYRead(EMAC0_BASE, EMAC_PHY_ADDR, EPHY_STS);
    uint32_t cfg, mode, rx_max_frame_size;

    MAP_EMACConfigGet(EMAC0_BASE, &cfg, &mode, &rx_max_frame_size);

    if(sts & EPHY_STS_SPEED) {
        cfg &= ~EMAC_CONFIG_100MBPS;      /* 10 Mbps */
    } else {
        cfg |= EMAC_CONFIG_100MBPS;       /* 100 Mbps */
    }

    if(sts & EPHY_STS_DUPLEX) {
        cfg |= EMAC_CONFIG_FULL_DUPLEX;
    } else {
        cfg &= ~EMAC_CONFIG_FULL_DUPLEX;
    }

    MAP_EMACConfigSet(EMAC0_BASE, cfg, mode, rx_max_frame_size);

    MAP_EMACTxEnable(EMAC0_BASE);
    MAP_EMACRxEnable(EMAC0_BASE);
}

uint32_t Ethernet_Get_Tick(void)
{
    return NeonRTOS_Millis();
}

void Ethernet_Get_Hardware_Mac(uint8_t mac[6])
{
    if(mac == NULL) {
        return;
    }

    uint32_t user0, user1;
    MAP_FlashUserGet(&user0, &user1);

    if((user0 == 0xFFFFFFFFU) || (user1 == 0xFFFFFFFFU)) {
        memset(mac, 0, 6U);
        return;
    }

    mac[0] = (uint8_t)( user0        & 0xFFU);
    mac[1] = (uint8_t)((user0 >>  8) & 0xFFU);
    mac[2] = (uint8_t)((user0 >> 16) & 0xFFU);
    mac[3] = (uint8_t)( user1        & 0xFFU);
    mac[4] = (uint8_t)((user1 >>  8) & 0xFFU);
    mac[5] = (uint8_t)((user1 >> 16) & 0xFFU);
}

static uint32_t Ethernet_Crc32Le(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFU;

    for(uint32_t i = 0U; i < len; i++) {
        uint8_t v = data[i];
        for(uint32_t b = 0U; b < 8U; b++) {
            uint32_t mix = (crc ^ v) & 1U;
            crc >>= 1U;
            if(mix != 0U) {
                crc ^= 0xEDB88320U;
            }
            v >>= 1U;
        }
    }

    return crc;
}

hwEthernet_OpResult Ethernet_Register_Multicast_Address(const uint8_t *mac,
                                                        uint32_t *eth_HashTableHigh,
                                                        uint32_t *eth_HashTableLow)
{
    if((mac == NULL) || (eth_HashTableHigh == NULL) || (eth_HashTableLow == NULL)) {
        return hwEthernet_InvalidParameter;
    }

    if((mac[0] & 0x01U) == 0U) {
        return hwEthernet_InvalidParameter;
    }

    uint32_t crc = Ethernet_Crc32Le(mac, 6U);
    uint32_t hash_index = (crc >> 26) & 0x3FU;

    if(hash_index < 32U) {
        *eth_HashTableLow |= (1UL << hash_index);
    } else {
        *eth_HashTableHigh |= (1UL << (hash_index - 32U));
    }

    g_eth_hash_low = *eth_HashTableLow;
    g_eth_hash_high = *eth_HashTableHigh;

    if(ETH_Init) {
        MAP_EMACHashFilterSet(EMAC0_BASE, g_eth_hash_high, g_eth_hash_low);
    }

    return hwEthernet_OK;
}

#endif //CONFIG_ETHERNET_ONBOARD

#endif //DEVICE_TIMSP432E