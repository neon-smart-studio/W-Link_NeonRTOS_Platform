/******************************************************************************
  * \attention
  *
  * <h2><center>&copy; COPYRIGHT 2021 STMicroelectronics</center></h2>
  *
  * Licensed under ST MIX MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        www.st.com/mix_myliberty
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/
/******************************************************************************
 * This file contains code derived from or based on software provided by
 * STMicroelectronics.
 *
 * Original source:
 * STMicroelectronics X-CUBE / BSP / Middleware component
 *
 * Modifications:
 * Copyright (c) 2026 Neon Smart Studio
 * Author: Neon / Neona
 *
 * Licensed under:
 * - Original ST license: ST MIX MYLIBERTY SOFTWARE LICENSE AGREEMENT
 * - Additional modifications may be licensed separately where applicable.
 *
 * The original ST copyright and license notice are preserved below.
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"

#include "NeonRTOS.h"

#include "ST25R95_Def.h"

#include "ST25R95_IO.h"

#include "ST25R95.h"

static uint8_t ST25R95_CommandIDN[] = {ST25R95_COMMAND_IDN, 0x00};

static uint8_t ProtocolSelectCommandFieldOff[]     = {0x02, 0x02, 0x00, 0x00};
static uint8_t ProtocolSelectCommandISO15693[]     = {0x02, 0x02, 0x01, 0x0D};
static uint8_t ProtocolSelectCommandISO14443A[]    = {0x02, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t ProtocolSelectCommandISO14443B[]    = {0x02, 0x05, 0x03, 0x01, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t ProtocolSelectCommandISO18092[]     = {0x02, 0x05, 0x04, 0x51, 0x1F, 0x06, 0x00, 0x00}; /* WA: keep len=5 & do not use DD */
static uint8_t ProtocolSelectCommandCEISO14443A[]  = {0x02, 0x02, 0x12, 0x0A};

static uint8_t *ProtocolSelectCommands[6] = {
  ProtocolSelectCommandFieldOff,
  ProtocolSelectCommandISO15693,
  ProtocolSelectCommandISO14443A,
  ProtocolSelectCommandISO14443B,
  ProtocolSelectCommandISO18092,
  ProtocolSelectCommandCEISO14443A,
};

static uint8_t WrRegAnalogRegConfigISO15693[]   = {0x09, 0x04, 0x68, 0x01, 0x01, 0x53};
static uint8_t WrRegAnalogRegConfigISO14443A[]  = {0x09, 0x04, 0x68, 0x01, 0x01, 0xD3};
static uint8_t WrRegAnalogRegConfigISO14443B[]  = {0x09, 0x04, 0x68, 0x01, 0x01, 0x30};
static uint8_t WrRegAnalogRegConfigISO18092[]   = {0x09, 0x04, 0x68, 0x01, 0x01, 0x50};
static uint8_t WrRegAnalogRegConfigCEISO1443A[] = {0x09, 0x04, 0x68, 0x01, 0x04, 0x27};
static uint8_t *WrRegAnalogRegConfigs[6] = {
  NULL,
  WrRegAnalogRegConfigISO15693,
  WrRegAnalogRegConfigISO14443A,
  WrRegAnalogRegConfigISO14443B,
  WrRegAnalogRegConfigISO18092,
  WrRegAnalogRegConfigCEISO1443A
};

static uint8_t WrRegEnableAutoDetectFilter[] = {0x09, 0x04, 0x0A, 0x01, 0x02, 0xA1};
static uint8_t WrRegTimerWindowValue[]       = {0x09, 0x04, 0x3A, 0x00, 0x58, 0x04};

static uint8_t Calibrate[] = {ST25R95_COMMAND_IDLE, 0x0E, 0x03, 0xA1, 0x00, 0xB8, 0x01, 0x18, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x3F, 0x01};
static uint8_t WrRegAnalogRegConfigIndex[]  = {0x09, 0x03, 0x68, 0x00, 0x01};
static uint8_t RdRegAnalogRegConfig[]       = {0x08, 0x03, 0x69, 0x01, 0x00};

