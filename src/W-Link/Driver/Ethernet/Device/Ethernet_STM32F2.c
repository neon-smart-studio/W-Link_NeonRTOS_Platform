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

#ifdef STM32F2

#ifdef CONFIG_ETHERNET_ONBOARD

#include "GPIO/Device/STM32/GPIO_STM32.h"

#define LAN8742A_PHY_ADDRESS            0x00U

/* Definition of the Ethernet driver buffers size and count */
#define ETH_RX_BUF_SIZE     ETH_MAX_PACKET_SIZE /* buffer size for receive               */
#define ETH_TX_BUF_SIZE     ETH_MAX_PACKET_SIZE /* buffer size for transmit              */
#define ETH_RXBUFNB         4U       /* 4 Rx buffers of size ETH_RX_BUF_SIZE  */
#define ETH_TXBUFNB         4U       /* 4 Tx buffers of size ETH_TX_BUF_SIZE  */

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

static onLinkUpCallback onLinkUpCB = NULL;
static onLinkDownCallback onLinkDownCB = NULL;

static void Ethernet_Release_Rx(void)
{
    __IO ETH_DMADescTypeDef *dmarxdesc;
    uint32_t i;

    dmarxdesc = EthHandle.RxFrameInfos.FSRxDesc;

    for (i = 0; i < EthHandle.RxFrameInfos.SegCount; i++) {
        dmarxdesc->Status |= ETH_DMARXDESC_OWN;
        dmarxdesc = (ETH_DMADescTypeDef *)dmarxdesc->Buffer2NextDescAddr;
    }

    EthHandle.RxFrameInfos.SegCount = 0;

    if ((EthHandle.Instance->DMASR & ETH_DMASR_RBUS) != 0U) {
        EthHandle.Instance->DMASR = ETH_DMASR_RBUS;
        EthHandle.Instance->DMARPDR = 0;
    }
}

