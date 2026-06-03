/**
  ******************************************************************************
  * @file    ethernetif.c
  * @author  MCD Application Team & Wi6Labs
  * @version V1.5.0
  * @date    20-june-2017
  * @brief   This file implements Ethernet network interface drivers for lwIP
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/*
 * Derived from STMicroelectronics ethernetif.c
 *
 * Copyright (c) STMicroelectronics
 *
 * Modified by Neon Smart Studio
 * - Removed lwIP dependency
 * - Reworked TX/RX API
 * - Added NeonRTOS integration
 * - Added W-Link Ethernet abstraction
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "soc.h"

#include <string.h>

#include "NeonRTOS.h"

#include "Ethernet/Ethernet.h"

#include "Ethernet/Ethernet_Def.h"

#include "GPIO/GPIO.h"

#ifdef STM32H7

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

/* Section 3: Common PHY Registers */
#define PHY_BCR                         ((uint16_t)0x00U)    /*!< Transceiver Basic Control Register   */
#define PHY_BSR                         ((uint16_t)0x01U)    /*!< Transceiver Basic Status Register    */

#define PHY_RESET                       ((uint16_t)0x8000U)  /*!< PHY Reset */
#define PHY_LOOPBACK                    ((uint16_t)0x4000U)  /*!< Select loop-back mode */
#define PHY_FULLDUPLEX_100M             ((uint16_t)0x2100U)  /*!< Set the full-duplex mode at 100 Mb/s */
#define PHY_HALFDUPLEX_100M             ((uint16_t)0x2000U)  /*!< Set the half-duplex mode at 100 Mb/s */
#define PHY_FULLDUPLEX_10M              ((uint16_t)0x0100U)  /*!< Set the full-duplex mode at 10 Mb/s  */
#define PHY_HALFDUPLEX_10M              ((uint16_t)0x0000U)  /*!< Set the half-duplex mode at 10 Mb/s  */
#define PHY_AUTONEGOTIATION             ((uint16_t)0x1000U)  /*!< Enable auto-negotiation function     */
#define PHY_RESTART_AUTONEGOTIATION     ((uint16_t)0x0200U)  /*!< Restart auto-negotiation function    */
#define PHY_POWERDOWN                   ((uint16_t)0x0800U)  /*!< Select the power down mode           */
#define PHY_ISOLATE                     ((uint16_t)0x0400U)  /*!< Isolate PHY from MII                 */

#define PHY_AUTONEGO_COMPLETE           ((uint16_t)0x0020U)  /*!< Auto-Negotiation process completed   */
#define PHY_LINKED_STATUS               ((uint16_t)0x0004U)  /*!< Valid link established               */
#define PHY_JABBER_DETECTION            ((uint16_t)0x0002U)  /*!< Jabber condition detected            */

/* Section 4: Extended PHY Registers */

#define PHY_SR                          ((uint16_t)0x1FU)    /*!< PHY special control/ status register Offset     */

#define PHY_SPEED_STATUS                ((uint16_t)0x0004U)  /*!< PHY Speed mask                                  */
#define PHY_DUPLEX_STATUS               ((uint16_t)0x0010U)  /*!< PHY Duplex mask                                 */

#define PHY_ISFR                        ((uint16_t)0x01DU)   /*!< PHY Interrupt Source Flag register Offset       */
#define PHY_IMR                         ((uint16_t)0x001E)   /*!< PHY Interrupt Mask register Offset              */
#define PHY_ISFR_INT4                   ((uint16_t)0x0010U)  /*!< PHY Link down inturrupt                         */

/* Definition of PHY SPECIAL CONTROL/STATUS REGISTER bitfield Auto-negotiation done indication */
/* Placed in STM32Ethernet library instead of HAL conf to avoid compatibility dependence with Arduino_Core_STM32 */
/* Could be moved from this file once Generic PHY is implemented */
#define PHY_SR_AUTODONE ((uint16_t)0x1000)

__ALIGN_BEGIN ETH_DMADescTypeDef  DMARxDscrTab[ETH_RXBUFNB] __ALIGN_END;/* Ethernet Rx MA Descriptor */

__ALIGN_BEGIN ETH_DMADescTypeDef  DMATxDscrTab[ETH_TXBUFNB] __ALIGN_END;/* Ethernet Tx DMA Descriptor */