ST25R95_OpResult ST25R95_Field_On(ST25R95_Protocol protocol)
{
  if (protocol == ST25R95_Protocol_FieldOff) {
    protocol = ST25R95_Protocol_ISO15693;
  }
  return (ST25R95_ProtocolSelect(protocol));
}

ST25R95_OpResult ST25R95_Field_Off(void)
{
  return (ST25R95_ProtocolSelect(ST25R95_Protocol_FieldOff));
}

ST25R95_OpResult ST25R95_Set_BitRate(ST25R95_Protocol protocol, ST25R95_BitRate txBR, ST25R95_BitRate rxBR)
{
    uint8_t *conf;
    if ((protocol == ST25R95_Protocol_FieldOff) || (protocol > ST25R95_Protocol_MAX))
    {
      return ST25R95_InvalidParameter;
    }
    
    conf = &ProtocolSelectCommands[protocol][ST25R95_PROTOCOLSELECT_BR_OFFSET];
    *conf &= 0x0F;

    switch (protocol) {
      case (ST25R95_Protocol_ISO15693):
        switch (rxBR) {
          case (ST25R95_BitRate_26p48):
            break;
          case (ST25R95_BitRate_52p97):
            *conf |= 0x10;
            break;
          default:
            return ST25R95_Unsupport;
        }
        break;
      case (ST25R95_Protocol_ISO14443A):
        switch (txBR) {
          case (ST25R95_BitRate_106):
            break;
          case (ST25R95_BitRate_212):
            *conf |= 0x40;
            break;
          case (ST25R95_BitRate_424):
            *conf |= 0x80;
            break;
          default:
            return ST25R95_Unsupport;
        }
        switch (rxBR) {
          case (ST25R95_BitRate_106):
            break;
          case (ST25R95_BitRate_212):
            *conf |= 0x10;
            break;
          case (ST25R95_BitRate_424):
            *conf |= 0x20;
            break;
          default:
            return ST25R95_Unsupport;
        }
        break;
      case (ST25R95_Protocol_ISO14443B):
        switch (txBR) {
          case (ST25R95_BitRate_106):
            break;
          case (ST25R95_BitRate_212):
            *conf |= 0x40;
            break;
          case (ST25R95_BitRate_424):
            *conf |= 0x80;
            break;
          case (ST25R95_BitRate_848):
            *conf |= 0xC0;
            break;
          default:
            return ST25R95_Unsupport;
        }
        switch (rxBR) {
          case (ST25R95_BitRate_106):
            break;
          case (ST25R95_BitRate_212):
            *conf |= 0x10;
            break;
          case (ST25R95_BitRate_424):
            *conf |= 0x20;
            break;
          case (ST25R95_BitRate_848):
            *conf |= 0x30;
            break;
          default:
            return ST25R95_Unsupport;
        }
        break;
      case (ST25R95_Protocol_ISO18092):
        switch (txBR) {
          case (ST25R95_BitRate_212):
            *conf |= 0x40;
            break;
          case (ST25R95_BitRate_424):
            *conf |= 0x80;
            break;
          default:
            return ST25R95_Unsupport;
        }
        switch (rxBR) {
          case (ST25R95_BitRate_212):
            *conf |= 0x10;
            break;
          case (ST25R95_BitRate_424):
            *conf |= 0x20;
            break;
          default:
            return ST25R95_Unsupport;
        }
        break;
      case (ST25R95_Protocol_CE_ISO14443A):
        switch (txBR) {
          case (ST25R95_BitRate_106):
            break;
          default:
            return ST25R95_Unsupport;
        }
        switch (rxBR) {
          case (ST25R95_BitRate_106):
            break;
          default:
            return ST25R95_Unsupport;
        }
        break;
      default:
        return ST25R95_Unsupport;
    }

    return ST25R95_IO_Set_BitRate(txBR, rxBR);
}