hwEthernet_OpResult Ethernet_Init(const uint8_t mac[6], onLinkUpCallback link_up_cb, onLinkDownCallback link_down_cb)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* ---------- Enable clocks ---------- */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    /* ---------- GPIOA ---------- */
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
#endif
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    gpio_pin_init_status[hwGPIO_Pin_A1] = true;
    gpio_pin_init_status[hwGPIO_Pin_A2] = true;
    gpio_pin_init_status[hwGPIO_Pin_A7] = true;

    /* ---------- GPIOB ---------- */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    gpio_pin_init_status[hwGPIO_Pin_B13] = true;

    /* ---------- GPIOC ---------- */
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    gpio_pin_init_status[hwGPIO_Pin_C1] = true;
    gpio_pin_init_status[hwGPIO_Pin_C4] = true;
    gpio_pin_init_status[hwGPIO_Pin_C5] = true;

    /* ---------- GPIOG ---------- */
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_13;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    gpio_pin_init_status[hwGPIO_Pin_G11] = true;
    gpio_pin_init_status[hwGPIO_Pin_G13] = true;

    __HAL_RCC_ETH_CLK_ENABLE();

    uint32_t regvalue;

    EthHandle.Instance = ETH;
    EthHandle.Init.MACAddr = (uint8_t *)mac;
    EthHandle.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
    EthHandle.Init.Speed = ETH_SPEED_100M;
    EthHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
    EthHandle.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
    EthHandle.Init.RxMode = ETH_RXPOLLING_MODE;
    EthHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
    EthHandle.Init.PhyAddress = LAN8742A_PHY_ADDRESS;

    /* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
    if(HAL_ETH_Init(&EthHandle) != HAL_OK) {
        return hwEthernet_HwError;
    }

    /* Initialize Tx Descriptors list: Chain Mode */
    HAL_ETH_DMATxDescListInit(&EthHandle, DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);

    /* Initialize Rx Descriptors list: Chain Mode  */
    HAL_ETH_DMARxDescListInit(&EthHandle, DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

    /* Enable MAC and DMA transmission and reception */
    HAL_ETH_Start(&EthHandle);

    /**** Configure PHY to generate an interrupt when Eth Link state changes ****/
    /* Read Register Configuration */
    HAL_ETH_ReadPHYRegister(&EthHandle, PHY_IMR, &regvalue);

    regvalue |= PHY_ISFR_INT4;

    /* Enable Interrupt on change of link status */
    HAL_ETH_WritePHYRegister(&EthHandle, PHY_IMR, regvalue);

    onLinkUpCB = link_up_cb;
    onLinkDownCB = link_down_cb;
    
    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Output(const uint8_t *out_data, uint16_t out_len)
{
    hwEthernet_OpResult ret = hwEthernet_OK;
    __IO ETH_DMADescTypeDef *DmaTxDesc;
    uint8_t *buffer;
    uint32_t framelength = 0U;
    uint32_t bufferoffset = 0U;
    uint32_t byteslefttocopy;
    uint32_t payloadoffset = 0U;
    uint32_t desc_count = 0U;

    if ((out_data == NULL) || (out_len == 0U)) {
        return hwEthernet_InvalidParameter;
    }

    if (out_len > ETH_MAX_PACKET_SIZE) {
        return hwEthernet_BufferError;
    }

    DmaTxDesc = EthHandle.TxDesc;
    buffer = (uint8_t *)DmaTxDesc->Buffer1Addr;
    byteslefttocopy = out_len;
    payloadoffset = 0U;
    bufferoffset = 0U;

    /* Reference ST ethernetif.c flow: copy payload into chained DMA Tx buffers. */
    while (byteslefttocopy > 0U) {
        uint32_t copy_len;

        if (desc_count >= ETH_TXBUFNB) {
            ret = hwEthernet_BufferError;
            break;
        }

        if ((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != 0U) {
            ret = hwEthernet_Busy;
            break;
        }

        copy_len = ETH_TX_BUF_SIZE - bufferoffset;
        if (copy_len > byteslefttocopy) {
            copy_len = byteslefttocopy;
        }

        memcpy(buffer + bufferoffset, out_data + payloadoffset, copy_len);

        byteslefttocopy -= copy_len;
        payloadoffset += copy_len;
        framelength += copy_len;
        bufferoffset += copy_len;

        if (byteslefttocopy > 0U) {
            DmaTxDesc = (ETH_DMADescTypeDef *)DmaTxDesc->Buffer2NextDescAddr;
            buffer = (uint8_t *)DmaTxDesc->Buffer1Addr;
            bufferoffset = 0U;
            desc_count++;
        }
    }

    if (ret == hwEthernet_OK) {
        if (HAL_ETH_TransmitFrame(&EthHandle, framelength) != HAL_OK) {
            ret = hwEthernet_HwError;
        }
    }

    if ((EthHandle.Instance->DMASR & ETH_DMASR_TUS) != 0U) {
        EthHandle.Instance->DMASR = ETH_DMASR_TUS;
        EthHandle.Instance->DMATPDR = 0U;
    }

    return ret;
}

hwEthernet_OpResult Ethernet_Get_Input_Frame_Length(uint32_t* frame_len)
{
    if ((frame_len == 0U)) {
        return hwEthernet_InvalidParameter;
    }

    if (HAL_ETH_GetReceivedFrame(&EthHandle) != HAL_OK) {
        return hwEthernet_Busy;
    }
    
    *frame_len = EthHandle.RxFrameInfos.length;

    if(*frame_len == 0 || *frame_len > ETH_MAX_PACKET_SIZE)
    {
        Ethernet_Release_Rx();

        return hwEthernet_Busy;
    }

    return hwEthernet_OK;
}

hwEthernet_OpResult Ethernet_Input(uint8_t *in_data, uint32_t in_len)
{
    uint8_t *buffer;
    __IO ETH_DMADescTypeDef *dmarxdesc;
    uint32_t bufferoffset = 0U;
    uint32_t payloadoffset = 0U;
    uint32_t byteslefttocopy;

    if ((in_len == 0U) || (in_data == NULL)) {
        return hwEthernet_InvalidParameter;
    }

    if ((in_len > ETH_MAX_PACKET_SIZE)) {
        Ethernet_Release_Rx();
        
        return hwEthernet_BufferError;
    }

    buffer = (uint8_t *)EthHandle.RxFrameInfos.buffer;
    dmarxdesc = EthHandle.RxFrameInfos.FSRxDesc;
    byteslefttocopy = in_len;
    bufferoffset = 0U;
    payloadoffset = 0U;

    /* Reference ST ethernetif.c flow: copy chained DMA Rx buffers into flat W-Link buffer. */
    while (byteslefttocopy > 0U) {
        uint32_t copy_len = ETH_RX_BUF_SIZE - bufferoffset;

        if (copy_len > byteslefttocopy) {
            copy_len = byteslefttocopy;
        }

        memcpy(in_data + payloadoffset, buffer + bufferoffset, copy_len);

        byteslefttocopy -= copy_len;
        payloadoffset += copy_len;
        bufferoffset += copy_len;

        if (byteslefttocopy > 0U) {
            dmarxdesc = (ETH_DMADescTypeDef *)dmarxdesc->Buffer2NextDescAddr;
            buffer = (uint8_t *)dmarxdesc->Buffer1Addr;
            bufferoffset = 0U;
        }
    }

    Ethernet_Release_Rx();
    
    return hwEthernet_OK;
}

bool Ethernet_isInit(void)
{
  return (EthHandle.State != HAL_ETH_STATE_RESET);
}

void Ethernet_Set_Link()
{
  uint32_t regvalue = 0;

  /* Read PHY_MISR*/
  HAL_ETH_ReadPHYRegister(&EthHandle, PHY_ISFR, &regvalue);

  /* Check whether the link interrupt has occurred or not */
  if ((regvalue & PHY_ISFR_INT4) != (uint16_t)RESET) {
    if(onLinkDownCB != NULL)
    {
      onLinkDownCB();
    }
  }

  HAL_ETH_ReadPHYRegister(&EthHandle, PHY_BSR, &regvalue);

  if ((regvalue & PHY_LINKED_STATUS) != (uint16_t)RESET) {
    if(onLinkUpCB != NULL)
    {
      onLinkUpCB();
    }
  }
}

void Ethernet_Update_Config(bool isLinkUp)
{
  uint32_t regvalue = 0;

  if (isLinkUp) {
    /* Restart the auto-negotiation */
    if (EthHandle.Init.AutoNegotiation != ETH_AUTONEGOTIATION_DISABLE) {

      /* Check Auto negotiation */
      HAL_ETH_ReadPHYRegister(&EthHandle, PHY_SR, &regvalue);
      if ((regvalue & PHY_SR_AUTODONE) != PHY_SR_AUTODONE) {
        goto error;
      }

      /* Configure the MAC with the Duplex Mode fixed by the auto-negotiation process */
      if ((regvalue & PHY_DUPLEX_STATUS) != (uint32_t)RESET) {
        /* Set Ethernet duplex mode to Full-duplex following the auto-negotiation */
        EthHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
      } else {
        /* Set Ethernet duplex mode to Half-duplex following the auto-negotiation */
        EthHandle.Init.DuplexMode = ETH_MODE_HALFDUPLEX;
      }
      /* Configure the MAC with the speed fixed by the auto-negotiation process */
      if (regvalue & PHY_SPEED_STATUS) {
        /* Set Ethernet speed to 10M following the auto-negotiation */
        EthHandle.Init.Speed = ETH_SPEED_10M;
      } else {
        /* Set Ethernet speed to 100M following the auto-negotiation */
        EthHandle.Init.Speed = ETH_SPEED_100M;
      }
    } else { /* AutoNegotiation Disable */
error :
      /* Check parameters */
      assert_param(IS_ETH_SPEED(EthHandle.Init.Speed));
      assert_param(IS_ETH_DUPLEX_MODE(EthHandle.Init.DuplexMode));

      /* Set MAC Speed and Duplex Mode to PHY */
      HAL_ETH_WritePHYRegister(&EthHandle, PHY_BCR, ((uint16_t)(EthHandle.Init.DuplexMode >> 3) |
                                                     (uint16_t)(EthHandle.Init.Speed >> 1)));
    }

    /* ETHERNET MAC Re-Configuration */
    HAL_ETH_ConfigMAC(&EthHandle, (ETH_MACInitTypeDef *) NULL);

    /* Restart MAC interface */
    HAL_ETH_Start(&EthHandle);
  } else {
    /* Stop MAC interface */
    HAL_ETH_Stop(&EthHandle);
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

#ifndef HASH_BITS
#define HASH_BITS 6 /* #bits in hash */
#endif

static uint32_t ethcrc(const uint8_t *data, size_t length)
{
  uint32_t crc = 0xffffffff;
  size_t i;
  int j;

  for (i = 0; i < length; i++) {
    for (j = 0; j < 8; j++) {
      if (((crc >> 31) ^ (data[i] >> j)) & 0x01) {
        /* x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1 */
        crc = (crc << 1) ^ 0x04C11DB7;
      } else {
        crc = crc << 1;
      }
    }
  }
  return ~crc;
}

hwEthernet_OpResult Ethernet_Register_Multicast_Address(const uint8_t *mac, uint32_t *eth_HashTableHigh, uint32_t *eth_HashTableLow)
{
  uint32_t crc;
  uint8_t hash;

  /* Calculate crc32 value of mac address */
  crc = ethcrc(mac, HASH_BITS);

  /*
   * Only upper HASH_BITS are used
   * which point to specific bit in the hash registers
   */
  hash = (crc >> 26) & 0x3F;

  if (hash > 31) {
    *eth_HashTableHigh |= 1 << (hash - 32);
    EthHandle.Instance->MACHTHR = *eth_HashTableHigh;
  } else {
    *eth_HashTableLow |= 1 << hash;
    EthHandle.Instance->MACHTLR = *eth_HashTableLow;
  }

  return hwEthernet_OK;
}

#endif //CONFIG_ETHERNET_ONBOARD

#endif //STM32F2