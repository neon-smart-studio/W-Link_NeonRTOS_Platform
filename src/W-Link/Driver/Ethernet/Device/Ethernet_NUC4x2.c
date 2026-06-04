#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "Ethernet/Ethernet.h"
#include "Ethernet/Ethernet_Def.h"

#include "GPIO/GPIO.h"

#if defined(NUC472) || defined(NUC442)

/* ================= User Config ================= */

#ifndef NUC472_PHY_ADDRESS
#define NUC472_PHY_ADDRESS              0x01U
#endif

#ifndef ETH_RX_BUF_SIZE
#define ETH_RX_BUF_SIZE                 ETH_MAX_PACKET_SIZE
#endif

#ifndef ETH_TX_BUF_SIZE
#define ETH_TX_BUF_SIZE                 ETH_MAX_PACKET_SIZE
#endif

#ifndef NUC472_ETH_RX_CACHE_BUF_SIZE
#define NUC472_ETH_RX_CACHE_BUF_SIZE    ETH_MAX_PACKET_SIZE
#endif

#ifndef NUC472_MDIO_TIMEOUT
#define NUC472_MDIO_TIMEOUT             1000000U
#endif

/* ================= PHY Register ================= */

#define PHY_BCR                         0x00U
#define PHY_BSR                         0x01U
#define PHY_ID1                         0x02U
#define PHY_ID2                         0x03U
#define PHY_ANAR                        0x04U
#define PHY_ANLPAR                      0x05U

#define PHY_RESET                       0x8000U
#define PHY_LOOPBACK                    0x4000U
#define PHY_FULLDUPLEX_100M             0x2100U
#define PHY_HALFDUPLEX_100M             0x2000U
#define PHY_FULLDUPLEX_10M              0x0100U
#define PHY_HALFDUPLEX_10M              0x0000U
#define PHY_AUTONEGOTIATION             0x1000U
#define PHY_RESTART_AUTONEGOTIATION     0x0200U
#define PHY_POWERDOWN                   0x0800U
#define PHY_ISOLATE                     0x0400U

#define PHY_AUTONEGO_COMPLETE           0x0020U
#define PHY_LINKED_STATUS               0x0004U

/* ================= Static State ================= */

static bool ethernet_is_init = false;
static bool ethernet_link_up = false;

static onLinkUpCallback onLinkUpCB = NULL;
static onLinkDownCallback onLinkDownCB = NULL;

static uint8_t rx_cache_buf[NUC472_ETH_RX_CACHE_BUF_SIZE];
static uint32_t rx_cache_len = 0;
static bool rx_cache_valid = false;