ST25R95_OpResult ST25R95_Set_FWT(ST25R95_Protocol protocol, uint32_t fwt)
{
    uint8_t PP;
    uint32_t MM;
    uint32_t DD;
    uint32_t FWT;

    FWT = (fwt < ST25R95_FWT_MAX) ? fwt : ST25R95_FWT_MAX;     /* Limit the FWT to the max supported */
    fwt = FWT;
    PP = 0;

    if (protocol == ST25R95_Protocol_ISO18092) {
      /* Workaround for ST25R95_Protocol_ISO18092:
      * DD parameters seems to overwritten by MM by the ROM code.
      * So this parameter should not be used (i.e ProtocolSelect Len should be 5)
      */
      DD = 0; /* Should not be used in protocolSelect */
      while (FWT > ((128U + 1U) * (128U) * 32U)) {
        PP++;
        FWT /= 2U;
      }
      MM = FWT / (128U * 32U);
    } else {
      while (FWT > ((128 + 1) * (255) * 32)) {
        PP++;
        FWT /= 2;
      }
      do {
        if (FWT > ((64 + 1) * (255) * 32)) {
          MM = 128UL;
          break;
        }
        if (FWT > ((32 + 1) * (255) * 32)) {
          MM = 64UL;
          break;
        }
        if (FWT > ((16 + 1) * (255) * 32)) {
          MM = 32UL;
          break;
        }
        if (FWT > ((8 + 1) * (255) * 32)) {
          MM = 16UL;
          break;
        }
        if (FWT > ((4 + 1) * (255) * 32)) {
          MM = 8UL;
          break;
        }
        if (FWT > ((2 + 1) * (255) * 32)) {
          MM = 4UL;
          break;
        }
        if (FWT > ((1 + 1) * (255) * 32)) {
          MM = 2UL;
          break;
        }
        if (FWT > ((0 + 1) * (255) * 32)) {
          MM = 1UL;
          break;
        }
        MM = 0UL;
      } while (0);

      DD = (((FWT + 31UL) / 32UL) + MM) / (MM + 1UL);
      DD = (DD > 128) ? DD - 128UL : 0;
    }

    switch (protocol) {
      case ST25R95_Protocol_ISO14443A:
        if (
          (ProtocolSelectCommandISO14443A[4] != PP) ||
          (ProtocolSelectCommandISO14443A[5] != MM) ||
          (ProtocolSelectCommandISO14443A[6] != DD)) {
          ProtocolSelectCommandISO14443A[4] = PP;
          ProtocolSelectCommandISO14443A[5] = (uint8_t)MM;
          ProtocolSelectCommandISO14443A[6] = (uint8_t)DD;

          return (ST25R95_ProtocolSelect(protocol));
        }
        break;

      case ST25R95_Protocol_ISO14443B:
        if (
          (ProtocolSelectCommandISO14443B[4] != PP) ||
          (ProtocolSelectCommandISO14443B[5] != MM) ||
          (ProtocolSelectCommandISO14443B[6] != DD)) {
          ProtocolSelectCommandISO14443B[4] = PP;
          ProtocolSelectCommandISO14443B[5] = (uint8_t)MM;
          ProtocolSelectCommandISO14443B[6] = (uint8_t)DD;

          return (ST25R95_ProtocolSelect(protocol));
        }
        break;

      case ST25R95_Protocol_ISO18092:
        if (
          (ProtocolSelectCommandISO18092[5] != PP) ||
          (ProtocolSelectCommandISO18092[6] != MM) ||
          (ProtocolSelectCommandISO18092[7] != DD)) {
          ProtocolSelectCommandISO18092[5] = PP;
          ProtocolSelectCommandISO18092[6] = (uint8_t)MM;
          ProtocolSelectCommandISO18092[7] = (uint8_t)DD;

          return (ST25R95_ProtocolSelect(protocol));
        }
        break;

      default:
        break;
    }
    return ST25R95_OK;
}

bool ST25R95_CheckChipID(void)
{
    bool ret = false;
    uint8_t respBuffer[ST25R95_IDN_RESPONSE_BUFLEN];

    if (ST25R95_IO_SPI_Send_Command_Type_And_Len(ST25R95_CommandIDN, respBuffer, ST25R95_IDN_RESPONSE_BUFLEN) == ST25R95_OK)
    {
      if (respBuffer[ST25R95_CMD_LENGTH_OFFSET] != 0)
      {
        ret = (strcmp((const char *)&respBuffer[ST25R95_CMD_DATA_OFFSET], "NFC FS2JAST4") == 0);
      }
    }

    return (ret);
}