__ALIGN_BEGIN uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __ALIGN_END; /* Ethernet Receive Buffer */

__ALIGN_BEGIN uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __ALIGN_END; /* Ethernet Transmit Buffer */

static ETH_HandleTypeDef EthHandle;
static ETH_TxPacketConfigTypeDef TxConfig;

static onLinkUpCallback onLinkUpCB = NULL;
static onLinkDownCallback onLinkDownCB = NULL;

static ETH_BufferTypeDef Rx_Link_Buff[ETH_RXBUFNB];

static ETH_BufferTypeDef *rx_frame_ptr = NULL;
static uint32_t rx_frame_len = 0U;
static uint32_t tx_buf_index = 0U;
static uint32_t rx_alloc_index = 0U;

static uint32_t Ethernet_AlignDown32(uint32_t addr)
{
    return addr & ~31UL;
}

static uint32_t Ethernet_AlignUp32(uint32_t size)
{
    return (size + 31UL) & ~31UL;
}

static void Ethernet_CleanDCache(void *addr, uint32_t len)
{
#if (__DCACHE_PRESENT == 1U)
    if ((addr == NULL) || (len == 0U)) {
        return;
    }

    /* D-Cache 沒開就不要呼叫 SCB_CleanDCache_by_Addr */
    if ((SCB->CCR & SCB_CCR_DC_Msk) == 0U) {
        return;
    }

    uintptr_t start = Ethernet_AlignDown32((uintptr_t)addr);
    uintptr_t end   = Ethernet_AlignUp32(((uintptr_t)addr) + len);
    uintptr_t size  = end - start;

    SCB_CleanDCache_by_Addr((uint32_t *)start, (int32_t)size);
#else
    (void)addr;
    (void)len;
#endif
}

static void Ethernet_InvalidateDCache(void *addr, uint32_t len)
{
#if (__DCACHE_PRESENT == 1U)
    if ((addr == NULL) || (len == 0U)) {
        return;
    }

    /* D-Cache 沒開就不要呼叫 SCB_InvalidateDCache_by_Addr */
    if ((SCB->CCR & SCB_CCR_DC_Msk) == 0U) {
        return;
    }

    uint32_t start = Ethernet_AlignDown32((uint32_t)addr);
    uint32_t size = Ethernet_AlignUp32(((uint32_t)addr + len) - start);
    SCB_InvalidateDCache_by_Addr((uint32_t *)start, (int32_t)size);
#else
    (void)addr;
    (void)len;
#endif
}

static void Ethernet_Release_Rx(void)
{
    rx_frame_ptr = NULL;
    rx_frame_len = 0U;
}

void HAL_ETH_RxAllocateCallback(uint8_t **buff)
{
    if (buff == NULL) {
        return;
    }

    *buff = NULL;

    if (rx_alloc_index >= ETH_RXBUFNB) {
        rx_alloc_index = 0U;
    }

    *buff = Rx_Buff[rx_alloc_index];

    rx_alloc_index++;
}

void HAL_ETH_RxLinkCallback(void **pStart, void **pEnd, uint8_t *buff, uint16_t Length)
{
    ETH_BufferTypeDef *node = NULL;

    if ((pStart == NULL) || (pEnd == NULL) || (buff == NULL) || (Length == 0U)) {
        return;
    }

    for (uint32_t i = 0U; i < ETH_RXBUFNB; i++) {
        if (buff == Rx_Buff[i]) {
            node = &Rx_Link_Buff[i];
            break;
        }
    }

    if (node == NULL) {
        return;
    }

    node->buffer = buff;
    node->len = Length;
    node->next = NULL;

    if (*pStart == NULL) {
        *pStart = node;
    } else {
        ((ETH_BufferTypeDef *)(*pEnd))->next = node;
    }

    *pEnd = node;
}

void HAL_ETH_TxFreeCallback(uint32_t *buff)
{
    (void)buff;
}