void Ethernet_Board_PinMux_Init(void)
{
    uint32_t temp;

    /*
     * NuMaker-PFM-NUC472 / Nu-mbed NUC472
     * RMII mode
     */
    EMAC->CTL |= EMAC_CTL_RMIIEN_Msk | EMAC_CTL_RMIIRXCTL_Msk;

    /* PB.14 = EMAC_MII_MDC
     * PB.15 = EMAC_MII_MDIO
     */
    temp = SYS->GPB_MFPH;
    temp &= ~(SYS_GPB_MFPH_PB14MFP_Msk |
              SYS_GPB_MFPH_PB15MFP_Msk);
    temp |=  (SYS_GPB_MFPH_PB14MFP_EMAC_MII_MDC |
              SYS_GPB_MFPH_PB15MFP_EMAC_MII_MDIO);
    SYS->GPB_MFPH = temp;

    /* PC.0 = EMAC_REFCLK
     * PC.1 = EMAC_MII_RXERR
     * PC.2 = EMAC_MII_RXDV / CRS_DV
     * PC.3 = EMAC_MII_RXD1
     * PC.4 = EMAC_MII_RXD0
     * PC.6 = EMAC_MII_TXD0
     * PC.7 = EMAC_MII_TXD1
     */
    temp = SYS->GPC_MFPL;
    temp &= ~(SYS_GPC_MFPL_PC0MFP_Msk |
              SYS_GPC_MFPL_PC1MFP_Msk |
              SYS_GPC_MFPL_PC2MFP_Msk |
              SYS_GPC_MFPL_PC3MFP_Msk |
              SYS_GPC_MFPL_PC4MFP_Msk |
              SYS_GPC_MFPL_PC6MFP_Msk |
              SYS_GPC_MFPL_PC7MFP_Msk);
    temp |=  (SYS_GPC_MFPL_PC0MFP_EMAC_REFCLK |
              SYS_GPC_MFPL_PC1MFP_EMAC_MII_RXERR |
              SYS_GPC_MFPL_PC2MFP_EMAC_MII_RXDV |
              SYS_GPC_MFPL_PC3MFP_EMAC_MII_RXD1 |
              SYS_GPC_MFPL_PC4MFP_EMAC_MII_RXD0 |
              SYS_GPC_MFPL_PC6MFP_EMAC_MII_TXD0 |
              SYS_GPC_MFPL_PC7MFP_EMAC_MII_TXD1);
    SYS->GPC_MFPL = temp;

    /* PC.8 = EMAC_MII_TXEN */
    temp = SYS->GPC_MFPH;
    temp &= ~SYS_GPC_MFPH_PC8MFP_Msk;
    temp |=  SYS_GPC_MFPH_PC8MFP_EMAC_MII_TXEN;
    SYS->GPC_MFPH = temp;

    /* High slew rate for RMII output pins:
     * PC.6 TXD0
     * PC.7 TXD1
     * PC.8 TXEN
     */
    PC->SLEWCTL |= GPIO_SLEWCTL_HSREN6_Msk |
                   GPIO_SLEWCTL_HSREN7_Msk |
                   GPIO_SLEWCTL_HSREN8_Msk;
}

static int32_t NUC472_EMAC_Open(uint8_t mac[6])
{
    EMAC_Open(mac);
    return 0;
}

static void NUC472_EMAC_Close(void)
{
    EMAC_Close();
}

static int32_t NUC472_EMAC_SendPacket(uint8_t *buf, uint32_t len)
{
    return EMAC_SendPkt(buf, len);
}

static int32_t NUC472_EMAC_RecvPacket(uint8_t *buf, uint32_t *len)
{
    return EMAC_RecvPkt(buf, len);
}

static uint32_t NUC472_EMAC_MDIO_Read(uint32_t phy, uint32_t reg)
{
    uint32_t timeout;

    EMAC->MIIMDAT = 0U;

    EMAC->MIIMCTL =
        ((phy & 0x1FU) << EMAC_MIIMCTL_PHYADDR_Pos) |
        ((reg & 0x1FU) << EMAC_MIIMCTL_PHYREG_Pos) |
        EMAC_MIIMCTL_BUSY_Msk |
        EMAC_MIIMCTL_MDCON_Msk;

    timeout = NUC472_MDIO_TIMEOUT;

    while ((EMAC->MIIMCTL & EMAC_MIIMCTL_BUSY_Msk) != 0U) {
        if (--timeout == 0U) {
            return 0xFFFFU;
        }
    }

    return EMAC->MIIMDAT & 0xFFFFU;
}

static void NUC472_EMAC_MDIO_Write(uint32_t phy, uint32_t reg, uint32_t value)
{
    uint32_t timeout;

    EMAC->MIIMDAT = value & 0xFFFFU;

    EMAC->MIIMCTL =
        ((phy & 0x1FU) << EMAC_MIIMCTL_PHYADDR_Pos) |
        ((reg & 0x1FU) << EMAC_MIIMCTL_PHYREG_Pos) |
        EMAC_MIIMCTL_BUSY_Msk |
        EMAC_MIIMCTL_WRITE_Msk |
        EMAC_MIIMCTL_MDCON_Msk;

    timeout = NUC472_MDIO_TIMEOUT;

    while ((EMAC->MIIMCTL & EMAC_MIIMCTL_BUSY_Msk) != 0U) {
        if (--timeout == 0U) {
            return;
        }
    }
}

/* ================= Internal Helper ================= */