ST25R95_OpResult ST25R95_Set_SlotCounter(ST25R95_FeliCaPollSlots slots)
{
    if ((ProtocolSelectCommandISO18092[4] & 0xF) != slots)
    {
      ProtocolSelectCommandISO18092[4] &= 0xF0;
      ProtocolSelectCommandISO18092[4] |= (slots & 0xF);
      return (ST25R95_Protocol_Select(ST25R95_Protocol_ISO18092));
    }

    return ST25R95_OK;
}

ST25R95_OpResult ST25R95_Protocol_Select(ST25R95_Protocol protocol)
{
    ST25R95_OpResult op_status;

    uint8_t protocolResp[ST25R95_PROTOCOLSELECT_RESPONSE_BUFLEN];
    uint8_t wrregResp[ST25R95_WRREG_RESPONSE_BUFLEN];

    if (protocol > ST25R95_Protocol_CE_ISO14443A)
    {
        return ST25R95_InvalidParameter;
    }

    if (ProtocolSelectCommands[protocol] == NULL)
    {
        return ST25R95_InvalidParameter;
    }

    op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(ProtocolSelectCommands[protocol], protocolResp, ST25R95_PROTOCOLSELECT_RESPONSE_BUFLEN);
    if ((op_status == ST25R95_OK) && (protocolResp[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_NONE)) {
      return ST25R95_InvalidParameter;
    }

    /* Adjust ARC_B or ACC_A register */
    if ((protocol != ST25R95_Protocol_FieldOff)) {
      op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(WrRegAnalogRegConfigs[protocol], wrregResp, ST25R95_WRREG_RESPONSE_BUFLEN);
      if(op_status < ST25R95_OK)
      {
          return op_status;
      }
    }

    if (protocol == ST25R95_Protocol_ISO18092) {
      op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(WrRegEnableAutoDetectFilter, wrregResp, ST25R95_WRREG_RESPONSE_BUFLEN);
      if(op_status < ST25R95_OK)
      {
          return op_status;
      }
    }
    if (protocol == ST25R95_Protocol_ISO14443A) {
      op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(WrRegTimerWindowValue, wrregResp, ST25R95_WRREG_RESPONSE_BUFLEN);
      if(op_status < ST25R95_OK)
      {
          return op_status;
      }
    }
    
    return (ST25R95_OK);
}

ST25R95_OpResult ST25R95_Calibrate_Tag_Detector(uint8_t* pCalibrate)
{
    ST25R95_OpResult op_status;

    const uint8_t steps[6] = {0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x4U};
    uint8_t       respBuffer[ST25R95_IDLE_RESPONSE_BUFLEN];
    uint8_t       i;

    /* 8 steps dichotomy implementation as per AN3433 */

    /* Check that wake up detection is tag detect (0x02) when DacDataH is Min Dac value 0x00 */
    Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] = 0x00U;

    op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(Calibrate, respBuffer, ST25R95_IDLE_RESPONSE_BUFLEN);
    if(op_status < ST25R95_OK)
    {
        return op_status;
    }

    if ((respBuffer[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_NONE) || (respBuffer[ST25R95_CMD_LENGTH_OFFSET] != 0x01) || (respBuffer[ST25R95_CMD_DATA_OFFSET] != ST25R95_IDLE_WKUP_TAGDETECT)) {
        *pCalibrate = (0xFFU);
        return ST25R95_CalibrateError;
    }

    /* Check that wake up detection is timeout (0x01) when DacDataH is Max Dac value 0xFC */
    Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] = ST25R95_DACDATA_MAX;

    op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(Calibrate, respBuffer, ST25R95_IDLE_RESPONSE_BUFLEN);
    if(op_status < ST25R95_OK)
    {
        return op_status;
    }

    if ((respBuffer[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_NONE) || (respBuffer[ST25R95_CMD_LENGTH_OFFSET] != 0x01) || (respBuffer[ST25R95_CMD_DATA_OFFSET] != ST25R95_IDLE_WKUP_TIMEOUT)) {
        *pCalibrate = (0xFFU);
        return ST25R95_CalibrateError;
    }

    for (i = 0; i < 6; i++)
    {
        switch (respBuffer[ST25R95_CMD_DATA_OFFSET])
        {
          case ST25R95_IDLE_WKUP_TIMEOUT:
            Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] -= steps[i];
            break;
          case ST25R95_IDLE_WKUP_TAGDETECT:
            Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] += steps[i];
            break;
          default:
            return ST25R95_System;
            /*NOTREACHED*/
            break;
        }

        respBuffer[ST25R95_CMD_DATA_OFFSET] = 0x00U;

        op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(Calibrate, respBuffer, ST25R95_IDLE_RESPONSE_BUFLEN);
        if(op_status < ST25R95_OK)
        {
            return op_status;
        }
    }

    if (respBuffer[2U] == ST25R95_IDLE_WKUP_TIMEOUT) {
      Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] -= 0x04U;
    }

    *pCalibrate = (Calibrate[ST25R95_IDLE_DACDATAH_OFFSET]);

    return ST25R95_OK;
}