hwEthernet_OpResult Ethernet_Init(const uint8_t mac[6], onLinkUpCallback link_up_cb, onLinkDownCallback link_down_cb)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
#endif
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;

    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    gpio_pin_init_status[hwGPIO_Pin_A1] = true;
    gpio_pin_init_status[hwGPIO_Pin_A2] = true;
    gpio_pin_init_status[hwGPIO_Pin_A7] = true;

    GPIO_InitStruct.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    gpio_pin_init_status[hwGPIO_Pin_B13] = true;

    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    gpio_pin_init_status[hwGPIO_Pin_C1] = true;
    gpio_pin_init_status[hwGPIO_Pin_C4] = true;
    gpio_pin_init_status[hwGPIO_Pin_C5] = true;

    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_13;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    gpio_pin_init_status[hwGPIO_Pin_G11] = true;
    gpio_pin_init_status[hwGPIO_Pin_G13] = true;

    __HAL_RCC_ETH1MAC_CLK_ENABLE();
    __HAL_RCC_ETH1TX_CLK_ENABLE();
    __HAL_RCC_ETH1RX_CLK_ENABLE();

    EthHandle.Instance = ETH;
    EthHandle.Init.MACAddr = (uint8_t *)mac;
    EthHandle.Init.MediaInterface = HAL_ETH_RMII_MODE;
    EthHandle.Init.TxDesc = DMATxDscrTab;
    EthHandle.Init.RxDesc = DMARxDscrTab;
    EthHandle.Init.RxBuffLen = ETH_RX_BUF_SIZE;

    if (HAL_ETH_Init(&EthHandle) != HAL_OK) {
        return hwEthernet_HwError;
    }

    if (HAL_ETH_RegisterRxAllocateCallback(&EthHandle, HAL_ETH_RxAllocateCallback) != HAL_OK) {
        return hwEthernet_HwError;
    }

    if (HAL_ETH_RegisterRxLinkCallback(&EthHandle, HAL_ETH_RxLinkCallback) != HAL_OK) {
        return hwEthernet_HwError;
    }

    if (HAL_ETH_RegisterTxFreeCallback(&EthHandle, HAL_ETH_TxFreeCallback) != HAL_OK) {
        return hwEthernet_HwError;
    }

    memset(Rx_Link_Buff, 0, sizeof(Rx_Link_Buff));
    rx_frame_ptr = NULL;
    rx_frame_len = 0U;
    tx_buf_index = 0U;
    rx_alloc_index = 0U;

    memset(&TxConfig, 0, sizeof(TxConfig));
    TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
    TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
    TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;

    if (HAL_ETH_Start(&EthHandle) != HAL_OK) {
        return hwEthernet_HwError;
    }

    uint32_t regvalue = 0U;

    (void)HAL_ETH_ReadPHYRegister(&EthHandle, LAN8742A_PHY_ADDRESS, PHY_IMR, &regvalue);
    regvalue |= PHY_ISFR_INT4;
    (void)HAL_ETH_WritePHYRegister(&EthHandle, LAN8742A_PHY_ADDRESS, PHY_IMR, regvalue);

    onLinkUpCB = link_up_cb;
    onLinkDownCB = link_down_cb;

    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Output(const uint8_t *out_data, uint16_t out_len)
{
    ETH_BufferTypeDef txbuffer;
    uint8_t *txbuf;

    if ((out_data == NULL) || (out_len == 0U)) {
        return hwEthernet_InvalidParameter;
    }

    if ((out_len > ETH_MAX_PACKET_SIZE) || (out_len > ETH_TX_BUF_SIZE)) {
        return hwEthernet_BufferError;
    }

    txbuf = Tx_Buff[tx_buf_index];
    memcpy(txbuf, out_data, out_len);
    Ethernet_CleanDCache(txbuf, out_len);

    memset(&txbuffer, 0, sizeof(txbuffer));
    txbuffer.buffer = txbuf;
    txbuffer.len = out_len;
    txbuffer.next = NULL;

    TxConfig.Length = out_len;
    TxConfig.TxBuffer = &txbuffer;

    if (HAL_ETH_Transmit(&EthHandle, &TxConfig, ETH_DMA_TRANSMIT_TIMEOUT) != HAL_OK) {
        return hwEthernet_HwError;
    }

    tx_buf_index++;
    if (tx_buf_index >= ETH_TX_DESC_CNT) {
        tx_buf_index = 0U;
    }

    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Get_Input_Frame_Length(uint32_t *frame_len)
{
    ETH_BufferTypeDef *rxbuf = NULL;
    ETH_BufferTypeDef *node = NULL;
    uint32_t total_len = 0U;

    if (frame_len == NULL) {
        return hwEthernet_InvalidParameter;
    }

    *frame_len = 0U;

    if (HAL_ETH_ReadData(&EthHandle, (void **)&rxbuf) != HAL_OK) {
        return hwEthernet_Busy;
    }

    if (rxbuf == NULL) {
        return hwEthernet_Busy;
    }

    node = rxbuf;
    while (node != NULL) {
        if ((node->buffer == NULL) || (node->len == 0U)) {
            Ethernet_Release_Rx();
            return hwEthernet_Busy;
        }

        total_len += node->len;
        if (total_len > ETH_MAX_PACKET_SIZE) {
            Ethernet_Release_Rx();
            return hwEthernet_BufferError;
        }

        node = node->next;
    }

    rx_frame_ptr = rxbuf;
    rx_frame_len = total_len;
    *frame_len = rx_frame_len;

    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Input(uint8_t *in_data, uint32_t in_len)
{
    if ((in_data == NULL) || (in_len == 0U)) {
        return hwEthernet_InvalidParameter;
    }

    if ((rx_frame_ptr == NULL) || (rx_frame_len == 0U)) {
        Ethernet_Release_Rx();
        return hwEthernet_Busy;
    }

    if (in_len < rx_frame_len) {
        Ethernet_Release_Rx();
        return hwEthernet_BufferError;
    }

    uint32_t offset = 0U;
    ETH_BufferTypeDef *node = rx_frame_ptr;

    while (node != NULL) {
        Ethernet_InvalidateDCache(node->buffer, node->len);
        memcpy(&in_data[offset], node->buffer, node->len);
        offset += node->len;
        node = node->next;
    }

    Ethernet_Release_Rx();

    return hwEthernet_OK;
}

bool Ethernet_isInit(void)
{
    return (EthHandle.gState != HAL_ETH_STATE_RESET);
}


void Ethernet_Set_Link(void)
{
    uint32_t regvalue = 0U;

    (void)HAL_ETH_ReadPHYRegister(&EthHandle, LAN8742A_PHY_ADDRESS, PHY_ISFR, &regvalue);

    if ((regvalue & PHY_ISFR_INT4) != 0U) {
        if (onLinkDownCB != NULL) {
            onLinkDownCB();
        }
    }

    (void)HAL_ETH_ReadPHYRegister(&EthHandle, LAN8742A_PHY_ADDRESS, PHY_BSR, &regvalue);
    (void)HAL_ETH_ReadPHYRegister(&EthHandle, LAN8742A_PHY_ADDRESS, PHY_BSR, &regvalue);

    if ((regvalue & PHY_LINKED_STATUS) != 0U) {
        if (onLinkUpCB != NULL) {
            onLinkUpCB();
        }
    }
}

void Ethernet_Update_Config(bool isLinkUp)
{
    uint32_t regvalue = 0U;
    ETH_MACConfigTypeDef MACConf;

    if (isLinkUp) {
        (void)HAL_ETH_ReadPHYRegister(&EthHandle, LAN8742A_PHY_ADDRESS, PHY_SR, &regvalue);
        HAL_ETH_GetMACConfig(&EthHandle, &MACConf);

        if ((regvalue & PHY_DUPLEX_STATUS) != 0U) {
            MACConf.DuplexMode = ETH_FULLDUPLEX_MODE;
        } else {
            MACConf.DuplexMode = ETH_HALFDUPLEX_MODE;
        }

        if ((regvalue & PHY_SPEED_STATUS) != 0U) {
            MACConf.Speed = ETH_SPEED_10M;
        } else {
            MACConf.Speed = ETH_SPEED_100M;
        }

        (void)HAL_ETH_SetMACConfig(&EthHandle, &MACConf);
        (void)HAL_ETH_Start(&EthHandle);
    } else {
        (void)HAL_ETH_Stop(&EthHandle);
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
    mac[1] = 0x80;
    mac[2] = 0xE1;
    mac[3] = (baseUID & 0x00FF0000) >> 16;
    mac[4] = (baseUID & 0x0000FF00) >> 8;
    mac[5] = (baseUID & 0x000000FF);
}

#endif //STM32H7