static bool Ethernet_Read_Link_Status(void)
{
    uint32_t bsr1;
    uint32_t bsr2;

    /*
     * PHY_BSR link bit 有些 PHY 需要讀兩次才準。
     */
    bsr1 = NUC472_EMAC_MDIO_Read(NUC472_PHY_ADDRESS, PHY_BSR);
    bsr2 = NUC472_EMAC_MDIO_Read(NUC472_PHY_ADDRESS, PHY_BSR);

    (void)bsr1;

    return ((bsr2 & PHY_LINKED_STATUS) != 0U);
}

static void Ethernet_Check_Link_Change(void)
{
    bool now_link_up;

    if (!ethernet_is_init) {
        return;
    }

    now_link_up = Ethernet_Read_Link_Status();

    if (now_link_up != ethernet_link_up) {
        ethernet_link_up = now_link_up;

        if (ethernet_link_up) {
            if (onLinkUpCB != NULL) {
                onLinkUpCB();
            }
        } else {
            if (onLinkDownCB != NULL) {
                onLinkDownCB();
            }
        }
    }
}

static hwEthernet_OpResult Ethernet_Try_Fetch_Rx_Frame(void)
{
    uint32_t len = NUC472_ETH_RX_CACHE_BUF_SIZE;
    int32_t ret;

    if (rx_cache_valid) {
        return hwEthernet_OK;
    }

    ret = NUC472_EMAC_RecvPacket(rx_cache_buf, &len);

    if (ret != 0) {
        rx_cache_len = 0;
        rx_cache_valid = false;
        return hwEthernet_Busy;
    }

    if ((len == 0U) || (len > ETH_MAX_PACKET_SIZE)) {
        rx_cache_len = 0;
        rx_cache_valid = false;
        return hwEthernet_Busy;
    }

    rx_cache_len = len;
    rx_cache_valid = true;

    return hwEthernet_OK;
}

/* ================= Public API ================= */