ST25R95_OpResult ST25R95_Read_Reg(uint16_t reg, uint8_t *value)
{
  ST25R95_OpResult op_status;

  uint8_t respBuffer[ST25R95_RDREG_RESPONSE_BUFLEN];

  switch (reg) {
    case (ST25R95_REG_ARC_B):
      WrRegAnalogRegConfigIndex[4U] = 0x01U;
      break;

    case (ST25R95_REG_ACC_A):
      WrRegAnalogRegConfigIndex[4U] = 0x04U;
      break;

    default:
      return ST25R95_InvalidParameter;
  }

  op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(WrRegAnalogRegConfigIndex, respBuffer, ST25R95_RDREG_RESPONSE_BUFLEN);
  if(op_status < ST25R95_OK)
  {
      return op_status;
  }

  if (respBuffer[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_NONE)
  {
      return ST25R95_InvalidParameter;
  }

  op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(RdRegAnalogRegConfig, respBuffer, ST25R95_RDREG_RESPONSE_BUFLEN);
  if(op_status < ST25R95_OK)
  {
      return op_status;
  }

  if (respBuffer[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_NONE)
  {
    return ST25R95_InvalidParameter;
  }

  *value = respBuffer[2];

  return ST25R95_OK;
}

ST25R95_OpResult ST25R95_Write_Reg(ST25R95_Protocol protocol, uint16_t reg, uint8_t value)
{
  ST25R95_OpResult op_status;

  uint8_t respBuffer[ST25R95_WRREG_RESPONSE_BUFLEN];

  switch (reg) {
    case (ST25R95_REG_ARC_B):
      if ((protocol == ST25R95_Protocol_ISO15693)  ||
          (protocol == ST25R95_Protocol_ISO14443A) ||
          (protocol == ST25R95_Protocol_ISO14443B) ||
          (protocol == ST25R95_Protocol_ISO18092))
          {
              WrRegAnalogRegConfigs[protocol][5] = value;

              op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(WrRegAnalogRegConfigs[protocol], respBuffer, ST25R95_WRREG_RESPONSE_BUFLEN);
              if(op_status < ST25R95_OK)
              {
                  return op_status;
              }
              
              if(respBuffer[ST25R95_CMD_RESULT_OFFSET] != 0)
              {
                  return ST25R95_InvalidParameter;
              }
          }
          else
          {
              return ST25R95_WrongState;
          }
      break;

    case (ST25R95_REG_ACC_A):
      if (protocol == ST25R95_Protocol_CE_ISO14443A)
      {
          WrRegAnalogRegConfigs[protocol][5] = value;

          op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(WrRegAnalogRegConfigs[protocol], respBuffer, ST25R95_WRREG_RESPONSE_BUFLEN);
          
          if(op_status < ST25R95_OK)
          {
              return op_status;
          }

          if(respBuffer[ST25R95_CMD_RESULT_OFFSET] != 0)
          {
              return ST25R95_InvalidParameter;
          }
      }
      else
      {
          return ST25R95_WrongState;
      }
      break;
    default:
      return ST25R95_InvalidParameter;
  }

  return ST25R95_OK;
}