hwEthernet_OpResult Ethernet_Init(const uint8_t mac[6], onLinkUpCallback link_up_cb, onLinkDownCallback link_down_cb, onInterruptCallback interrupt_cb)
{
    uint8_t mac_buf[6];
    uint32_t timeout;

    if (mac == NULL) {
        return hwEthernet_InvalidParameter;
    }

    memcpy(mac_buf, mac, sizeof(mac_buf));

    onLinkUpCB = link_up_cb;
    onLinkDownCB = link_down_cb;

    rx_cache_len = 0;
    rx_cache_valid = false;
    ethernet_link_up = false;
    ethernet_is_init = false;

    SYS_UnlockReg();

    CLK_EnableModuleClock(EMAC_MODULE);

    Ethernet_Board_PinMux_Init();

    SYS_LockReg();

    if (NUC472_EMAC_Open(mac_buf) != 0) {
        return hwEthernet_HwError;
    }

    /*
     * PHY reset + auto negotiation。
     */
    NUC472_EMAC_MDIO_Write(
        NUC472_PHY_ADDRESS,
        PHY_BCR,
        PHY_RESET
    );

    timeout = 1000000U;
    while ((NUC472_EMAC_MDIO_Read(NUC472_PHY_ADDRESS, PHY_BCR) & PHY_RESET) != 0U) {
        if (--timeout == 0U) {
            NUC472_EMAC_Close();
            return hwEthernet_HwError;
        }
    }

    NUC472_EMAC_MDIO_Write(
        NUC472_PHY_ADDRESS,
        PHY_BCR,
        PHY_AUTONEGOTIATION | PHY_RESTART_AUTONEGOTIATION
    );

#if defined(EMAC_IRQn)
    NVIC_EnableIRQ(EMAC_IRQn);
#endif

    ethernet_is_init = true;
    ethernet_link_up = Ethernet_Read_Link_Status();

    if (ethernet_link_up) {
        if (onLinkUpCB != NULL) {
            onLinkUpCB();
        }
    }

    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Output(const uint8_t *out_data, uint16_t out_len)
{
    int32_t ret;

    if ((out_data == NULL) || (out_len == 0U)) {
        return hwEthernet_InvalidParameter;
    }

    if (out_len > ETH_MAX_PACKET_SIZE) {
        return hwEthernet_BufferError;
    }

    if (!ethernet_is_init) {
        return hwEthernet_NotInit;
    }

    ret = NUC472_EMAC_SendPacket((uint8_t *)out_data, out_len);

    if (ret != 0) {
        return hwEthernet_HwError;
    }

    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Get_Input_Frame_Length(uint32_t *frame_len)
{
    hwEthernet_OpResult ret;

    if (frame_len == NULL) {
        return hwEthernet_InvalidParameter;
    }

    if (!ethernet_is_init) {
        return hwEthernet_NotInit;
    }

    ret = Ethernet_Try_Fetch_Rx_Frame();

    if (ret != hwEthernet_OK) {
        *frame_len = 0;
        return ret;
    }

    *frame_len = rx_cache_len;

    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Input(uint8_t *in_data, uint32_t in_len)
{
    hwEthernet_OpResult ret;

    if ((in_data == NULL) || (in_len == 0U)) {
        return hwEthernet_InvalidParameter;
    }

    if (!ethernet_is_init) {
        return hwEthernet_NotInit;
    }

    ret = Ethernet_Try_Fetch_Rx_Frame();

    if (ret != hwEthernet_OK) {
        return ret;
    }

    if (in_len < rx_cache_len) {
        rx_cache_len = 0;
        rx_cache_valid = false;
        return hwEthernet_BufferError;
    }

    memcpy(in_data, rx_cache_buf, rx_cache_len);

    rx_cache_len = 0;
    rx_cache_valid = false;

    return hwEthernet_OK;
}

bool Ethernet_isInit(void)
{
    return ethernet_is_init;
}

void Ethernet_Set_Link(void)
{
    Ethernet_Check_Link_Change();
}

void Ethernet_Update_Config(bool isLinkUp)
{
    if (!ethernet_is_init) {
        return;
    }

    ethernet_link_up = isLinkUp;

    if (isLinkUp) {
        NUC472_EMAC_MDIO_Write(
            NUC472_PHY_ADDRESS,
            PHY_BCR,
            PHY_AUTONEGOTIATION | PHY_RESTART_AUTONEGOTIATION
        );
    } else {
        /*
         * 這裡不直接 EMAC_Close，避免 lwIP 還持有 netif。
         * 只記錄 link down 狀態。
         */
    }
}

uint32_t Ethernet_Get_Tick(void)
{
#if defined(NeonRTOS_GetTick)
    return NeonRTOS_GetTick();
#elif defined(NeonRTOS_Millis)
    return NeonRTOS_Millis();
#else
    return 0;
#endif
}

void Ethernet_Get_Hardware_Mac(uint8_t mac[6])
{
    uint32_t uid0 = 0;
    uint32_t uid1 = 0;

    if (mac == NULL) {
        return;
    }

#if defined(FMC_ReadUID)
    SYS_UnlockReg();
    FMC_Open();
    uid0 = FMC_ReadUID(0);
    uid1 = FMC_ReadUID(1);
    FMC_Close();
    SYS_LockReg();
#else
    uid0 = 0x472442U;
    uid1 = 0x000001U;
#endif

    /*
     * 02:xx:xx 是 locally administered unicast MAC。
     */
    mac[0] = 0x02;
    mac[1] = 0x47;
    mac[2] = 0x20;
    mac[3] = (uint8_t)((uid0 >> 8) & 0xFFU);
    mac[4] = (uint8_t)(uid0 & 0xFFU);
    mac[5] = (uint8_t)(uid1 & 0xFFU);
}

/* ================= IRQ ================= */

void EMAC_IRQHandler(void)
{
#if defined(EMAC_INTSTS_RXIF_Msk)
    uint32_t intsts = EMAC->INTSTS;

    EMAC->INTSTS = intsts;
#else
    /*
     * 若你的 BSP 有 EMAC_IRQHandler 內建處理，
     * 這裡可改成呼叫 BSP 的 handler。
     */
#endif
}

#endif