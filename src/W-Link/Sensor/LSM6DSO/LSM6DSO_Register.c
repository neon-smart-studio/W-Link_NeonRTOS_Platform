/*
This LSM6DSO driver is based on STMicroelectronics stm32-lsm6dso / LSM6DSO component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "LSM6DSO_Def.h"

#include "LSM6DSO_Register_Def.h"
#include "LSM6DSO_Register.h"

#define LSM6DSO_BYTE_COPY_INTERNAL(target, source) if ( (target != NULL) && (source != NULL) ) { *target = *source; }

LSM6DSO_OpStatus LSM6DSO_Read_Reg(uint8_t reg, uint8_t* data, uint16_t len)
{
  return LSM6DSO_IO_Read(reg, data, len);
}

LSM6DSO_OpStatus LSM6DSO_Write_Reg(uint8_t reg, uint8_t* data, uint16_t len)
{
  return LSM6DSO_IO_Write(reg, data, len);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Full_Scale_Set(LSM6DSO_FS_XL val)
{
  LSM6DSO_Ctrl1_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.fs_xl = (uint8_t) val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL1_XL, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Full_Scale_Get(LSM6DSO_FS_XL *val)
{
  LSM6DSO_Ctrl1_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.fs_xl) {
    case LSM6DSO_2g:
      *val = LSM6DSO_2g;
      break;
    case LSM6DSO_16g:
      *val = LSM6DSO_16g;
      break;
    case LSM6DSO_4g:
      *val = LSM6DSO_4g;
      break;
    case LSM6DSO_8g:
      *val = LSM6DSO_8g;
      break;
    default:
      *val = LSM6DSO_2g;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Data_Rate_Set(LSM6DSO_ODR_XL val)
{
  LSM6DSO_ODR_XL odr_xl = val;
  LSM6DSO_Emb_FSM_Enable fsm_enable;
  LSM6DSO_FSM_ODR fsm_odr;
  LSM6DSO_Ctrl1_XL reg;
  LSM6DSO_OpStatus op_status;

  /* Check the Finite State Machine data rate constraints */
  op_status =  LSM6DSO_Reg_FSM_Enable_Get(&fsm_enable);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  if ( (fsm_enable.fsm_enable_a.fsm1_en  |
        fsm_enable.fsm_enable_a.fsm2_en  |
        fsm_enable.fsm_enable_a.fsm3_en  |
        fsm_enable.fsm_enable_a.fsm4_en  |
        fsm_enable.fsm_enable_a.fsm5_en  |
        fsm_enable.fsm_enable_a.fsm6_en  |
        fsm_enable.fsm_enable_a.fsm7_en  |
        fsm_enable.fsm_enable_a.fsm8_en  |
        fsm_enable.fsm_enable_b.fsm9_en  |
        fsm_enable.fsm_enable_b.fsm10_en |
        fsm_enable.fsm_enable_b.fsm11_en |
        fsm_enable.fsm_enable_b.fsm12_en |
        fsm_enable.fsm_enable_b.fsm13_en |
        fsm_enable.fsm_enable_b.fsm14_en |
        fsm_enable.fsm_enable_b.fsm15_en |
        fsm_enable.fsm_enable_b.fsm16_en ) == PROPERTY_ENABLE ){

    op_status = LSM6DSO_Reg_FSM_Data_Rate_Get(&fsm_odr);
    if (op_status < LSM6DSO_OK) {
      return op_status;
    }
    
    switch (fsm_odr) {
      case LSM6DSO_ODR_FSM_12Hz5:

        if (val == LSM6DSO_XL_ODR_OFF){
          odr_xl = LSM6DSO_XL_ODR_12Hz5;

        } else {
          odr_xl = val;
        }
        break;
      case LSM6DSO_ODR_FSM_26Hz:

        if (val == LSM6DSO_XL_ODR_OFF){
          odr_xl = LSM6DSO_XL_ODR_26Hz;

        } else if (val == LSM6DSO_XL_ODR_12Hz5){
          odr_xl = LSM6DSO_XL_ODR_26Hz;

        } else {
          odr_xl = val;
        }
        break;
      case LSM6DSO_ODR_FSM_52Hz:

        if (val == LSM6DSO_XL_ODR_OFF){
          odr_xl = LSM6DSO_XL_ODR_52Hz;

        } else if (val == LSM6DSO_XL_ODR_12Hz5){
          odr_xl = LSM6DSO_XL_ODR_52Hz;

        } else if (val == LSM6DSO_XL_ODR_26Hz){
          odr_xl = LSM6DSO_XL_ODR_52Hz;

        } else {
          odr_xl = val;
        }
        break;
      case LSM6DSO_ODR_FSM_104Hz:

        if (val == LSM6DSO_XL_ODR_OFF){
          odr_xl = LSM6DSO_XL_ODR_104Hz;

        } else if (val == LSM6DSO_XL_ODR_12Hz5){
          odr_xl = LSM6DSO_XL_ODR_104Hz;

        } else if (val == LSM6DSO_XL_ODR_26Hz){
          odr_xl = LSM6DSO_XL_ODR_104Hz;

        } else if (val == LSM6DSO_XL_ODR_52Hz){
          odr_xl = LSM6DSO_XL_ODR_104Hz;

        } else {
          odr_xl = val;
        }
        break;
      default:
        odr_xl = val;
        break;
    }
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.odr_xl = (uint8_t) odr_xl;
  return LSM6DSO_Write_Reg(LSM6DSO_CTRL1_XL, (uint8_t*)&reg, 1);
}

/**
  * @brief  Accelerometer UI data rate selection.[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      Get the values of odr_xl in reg CTRL1_XL
  *
  */
LSM6DSO_OpStatus LSM6DSO_Reg_XL_Data_Rate_Get(LSM6DSO_ODR_XL *val)
{
  LSM6DSO_Ctrl1_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.odr_xl) {
    case LSM6DSO_XL_ODR_OFF:
      *val = LSM6DSO_XL_ODR_OFF;
      break;
    case LSM6DSO_XL_ODR_12Hz5:
      *val = LSM6DSO_XL_ODR_12Hz5;
      break;
    case LSM6DSO_XL_ODR_26Hz:
      *val = LSM6DSO_XL_ODR_26Hz;
      break;
    case LSM6DSO_XL_ODR_52Hz:
      *val = LSM6DSO_XL_ODR_52Hz;
      break;
    case LSM6DSO_XL_ODR_104Hz:
      *val = LSM6DSO_XL_ODR_104Hz;
      break;
    case LSM6DSO_XL_ODR_208Hz:
      *val = LSM6DSO_XL_ODR_208Hz;
      break;
    case LSM6DSO_XL_ODR_417Hz:
      *val = LSM6DSO_XL_ODR_417Hz;
      break;
    case LSM6DSO_XL_ODR_833Hz:
      *val = LSM6DSO_XL_ODR_833Hz;
      break;
    case LSM6DSO_XL_ODR_1667Hz:
      *val = LSM6DSO_XL_ODR_1667Hz;
      break;
    case LSM6DSO_XL_ODR_3333Hz:
      *val = LSM6DSO_XL_ODR_3333Hz;
      break;
    case LSM6DSO_XL_ODR_6667Hz:
      *val = LSM6DSO_XL_ODR_6667Hz;
      break;
    case LSM6DSO_XL_ODR_1Hz6:
      *val = LSM6DSO_XL_ODR_1Hz6;
      break;
    default:
      *val = LSM6DSO_XL_ODR_OFF;
      break;
  }
  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Full_Scale_Set(LSM6DSO_FS_G val)
{
  LSM6DSO_Ctrl2_G reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL2_G, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.fs_g = (uint8_t) val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL2_G, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Full_Scale_Get(LSM6DSO_FS_G *val)
{
  LSM6DSO_Ctrl2_G reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL2_G, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.fs_g) {
    case LSM6DSO_250dps:
      *val = LSM6DSO_250dps;
      break;
    case LSM6DSO_125dps:
      *val = LSM6DSO_125dps;
      break;
    case LSM6DSO_500dps:
      *val = LSM6DSO_500dps;
      break;
    case LSM6DSO_1000dps:
      *val = LSM6DSO_1000dps;
      break;
    case LSM6DSO_2000dps:
      *val = LSM6DSO_2000dps;
      break;
    default:
      *val = LSM6DSO_250dps;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Data_Rate_Set(LSM6DSO_ODR_G val)
{
  LSM6DSO_ODR_G odr_gy = val;
  LSM6DSO_Emb_FSM_Enable fsm_enable;
  LSM6DSO_FSM_ODR fsm_odr;
  LSM6DSO_Ctrl2_G reg;
  LSM6DSO_OpStatus op_status;

  /* Check the Finite State Machine data rate constraints */
  op_status = LSM6DSO_Reg_FSM_Enable_Get(&fsm_enable);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  if ( (fsm_enable.fsm_enable_a.fsm1_en  |
        fsm_enable.fsm_enable_a.fsm2_en  |
        fsm_enable.fsm_enable_a.fsm3_en  |
        fsm_enable.fsm_enable_a.fsm4_en  |
        fsm_enable.fsm_enable_a.fsm5_en  |
        fsm_enable.fsm_enable_a.fsm6_en  |
        fsm_enable.fsm_enable_a.fsm7_en  |
        fsm_enable.fsm_enable_a.fsm8_en  |
        fsm_enable.fsm_enable_b.fsm9_en  |
        fsm_enable.fsm_enable_b.fsm10_en |
        fsm_enable.fsm_enable_b.fsm11_en |
        fsm_enable.fsm_enable_b.fsm12_en |
        fsm_enable.fsm_enable_b.fsm13_en |
        fsm_enable.fsm_enable_b.fsm14_en |
        fsm_enable.fsm_enable_b.fsm15_en |
        fsm_enable.fsm_enable_b.fsm16_en ) == PROPERTY_ENABLE ){

    op_status = LSM6DSO_Reg_FSM_Data_Rate_Get(&fsm_odr);
    if (op_status < LSM6DSO_OK) {
      return op_status;
    }
    
    switch (fsm_odr) {
      case LSM6DSO_ODR_FSM_12Hz5:

        if (val == LSM6DSO_GY_ODR_OFF){
          odr_gy = LSM6DSO_GY_ODR_12Hz5;

        } else {
          odr_gy = val;
        }
        break;
      case LSM6DSO_ODR_FSM_26Hz:

        if (val == LSM6DSO_GY_ODR_OFF){
          odr_gy = LSM6DSO_GY_ODR_26Hz;

        } else if (val == LSM6DSO_GY_ODR_12Hz5){
          odr_gy = LSM6DSO_GY_ODR_26Hz;

        } else {
          odr_gy = val;
        }
        break;
      case LSM6DSO_ODR_FSM_52Hz:

        if (val == LSM6DSO_GY_ODR_OFF){
          odr_gy = LSM6DSO_GY_ODR_52Hz;

        } else if (val == LSM6DSO_GY_ODR_12Hz5){
          odr_gy = LSM6DSO_GY_ODR_52Hz;

        } else if (val == LSM6DSO_GY_ODR_26Hz){
          odr_gy = LSM6DSO_GY_ODR_52Hz;

        } else {
          odr_gy = val;
        }
        break;
      case LSM6DSO_ODR_FSM_104Hz:

        if (val == LSM6DSO_GY_ODR_OFF){
          odr_gy = LSM6DSO_GY_ODR_104Hz;

        } else if (val == LSM6DSO_GY_ODR_12Hz5){
          odr_gy = LSM6DSO_GY_ODR_104Hz;

        } else if (val == LSM6DSO_GY_ODR_26Hz){
          odr_gy = LSM6DSO_GY_ODR_104Hz;

        } else if (val == LSM6DSO_GY_ODR_52Hz){
          odr_gy = LSM6DSO_GY_ODR_104Hz;

        } else {
          odr_gy = val;
        }
        break;
      default:
        odr_gy = val;
        break;
    }
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL2_G, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.odr_g = (uint8_t) odr_gy;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL2_G, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Data_Rate_Get(LSM6DSO_ODR_G *val)
{
  LSM6DSO_Ctrl2_G reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL2_G, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.odr_g) {
    case LSM6DSO_GY_ODR_OFF:
      *val = LSM6DSO_GY_ODR_OFF;
      break;
    case LSM6DSO_GY_ODR_12Hz5:
      *val = LSM6DSO_GY_ODR_12Hz5;
      break;
    case LSM6DSO_GY_ODR_26Hz:
      *val = LSM6DSO_GY_ODR_26Hz;
      break;
    case LSM6DSO_GY_ODR_52Hz:
      *val = LSM6DSO_GY_ODR_52Hz;
      break;
    case LSM6DSO_GY_ODR_104Hz:
      *val = LSM6DSO_GY_ODR_104Hz;
      break;
    case LSM6DSO_GY_ODR_208Hz:
      *val = LSM6DSO_GY_ODR_208Hz;
      break;
    case LSM6DSO_GY_ODR_417Hz:
      *val = LSM6DSO_GY_ODR_417Hz;
      break;
    case LSM6DSO_GY_ODR_833Hz:
      *val = LSM6DSO_GY_ODR_833Hz;
      break;
    case LSM6DSO_GY_ODR_1667Hz:
      *val = LSM6DSO_GY_ODR_1667Hz;
      break;
    case LSM6DSO_GY_ODR_3333Hz:
      *val = LSM6DSO_GY_ODR_3333Hz;
      break;
    case LSM6DSO_GY_ODR_6667Hz:
      *val = LSM6DSO_GY_ODR_6667Hz;
      break;
    default:
      *val = LSM6DSO_GY_ODR_OFF;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Block_Data_Update_Set(uint8_t val)
{
  LSM6DSO_Ctrl3_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.bdu = val;
  
  return LSM6DSO_Write_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Block_Data_Update_Get(uint8_t *val)
{
  LSM6DSO_Ctrl3_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  *val = reg.bdu;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Offset_Weight_Set(LSM6DSO_Usr_Off_W val)
{
  LSM6DSO_Ctrl6_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL6_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.usr_off_w = (uint8_t)val;
  
  return LSM6DSO_Write_Reg(LSM6DSO_CTRL6_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Offset_Weight_Get(LSM6DSO_Usr_Off_W *val)
{
  LSM6DSO_Ctrl6_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL6_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.usr_off_w) {
    case LSM6DSO_LSb_1mg:
      *val = LSM6DSO_LSb_1mg;
      break;
    case LSM6DSO_LSb_16mg:
      *val = LSM6DSO_LSb_16mg;
      break;
    default:
      *val = LSM6DSO_LSb_1mg;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Power_Mode_Set(LSM6DSO_XL_HM_Mode val)
{
  LSM6DSO_Ctrl5_C ctrl5_c;
  LSM6DSO_Ctrl6_C ctrl6_c;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL5_C, (uint8_t*) &ctrl5_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  ctrl5_c.xl_ulp_en = ((uint8_t)val & 0x02U) >> 1;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL5_C, (uint8_t*) &ctrl5_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL6_C, (uint8_t*) &ctrl6_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  ctrl6_c.xl_hm_mode = (uint8_t)val & 0x01U;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL6_C, (uint8_t*) &ctrl6_c, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Power_Mode_Get(LSM6DSO_XL_HM_Mode *val)
{
  LSM6DSO_Ctrl5_C ctrl5_c;
  LSM6DSO_Ctrl6_C ctrl6_c;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL5_C, (uint8_t*) &ctrl5_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL6_C, (uint8_t*) &ctrl6_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch ( (ctrl5_c.xl_ulp_en << 1) | ctrl6_c.xl_hm_mode) {
    case LSM6DSO_HIGH_PERFORMANCE_MD:
      *val = LSM6DSO_HIGH_PERFORMANCE_MD;
      break;
    case LSM6DSO_LOW_NORMAL_POWER_MD:
      *val = LSM6DSO_LOW_NORMAL_POWER_MD;
      break;
    case LSM6DSO_ULTRA_LOW_POWER_MD:
      *val = LSM6DSO_ULTRA_LOW_POWER_MD;
      break;
    default:
      *val = LSM6DSO_HIGH_PERFORMANCE_MD;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Power_Mode_Set(LSM6DSO_G_HM_Mode val)
{
  LSM6DSO_Ctrl7_G reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL7_G, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.g_hm_mode = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL7_G, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Power_Mode_Get(LSM6DSO_G_HM_Mode *val)
{
  LSM6DSO_Ctrl7_G reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL7_G, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.g_hm_mode) {
    case LSM6DSO_GY_HIGH_PERFORMANCE:
      *val = LSM6DSO_GY_HIGH_PERFORMANCE;
      break;
    case LSM6DSO_GY_NORMAL:
      *val = LSM6DSO_GY_NORMAL;
      break;
    default:
      *val = LSM6DSO_GY_HIGH_PERFORMANCE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Status_Reg_Get(LSM6DSO_Status_Reg *val)
{
  return LSM6DSO_Read_Reg(LSM6DSO_STATUS_REG, (uint8_t*) val, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Flag_Data_Ready_Get(uint8_t *val)
{
  LSM6DSO_Status_Reg reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_STATUS_REG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.xlda;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Flag_Data_Ready_Get(uint8_t *val)
{
  LSM6DSO_Status_Reg reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_STATUS_REG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.gda;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Temp_Flag_Data_Ready_Get(uint8_t *val)
{
  LSM6DSO_Status_Reg reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_STATUS_REG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.tda;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_X_Set(uint8_t *buff)
{
  return LSM6DSO_Write_Reg(LSM6DSO_X_OFS_USR, buff, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_X_Get(uint8_t *buff)
{
  return LSM6DSO_Read_Reg(LSM6DSO_X_OFS_USR, buff, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_Y_Set(uint8_t *buff)
{
  return LSM6DSO_Write_Reg(LSM6DSO_Y_OFS_USR, buff, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_Y_Get(uint8_t *buff)
{
  return LSM6DSO_Read_Reg(LSM6DSO_Y_OFS_USR, buff, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_Z_Set(uint8_t *buff)
{
  return LSM6DSO_Write_Reg(LSM6DSO_Z_OFS_USR, buff, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_Z_Get(uint8_t *buff)
{
  return LSM6DSO_Read_Reg(LSM6DSO_Z_OFS_USR, buff, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_Set(uint8_t val)
{
  LSM6DSO_Ctrl7_G reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL7_G, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.usr_off_on_out = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL7_G, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_Get(uint8_t *val)
{
  LSM6DSO_Ctrl7_G reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL7_G, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.usr_off_on_out;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_TimeStamp_Rst()
{
  uint8_t rst_val = 0xAA;

  return LSM6DSO_Write_Reg(LSM6DSO_TIMESTAMP2, &rst_val, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_TimeStamp_Set(uint8_t val)
{
  LSM6DSO_Ctrl10_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL10_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.timestamp_en = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL10_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_TimeStamp_Get(uint8_t *val)
{
  LSM6DSO_Ctrl10_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL10_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.timestamp_en;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_TimeStamp_Raw_Get(uint8_t *buff)
{
  return LSM6DSO_Read_Reg(LSM6DSO_TIMESTAMP0, buff, 4);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Rounding_Mode_Set(LSM6DSO_Rounding val)
{
  LSM6DSO_Ctrl5_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL5_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.rounding = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL5_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Rounding_Mode_Get(LSM6DSO_Rounding *val)
{
  LSM6DSO_Ctrl5_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL5_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.rounding) {
    case LSM6DSO_NO_ROUND:
      *val = LSM6DSO_NO_ROUND;
      break;
    case LSM6DSO_ROUND_XL:
      *val = LSM6DSO_ROUND_XL;
      break;
    case LSM6DSO_ROUND_GY:
      *val = LSM6DSO_ROUND_GY;
      break;
    case LSM6DSO_ROUND_GY_XL:
      *val = LSM6DSO_ROUND_GY_XL;
      break;
    default:
      *val = LSM6DSO_NO_ROUND;
      break;
  }
  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Temperature_Raw_Get(uint8_t *buff)
{
  return LSM6DSO_Read_Reg(LSM6DSO_OUT_TEMP_L, buff, 2);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Angular_Rate_Raw_Get(uint8_t *buff)
{
  return LSM6DSO_Read_Reg(LSM6DSO_OUTX_L_G, buff, 6);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Acceleration_Raw_Get(uint8_t *buff)
{
  return LSM6DSO_Read_Reg(LSM6DSO_OUTX_L_A, buff, 6);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Out_Raw_Get(uint8_t *buff)
{
  return LSM6DSO_Read_Reg(LSM6DSO_FIFO_DATA_OUT_X_L, buff, 6);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Number_Of_Steps_Get(uint8_t *buff)
{
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_STEP_COUNTER_L, buff, 2);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Steps_Reset()
{
  LSM6DSO_EMB_Func_Src reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_SRC, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.pedo_rst_step = PROPERTY_ENABLE;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_EMB_FUNC_SRC, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_ODR_Calibration_Reg_Set(uint8_t val)
{
  LSM6DSO_Internal_Freq_Fine reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INTERNAL_FREQ_FINE, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.freq_fine = val;

  return LSM6DSO_Write_Reg(LSM6DSO_INTERNAL_FREQ_FINE, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_ODR_Calibration_Reg_Get(uint8_t *val)
{
  LSM6DSO_Internal_Freq_Fine reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INTERNAL_FREQ_FINE, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  *val = reg.freq_fine;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_Reg_Access val)
{
  LSM6DSO_Func_Cfg_Access reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FUNC_CFG_ACCESS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.reg_access = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_FUNC_CFG_ACCESS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mem_Bank_Get(LSM6DSO_Reg_Access *val)
{
  LSM6DSO_Func_Cfg_Access reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FUNC_CFG_ACCESS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  switch (reg.reg_access) {
    case LSM6DSO_USER_BANK:
      *val = LSM6DSO_USER_BANK;
      break;
    case LSM6DSO_SENSOR_HUB_BANK:
      *val = LSM6DSO_SENSOR_HUB_BANK;
      break;
    case LSM6DSO_EMBEDDED_FUNC_BANK:
      *val = LSM6DSO_EMBEDDED_FUNC_BANK;
      break;
    default:
      *val = LSM6DSO_USER_BANK;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Ln_Page_Write_Byte(uint16_t address, uint8_t *val)
{
  LSM6DSO_Page_RW page_rw;
  LSM6DSO_Page_Sel page_sel;
  LSM6DSO_Page_Address page_address;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  page_rw.page_rw = 0x02; /* page_write enable */

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_SEL, (uint8_t*) &page_sel, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  page_sel.page_sel = ((uint8_t)(address >> 8) & 0x0FU);
  page_sel.not_used_01 = 1;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_SEL, (uint8_t*) &page_sel, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  page_address.page_addr = (uint8_t)address & 0xFFU;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_ADDRESS, (uint8_t*)&page_address, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_VALUE, val, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  page_rw.page_rw = 0x00; /* page_write disable */

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Ln_Page_Write(uint16_t address, uint8_t *buf, uint8_t len)
{
  LSM6DSO_Page_RW page_rw;
  LSM6DSO_Page_Sel page_sel;
  LSM6DSO_Page_Address  page_address;
  uint16_t addr_pointed;
  LSM6DSO_OpStatus op_status;
  uint8_t i ;

  addr_pointed = address;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  page_rw.page_rw = 0x02; /* page_write enable*/

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_SEL, (uint8_t*) &page_sel, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  page_sel.page_sel = ((uint8_t)(addr_pointed >> 8) & 0x0FU);
  page_sel.not_used_01 = 1;
    
  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_SEL, (uint8_t*) &page_sel, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  page_address.page_addr = (uint8_t)(addr_pointed & 0x00FFU);
  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_ADDRESS, (uint8_t*)&page_address, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  for (i = 0; i < len; i++)
  {
    op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_VALUE, &buf[i], 1);
    if (op_status < LSM6DSO_OK) {
      return op_status;
    }

    addr_pointed++;

      /* Check if page wrap */
    if ( (addr_pointed % 0x0100U) == 0x00U )
    {
      op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_SEL, (uint8_t*)&page_sel, 1);
      if (op_status < LSM6DSO_OK) {
        return op_status;
      }

      page_sel.page_sel = ((uint8_t)(addr_pointed >> 8) & 0x0FU);
      page_sel.not_used_01 = 1;

      op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_SEL, (uint8_t*)&page_sel, 1);
      if (op_status < LSM6DSO_OK) {
        return op_status;
      }
    }
  }

  page_sel.page_sel = 0;
  page_sel.not_used_01 = 1;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_SEL, (uint8_t*) &page_sel, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  page_rw.page_rw = 0x00; /* page_write disable */

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Ln_Page_Read_Byte(uint16_t address, uint8_t *val)
{
  LSM6DSO_Page_RW page_rw;
  LSM6DSO_Page_Sel page_sel;
  LSM6DSO_Page_Address page_address;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  page_rw.page_rw = 0x01; /* page_read enable*/

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_SEL, (uint8_t*) &page_sel, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  page_sel.page_sel = ((uint8_t)(address >> 8) & 0x0FU);
  page_sel.not_used_01 = 1;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_SEL, (uint8_t*) &page_sel, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  page_address.page_addr = (uint8_t)address & 0x00FFU;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_ADDRESS, (uint8_t*)&page_address, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_VALUE, val, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  page_rw.page_rw = 0x00; /* page_read disable */

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Ln_Page_Read(uint16_t address, uint8_t *buf, uint8_t len)
{
  LSM6DSO_Page_RW page_rw;
  LSM6DSO_Page_Sel page_sel;
  LSM6DSO_Page_Address page_address;
  uint16_t addr_pointed = address;;
  LSM6DSO_OpStatus op_status;
  uint8_t i ;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  page_rw.page_rw = 0x01; /* page_read enable*/

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_SEL, (uint8_t*) &page_sel, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  page_sel.page_sel = ((uint8_t)(addr_pointed >> 8) & 0x0FU);
  page_sel.not_used_01 = 1;
    
  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_SEL, (uint8_t*) &page_sel, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  page_address.page_addr = (uint8_t)(addr_pointed & 0x00FFU);
  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_ADDRESS, (uint8_t*)&page_address, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  for (i = 0; i < len; i++)
  {
    op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_VALUE, &buf[i], 1);
    if (op_status < LSM6DSO_OK) {
      return op_status;
    }

    addr_pointed++;

      /* Check if page wrap */
    if ( (addr_pointed % 0x0100U) == 0x00U )
    {
      op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_SEL, (uint8_t*)&page_sel, 1);
      if (op_status < LSM6DSO_OK) {
        return op_status;
      }

      page_sel.page_sel = ((uint8_t)(addr_pointed >> 8) & 0x0FU);
      page_sel.not_used_01 = 1;

      op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_SEL, (uint8_t*)&page_sel, 1);
      if (op_status < LSM6DSO_OK) {
        return op_status;
      }
    }
  }

  page_sel.page_sel = 0;
  page_sel.not_used_01 = 1;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_SEL, (uint8_t*) &page_sel, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  page_rw.page_rw = 0x00; /* page_read disable */

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Data_Ready_Mode_Set(LSM6DSO_DataReady_Pulsed val)
{
  LSM6DSO_Counter_BDR_Reg1 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_COUNTER_BDR_REG1, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.dataready_pulsed = (uint8_t)val;
  
  return LSM6DSO_Write_Reg(LSM6DSO_COUNTER_BDR_REG1, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Data_Ready_Mode_Get(LSM6DSO_DataReady_Pulsed *val)
{
  LSM6DSO_Counter_BDR_Reg1 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_COUNTER_BDR_REG1, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.dataready_pulsed) {
    case LSM6DSO_DRDY_LATCHED:
      *val = LSM6DSO_DRDY_LATCHED;
      break;
    case LSM6DSO_DRDY_PULSED:
      *val = LSM6DSO_DRDY_PULSED;
      break;
    default:
      *val = LSM6DSO_DRDY_LATCHED;
      break;
  }
  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Device_ID_Get(uint8_t *buff)
{
  return LSM6DSO_Read_Reg(LSM6DSO_WHO_AM_I, buff, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Reset_Set(uint8_t val)
{
  LSM6DSO_Ctrl3_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.sw_reset = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Reset_Get(uint8_t *val)
{
  LSM6DSO_Ctrl3_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.sw_reset;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Auto_Increment_Set(uint8_t val)
{
  LSM6DSO_Ctrl3_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.if_inc = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Auto_Increment_Get(uint8_t *val)
{
  LSM6DSO_Ctrl3_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.if_inc;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Boot_Set(uint8_t val)
{
  LSM6DSO_Ctrl3_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.boot = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Boot_Get(uint8_t *val)
{
  LSM6DSO_Ctrl3_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.boot;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Self_Test_Set(LSM6DSO_ST_XL val)
{
  LSM6DSO_Ctrl5_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL5_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.st_xl = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL5_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Self_Test_Get(LSM6DSO_ST_XL *val)
{
  LSM6DSO_Ctrl5_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL5_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.st_xl) {
    case LSM6DSO_XL_ST_DISABLE:
      *val = LSM6DSO_XL_ST_DISABLE;
      break;
    case LSM6DSO_XL_ST_POSITIVE:
      *val = LSM6DSO_XL_ST_POSITIVE;
      break;
    case LSM6DSO_XL_ST_NEGATIVE:
      *val = LSM6DSO_XL_ST_NEGATIVE;
      break;
    default:
      *val = LSM6DSO_XL_ST_DISABLE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Self_Test_Set(LSM6DSO_ST_G val)
{
  LSM6DSO_Ctrl5_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL5_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.st_g = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL5_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Self_Test_Get(LSM6DSO_ST_G *val)
{
  LSM6DSO_Ctrl5_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL5_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.st_g) {
    case LSM6DSO_GY_ST_DISABLE:
      *val = LSM6DSO_GY_ST_DISABLE;
      break;
    case LSM6DSO_GY_ST_POSITIVE:
      *val = LSM6DSO_GY_ST_POSITIVE;
      break;
    case LSM6DSO_GY_ST_NEGATIVE:
      *val = LSM6DSO_GY_ST_NEGATIVE;
      break;
    default:
      *val = LSM6DSO_GY_ST_DISABLE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Filter_LP2_Set(uint8_t val)
{
  LSM6DSO_Ctrl1_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.lpf2_xl_en = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL1_XL, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Filter_LP2_Get(uint8_t *val)
{
  LSM6DSO_Ctrl1_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.lpf2_xl_en;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Filter_LP1_Set(uint8_t val)
{
  LSM6DSO_Ctrl4_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.lpf1_sel_g = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Filter_LP1_Get(uint8_t *val)
{
  LSM6DSO_Ctrl4_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.lpf1_sel_g;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Filter_Settling_Mask_Set(uint8_t val)
{
  LSM6DSO_Ctrl4_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.drdy_mask = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Filter_Settling_Mask_Get(uint8_t *val)
{
  LSM6DSO_Ctrl4_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.drdy_mask;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_LP1_BandWidth_Set(LSM6DSO_fType_t val)
{
  LSM6DSO_Ctrl6_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL6_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.ftype = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL6_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_LP1_BandWidth_Get(LSM6DSO_fType_t *val)
{
  LSM6DSO_Ctrl6_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL6_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.ftype) {
    case LSM6DSO_ULTRA_LIGHT:
      *val = LSM6DSO_ULTRA_LIGHT;
      break;
    case LSM6DSO_VERY_LIGHT:
      *val = LSM6DSO_VERY_LIGHT;
      break;
    case LSM6DSO_LIGHT:
      *val = LSM6DSO_LIGHT;
      break;
    case LSM6DSO_MEDIUM:
      *val = LSM6DSO_MEDIUM;
      break;
    case LSM6DSO_STRONG:
      *val = LSM6DSO_STRONG;
      break;
    case LSM6DSO_VERY_STRONG:
      *val = LSM6DSO_VERY_STRONG;
      break;
    case LSM6DSO_AGGRESSIVE:
      *val = LSM6DSO_AGGRESSIVE;
      break;
    case LSM6DSO_XTREME:
      *val = LSM6DSO_XTREME;
      break;
    default:
      *val = LSM6DSO_ULTRA_LIGHT;
      break;
  }
  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_LP2_On_6D_Set(uint8_t val)
{
  LSM6DSO_Ctrl8_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL8_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.low_pass_on_6d = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL8_XL, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_LP2_On_6D_Get(uint8_t *val)
{
  LSM6DSO_Ctrl8_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL8_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.low_pass_on_6d;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_HP_Path_On_Out_Set(LSM6DSO_HP_Slope_XL_En val)
{
  LSM6DSO_Ctrl8_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL8_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.hp_slope_xl_en = ((uint8_t)val & 0x10U) >> 4;
  reg.hp_ref_mode_xl = ((uint8_t)val & 0x20U) >> 5;
  reg.hpcf_xl = (uint8_t)val & 0x07U;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL8_XL, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_HP_Path_On_Out_Get(LSM6DSO_HP_Slope_XL_En *val)
{
  LSM6DSO_Ctrl8_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL8_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch ((reg.hp_ref_mode_xl << 5) | (reg.hp_slope_xl_en << 4) | reg.hpcf_xl)
  {
    case LSM6DSO_HP_PATH_DISABLE_ON_OUT:
      *val = LSM6DSO_HP_PATH_DISABLE_ON_OUT;
      break;
    case LSM6DSO_SLOPE_ODR_DIV_4:
      *val = LSM6DSO_SLOPE_ODR_DIV_4;
      break;
    case LSM6DSO_HP_ODR_DIV_10:
      *val = LSM6DSO_HP_ODR_DIV_10;
      break;
    case LSM6DSO_HP_ODR_DIV_20:
      *val = LSM6DSO_HP_ODR_DIV_20;
      break;
    case LSM6DSO_HP_ODR_DIV_45:
      *val = LSM6DSO_HP_ODR_DIV_45;
      break;
    case LSM6DSO_HP_ODR_DIV_100:
      *val = LSM6DSO_HP_ODR_DIV_100;
      break;
    case LSM6DSO_HP_ODR_DIV_200:
      *val = LSM6DSO_HP_ODR_DIV_200;
      break;
    case LSM6DSO_HP_ODR_DIV_400:
      *val = LSM6DSO_HP_ODR_DIV_400;
      break;
    case LSM6DSO_HP_ODR_DIV_800:
      *val = LSM6DSO_HP_ODR_DIV_800;
      break;
    case LSM6DSO_HP_REF_MD_ODR_DIV_10:
      *val = LSM6DSO_HP_REF_MD_ODR_DIV_10;
      break;
    case LSM6DSO_HP_REF_MD_ODR_DIV_20:
      *val = LSM6DSO_HP_REF_MD_ODR_DIV_20;
      break;
    case LSM6DSO_HP_REF_MD_ODR_DIV_45:
      *val = LSM6DSO_HP_REF_MD_ODR_DIV_45;
      break;
    case LSM6DSO_HP_REF_MD_ODR_DIV_100:
      *val = LSM6DSO_HP_REF_MD_ODR_DIV_100;
      break;
    case LSM6DSO_HP_REF_MD_ODR_DIV_200:
      *val = LSM6DSO_HP_REF_MD_ODR_DIV_200;
      break;
    case LSM6DSO_HP_REF_MD_ODR_DIV_400:
      *val = LSM6DSO_HP_REF_MD_ODR_DIV_400;
      break;
    case LSM6DSO_HP_REF_MD_ODR_DIV_800:
      *val = LSM6DSO_HP_REF_MD_ODR_DIV_800;
      break;
    case LSM6DSO_LP_ODR_DIV_10:
      *val = LSM6DSO_LP_ODR_DIV_10;
      break;
    case LSM6DSO_LP_ODR_DIV_20:
      *val = LSM6DSO_LP_ODR_DIV_20;
      break;
    case LSM6DSO_LP_ODR_DIV_45:
      *val = LSM6DSO_LP_ODR_DIV_45;
      break;
    case LSM6DSO_LP_ODR_DIV_100:
      *val = LSM6DSO_LP_ODR_DIV_100;
      break;
    case LSM6DSO_LP_ODR_DIV_200:
      *val = LSM6DSO_LP_ODR_DIV_200;
      break;
    case LSM6DSO_LP_ODR_DIV_400:
      *val = LSM6DSO_LP_ODR_DIV_400;
      break;
    case LSM6DSO_LP_ODR_DIV_800:
      *val = LSM6DSO_LP_ODR_DIV_800;
      break;
    default:
      *val = LSM6DSO_HP_PATH_DISABLE_ON_OUT;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Fast_Settling_Set(uint8_t val)
{
  LSM6DSO_Ctrl8_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL8_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.fastsettl_mode_xl = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL8_XL, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Fast_Settling_Get(uint8_t *val)
{
  LSM6DSO_Ctrl8_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL8_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.fastsettl_mode_xl;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_HP_Path_Internal_Set(  LSM6DSO_Slope_Fds val)
{
  LSM6DSO_Tap_Cfg0 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.slope_fds = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_HP_Path_Internal_Get(LSM6DSO_Slope_Fds *val)
{
  LSM6DSO_Tap_Cfg0 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.slope_fds) {
    case LSM6DSO_USE_SLOPE:
      *val = LSM6DSO_USE_SLOPE;
      break;
    case LSM6DSO_USE_HPF:
      *val = LSM6DSO_USE_HPF;
      break;
    default:
      *val = LSM6DSO_USE_SLOPE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_HP_Path_Internal_Set(LSM6DSO_HPM_G val)
{
  LSM6DSO_Ctrl7_G reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL7_G, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.hp_en_g = ((uint8_t)val & 0x80U) >> 7;
  reg.hpm_g = (uint8_t)val & 0x03U;
  
  return LSM6DSO_Write_Reg(LSM6DSO_CTRL7_G, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_HP_Path_Internal_Get(LSM6DSO_HPM_G *val)
{
  LSM6DSO_Ctrl7_G reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL7_G, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch ((reg.hp_en_g << 7) + reg.hpm_g) {
    case LSM6DSO_HP_FILTER_NONE:
      *val = LSM6DSO_HP_FILTER_NONE;
      break;
    case LSM6DSO_HP_FILTER_16mHz:
      *val = LSM6DSO_HP_FILTER_16mHz;
      break;
    case LSM6DSO_HP_FILTER_65mHz:
      *val = LSM6DSO_HP_FILTER_65mHz;
      break;
    case LSM6DSO_HP_FILTER_260mHz:
      *val = LSM6DSO_HP_FILTER_260mHz;
      break;
    case LSM6DSO_HP_FILTER_1Hz04:
      *val = LSM6DSO_HP_FILTER_1Hz04;
      break;
    default:
      *val = LSM6DSO_HP_FILTER_NONE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_SDO_OCS_Mode_Set(LSM6DSO_Ois_PU_Dis val)
{
  LSM6DSO_Pin_Ctrl reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_PIN_CTRL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.ois_pu_dis = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_PIN_CTRL, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_SDO_OCS_Mode_Get(LSM6DSO_Ois_PU_Dis *val)
{
  LSM6DSO_Pin_Ctrl reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_PIN_CTRL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.ois_pu_dis) {
    case LSM6DSO_AUX_PULL_UP_DISC:
      *val = LSM6DSO_AUX_PULL_UP_DISC;
      break;
    case LSM6DSO_AUX_PULL_UP_CONNECT:
      *val = LSM6DSO_AUX_PULL_UP_CONNECT;
      break;
    default:
      *val = LSM6DSO_AUX_PULL_UP_DISC;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Pwr_On_Ctrl_Set(LSM6DSO_Ois_On val)
{
  LSM6DSO_Ctrl7_G reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL7_G, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.ois_on_en = (uint8_t)val & 0x01U;
  reg.ois_on = (uint8_t)val & 0x01U;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL7_G, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Pwr_On_Ctrl_Get(LSM6DSO_Ois_On *val)
{
  LSM6DSO_Ctrl7_G reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL7_G, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.ois_on) {
    case LSM6DSO_AUX_ON:
      *val = LSM6DSO_AUX_ON;
      break;
    case LSM6DSO_AUX_ON_BY_AUX_INTERFACE:
      *val = LSM6DSO_AUX_ON_BY_AUX_INTERFACE;
      break;
    default:
      *val = LSM6DSO_AUX_ON;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_FS_Mode_Set(LSM6DSO_XL_FS_Mode val)
{
  LSM6DSO_Ctrl8_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL8_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
\
  reg.xl_fs_mode = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL8_XL, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_FS_Mode_Get(LSM6DSO_XL_FS_Mode *val)
{
  LSM6DSO_Ctrl8_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL8_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.xl_fs_mode) {
    case LSM6DSO_USE_SAME_XL_FS:
      *val = LSM6DSO_USE_SAME_XL_FS;
      break;
    case LSM6DSO_USE_DIFFERENT_XL_FS:
      *val = LSM6DSO_USE_DIFFERENT_XL_FS;
      break;
    default:
      *val = LSM6DSO_USE_SAME_XL_FS;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Status_Get(LSM6DSO_Status_SpiAux *val)
{
  return LSM6DSO_Read_Reg(LSM6DSO_STATUS_SPIAUX, (uint8_t*) val, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_Flag_Data_Ready_Get(uint8_t *val)
{
  LSM6DSO_Status_SpiAux reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_STATUS_SPIAUX, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.xlda;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Flag_Data_Ready_Get(uint8_t *val)
{
  LSM6DSO_Status_SpiAux reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_STATUS_SPIAUX, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.gda;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Flag_Settling_Get(uint8_t *val)
{
  LSM6DSO_Status_SpiAux reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_STATUS_SPIAUX, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.gyro_settling;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_Self_Test_Set(LSM6DSO_St_XL_Ois val)
{
  LSM6DSO_Int_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.st_xl_ois = (uint8_t)val;
  
  return LSM6DSO_Write_Reg(LSM6DSO_INT_OIS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_Self_Test_Get(LSM6DSO_St_XL_Ois *val)
{
  LSM6DSO_Int_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.st_xl_ois) {
    case LSM6DSO_AUX_XL_DISABLE:
      *val = LSM6DSO_AUX_XL_DISABLE;
      break;
    case LSM6DSO_AUX_XL_POS:
      *val = LSM6DSO_AUX_XL_POS;
      break;
    case LSM6DSO_AUX_XL_NEG:
      *val = LSM6DSO_AUX_XL_NEG;
      break;
    default:
      *val = LSM6DSO_AUX_XL_DISABLE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Den_Polarity_Set(LSM6DSO_Den_LH_Ois val)
{
  LSM6DSO_Int_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.den_lh_ois = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_INT_OIS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Den_Polarity_Get(LSM6DSO_Den_LH_Ois *val)
{
  LSM6DSO_Int_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.den_lh_ois) {
    case LSM6DSO_AUX_DEN_ACTIVE_LOW:
      *val = LSM6DSO_AUX_DEN_ACTIVE_LOW;
      break;
    case LSM6DSO_AUX_DEN_ACTIVE_HIGH:
      *val = LSM6DSO_AUX_DEN_ACTIVE_HIGH;
      break;
    default:
      *val = LSM6DSO_AUX_DEN_ACTIVE_LOW;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Den_Mode_Set(LSM6DSO_Lvl2_Ois val)
{
  LSM6DSO_Ctrl1_Ois ctrl1_ois;
  LSM6DSO_Int_Ois int_ois;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT_OIS, (uint8_t*) &int_ois, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  int_ois.lvl2_ois = (uint8_t)val & 0x01U;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_INT_OIS, (uint8_t*) &int_ois, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_OIS, (uint8_t*) &ctrl1_ois, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  ctrl1_ois.lvl1_ois = ((uint8_t)val & 0x02U) >> 1;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL1_OIS, (uint8_t*) &ctrl1_ois, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Den_Mode_Get(LSM6DSO_Lvl2_Ois *val)
{
  LSM6DSO_Ctrl1_Ois ctrl1_ois;
  LSM6DSO_Int_Ois int_ois;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT_OIS, (uint8_t*) &int_ois, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_OIS, (uint8_t*) &ctrl1_ois, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch ((ctrl1_ois.lvl1_ois << 1) + int_ois.lvl2_ois) {
    case LSM6DSO_AUX_DEN_DISABLE:
      *val = LSM6DSO_AUX_DEN_DISABLE;
      break;
    case LSM6DSO_AUX_DEN_LEVEL_LATCH:
      *val = LSM6DSO_AUX_DEN_LEVEL_LATCH;
      break;
    case LSM6DSO_AUX_DEN_LEVEL_TRIG:
      *val = LSM6DSO_AUX_DEN_LEVEL_TRIG;
      break;
    default:
      *val = LSM6DSO_AUX_DEN_DISABLE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_dRdy_On_Int2_Set(uint8_t val)
{
  LSM6DSO_Int_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.int2_drdy_ois = val;
  
  return LSM6DSO_Write_Reg(LSM6DSO_INT_OIS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_dRdy_On_Int2_Get(uint8_t *val)
{
  LSM6DSO_Int_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.int2_drdy_ois;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Mode_Set(LSM6DSO_Ois_En_SPI2 val)
{
  LSM6DSO_Ctrl1_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.ois_en_spi2 = (uint8_t)val & 0x01U;
  reg.mode4_en = ((uint8_t)val & 0x02U) >> 1;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL1_OIS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Mode_Get(LSM6DSO_Ois_En_SPI2 *val)
{
  LSM6DSO_Ctrl1_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch ((reg.mode4_en << 1) | reg.ois_en_spi2) {
    case LSM6DSO_AUX_DISABLE:
      *val = LSM6DSO_AUX_DISABLE;
      break;
    case LSM6DSO_MODE_3_GY:
      *val = LSM6DSO_MODE_3_GY;
      break;
    case LSM6DSO_MODE_4_GY_XL:
      *val = LSM6DSO_MODE_4_GY_XL;
      break;
    default:
      *val = LSM6DSO_AUX_DISABLE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Full_Scale_Set(LSM6DSO_FS_G_Ois val)
{
  LSM6DSO_Ctrl1_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.fs_g_ois = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL1_OIS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Full_Scale_Get(LSM6DSO_FS_G_Ois *val)
{
  LSM6DSO_Ctrl1_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.fs_g_ois) {
    case LSM6DSO_250dps_AUX:
      *val = LSM6DSO_250dps_AUX;
      break;
    case LSM6DSO_125dps_AUX:
      *val = LSM6DSO_125dps_AUX;
      break;
    case LSM6DSO_500dps_AUX:
      *val = LSM6DSO_500dps_AUX;
      break;
    case LSM6DSO_1000dps_AUX:
      *val = LSM6DSO_1000dps_AUX;
      break;
    case LSM6DSO_2000dps_AUX:
      *val = LSM6DSO_2000dps_AUX;
      break;
    default:
      *val = LSM6DSO_250dps_AUX;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_SPI_Mode_Set(LSM6DSO_Sim_Ois val)
{
  LSM6DSO_Ctrl1_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.sim_ois = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL1_OIS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_SPI_Mode_Get(LSM6DSO_Sim_Ois *val)
{
  LSM6DSO_Ctrl1_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.sim_ois) {
    case LSM6DSO_AUX_SPI_4_WIRE:
      *val = LSM6DSO_AUX_SPI_4_WIRE;
      break;
    case LSM6DSO_AUX_SPI_3_WIRE:
      *val = LSM6DSO_AUX_SPI_3_WIRE;
      break;
    default:
      *val = LSM6DSO_AUX_SPI_4_WIRE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_LP1_BandWidth_Set(LSM6DSO_fType_Ois val)
{
  LSM6DSO_Ctrl2_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL2_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.ftype_ois = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL2_OIS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_LP1_BandWidth_Get(LSM6DSO_fType_Ois *val)
{
  LSM6DSO_Ctrl2_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL2_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.ftype_ois) {
    case LSM6DSO_351Hz39:
      *val = LSM6DSO_351Hz39;
      break;
    case LSM6DSO_236Hz63:
      *val = LSM6DSO_236Hz63;
      break;
    case LSM6DSO_172Hz70:
      *val = LSM6DSO_172Hz70;
      break;
    case LSM6DSO_937Hz91:
      *val = LSM6DSO_937Hz91;
      break;
    default:
      *val = LSM6DSO_351Hz39;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_HP_BandWidth_Set(LSM6DSO_HPM_Ois val)
{
  LSM6DSO_Ctrl2_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL2_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.hpm_ois = (uint8_t)val & 0x03U;
  reg.hp_en_ois = ((uint8_t)val & 0x10U) >> 4;
  
  return LSM6DSO_Write_Reg(LSM6DSO_CTRL2_OIS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_HP_BandWidth_Get(LSM6DSO_HPM_Ois *val)
{
  LSM6DSO_Ctrl2_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL2_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch ((reg.hp_en_ois << 4) | reg.hpm_ois) {
    case LSM6DSO_AUX_HP_DISABLE:
      *val = LSM6DSO_AUX_HP_DISABLE;
      break;
    case LSM6DSO_AUX_HP_Hz016:
      *val = LSM6DSO_AUX_HP_Hz016;
      break;
    case LSM6DSO_AUX_HP_Hz065:
      *val = LSM6DSO_AUX_HP_Hz065;
      break;
    case LSM6DSO_AUX_HP_Hz260:
      *val = LSM6DSO_AUX_HP_Hz260;
      break;
    case LSM6DSO_AUX_HP_1Hz040:
      *val = LSM6DSO_AUX_HP_1Hz040;
      break;
    default:
      *val = LSM6DSO_AUX_HP_DISABLE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Clamp_Set(LSM6DSO_ST_Ois_ClampDis val)
{
  LSM6DSO_Ctrl3_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.st_ois_clampdis = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL3_OIS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Clamp_Get(LSM6DSO_ST_Ois_ClampDis *val)
{
  LSM6DSO_Ctrl3_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.st_ois_clampdis) {
    case LSM6DSO_ENABLE_CLAMP:
      *val = LSM6DSO_ENABLE_CLAMP;
      break;
    case LSM6DSO_DISABLE_CLAMP:
      *val = LSM6DSO_DISABLE_CLAMP;
      break;
    default:
      *val = LSM6DSO_ENABLE_CLAMP;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Self_Test_Set(LSM6DSO_ST_Ois val)
{
  LSM6DSO_Ctrl3_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.st_ois = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL3_OIS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Self_Test_Get(LSM6DSO_ST_Ois *val)
{
  LSM6DSO_Ctrl3_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.st_ois) {
    case LSM6DSO_AUX_GY_DISABLE:
      *val = LSM6DSO_AUX_GY_DISABLE;
      break;
    case LSM6DSO_AUX_GY_POS:
      *val = LSM6DSO_AUX_GY_POS;
      break;
    case LSM6DSO_AUX_GY_NEG:
      *val = LSM6DSO_AUX_GY_NEG;
      break;
    default:
      *val = LSM6DSO_AUX_GY_DISABLE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_BandWidth_Set(LSM6DSO_Filter_XL_Conf_Ois val)
{
  LSM6DSO_Ctrl3_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.filter_xl_conf_ois = (uint8_t)val;
  
  return LSM6DSO_Write_Reg(LSM6DSO_CTRL3_OIS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_BandWidth_Get(LSM6DSO_Filter_XL_Conf_Ois *val)
{
  LSM6DSO_Ctrl3_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.filter_xl_conf_ois) {
    case LSM6DSO_289Hz:
      *val = LSM6DSO_289Hz;
      break;
    case LSM6DSO_258Hz:
      *val = LSM6DSO_258Hz;
      break;
    case LSM6DSO_120Hz:
      *val = LSM6DSO_120Hz;
      break;
    case LSM6DSO_65Hz2:
      *val = LSM6DSO_65Hz2;
      break;
    case LSM6DSO_33Hz2:
      *val = LSM6DSO_33Hz2;
      break;
    case LSM6DSO_16Hz6:
      *val = LSM6DSO_16Hz6;
      break;
    case LSM6DSO_8Hz30:
      *val = LSM6DSO_8Hz30;
      break;
    case LSM6DSO_4Hz15:
      *val = LSM6DSO_4Hz15;
      break;
    default:
      *val = LSM6DSO_289Hz;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_Full_Scale_Set(LSM6DSO_FS_XL_Ois val)
{
  LSM6DSO_Ctrl3_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.fs_xl_ois = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL3_OIS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_Full_Scale_Get(LSM6DSO_FS_XL_Ois *val)
{
  LSM6DSO_Ctrl3_Ois reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_OIS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.fs_xl_ois) {
    case LSM6DSO_AUX_2g:
      *val = LSM6DSO_AUX_2g;
      break;
    case LSM6DSO_AUX_16g:
      *val = LSM6DSO_AUX_16g;
      break;
    case LSM6DSO_AUX_4g:
      *val = LSM6DSO_AUX_4g;
      break;
    case LSM6DSO_AUX_8g:
      *val = LSM6DSO_AUX_8g;
      break;
    default:
      *val = LSM6DSO_AUX_2g;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_SDO_SA0_Mode_Set(LSM6DSO_SDO_PU_En val)
{
  LSM6DSO_Pin_Ctrl reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_PIN_CTRL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.sdo_pu_en = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_PIN_CTRL, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_SDO_SA0_Mode_Get(LSM6DSO_SDO_PU_En *val)
{
  LSM6DSO_Pin_Ctrl reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_PIN_CTRL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.sdo_pu_en) {
    case LSM6DSO_PULL_UP_DISC:
      *val = LSM6DSO_PULL_UP_DISC;
      break;
    case LSM6DSO_PULL_UP_CONNECT:
      *val = LSM6DSO_PULL_UP_CONNECT;
      break;
    default:
      *val = LSM6DSO_PULL_UP_DISC;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_SPI_Mode_Set(LSM6DSO_Sim val)
{
  LSM6DSO_Ctrl3_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.sim = (uint8_t)val;
  
  return LSM6DSO_Write_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_SPI_Mode_Get(LSM6DSO_Sim *val)
{
  LSM6DSO_Ctrl3_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.sim) {
    case LSM6DSO_SPI_4_WIRE:
      *val = LSM6DSO_SPI_4_WIRE;
      break;
    case LSM6DSO_SPI_3_WIRE:
      *val = LSM6DSO_SPI_3_WIRE;
      break;
    default:
      *val = LSM6DSO_SPI_4_WIRE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_I2C_Interface_Set(LSM6DSO_I2C_Disable val)
{
  LSM6DSO_Ctrl4_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.i2c_disable = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_I2C_Interface_Get(LSM6DSO_I2C_Disable *val)
{
  LSM6DSO_Ctrl4_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.i2c_disable) {
    case LSM6DSO_I2C_ENABLE:
      *val = LSM6DSO_I2C_ENABLE;
      break;
    case LSM6DSO_I2C_DISABLE:
      *val = LSM6DSO_I2C_DISABLE;
      break;
    default:
      *val = LSM6DSO_I2C_ENABLE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_I3C_Disable_Set(LSM6DSO_I3C_Disable val)
{
  LSM6DSO_I3C_Bus_Avb i3c_bus_avb;
  LSM6DSO_Ctrl9_XL ctrl9_xl;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&ctrl9_xl, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  ctrl9_xl.i3c_disable = ((uint8_t)val & 0x80U) >> 7;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&ctrl9_xl, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_I3C_BUS_AVB, (uint8_t*)&i3c_bus_avb, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  i3c_bus_avb.i3c_bus_avb_sel = (uint8_t)val & 0x03U;

  return LSM6DSO_Write_Reg(LSM6DSO_I3C_BUS_AVB, (uint8_t*)&i3c_bus_avb, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_I3C_Disable_Get(LSM6DSO_I3C_Disable *val)
{
  LSM6DSO_Ctrl9_XL ctrl9_xl;
  LSM6DSO_I3C_Bus_Avb i3c_bus_avb;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&ctrl9_xl, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_I3C_BUS_AVB, (uint8_t*)&i3c_bus_avb, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch ((ctrl9_xl.i3c_disable << 7) | i3c_bus_avb.i3c_bus_avb_sel) {
    case LSM6DSO_I3C_DISABLE:
      *val = LSM6DSO_I3C_DISABLE;
      break;
    case LSM6DSO_I3C_ENABLE_T_50us:
      *val = LSM6DSO_I3C_ENABLE_T_50us;
      break;
    case LSM6DSO_I3C_ENABLE_T_2us:
      *val = LSM6DSO_I3C_ENABLE_T_2us;
      break;
    case LSM6DSO_I3C_ENABLE_T_1ms:
      *val = LSM6DSO_I3C_ENABLE_T_1ms;
      break;
    case LSM6DSO_I3C_ENABLE_T_25ms:
      *val = LSM6DSO_I3C_ENABLE_T_25ms;
      break;
    default:
      *val = LSM6DSO_I3C_DISABLE;
      break;
  }
  
  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Int1_Mode_Set(LSM6DSO_Int1_PD_En val)
{
  LSM6DSO_I3C_Bus_Avb reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_I3C_BUS_AVB, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.pd_dis_int1 = (uint8_t)val;
  
  return LSM6DSO_Write_Reg(LSM6DSO_I3C_BUS_AVB, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Int1_Mode_Get(LSM6DSO_Int1_PD_En *val)
{
  LSM6DSO_I3C_Bus_Avb reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_I3C_BUS_AVB, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.pd_dis_int1) {
    case LSM6DSO_PULL_DOWN_DISC:
      *val = LSM6DSO_PULL_DOWN_DISC;
      break;
    case LSM6DSO_PULL_DOWN_CONNECT:
      *val = LSM6DSO_PULL_DOWN_CONNECT;
      break;
    default:
      *val = LSM6DSO_PULL_DOWN_DISC;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Mode_Set(LSM6DSO_PP_OD val)
{
  LSM6DSO_Ctrl3_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.pp_od = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Mode_Get(LSM6DSO_PP_OD *val)
{
  LSM6DSO_Ctrl3_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.pp_od) {
    case LSM6DSO_PUSH_PULL:
      *val = LSM6DSO_PUSH_PULL;
      break;
    case LSM6DSO_OPEN_DRAIN:
      *val = LSM6DSO_OPEN_DRAIN;
      break;
    default:
      *val = LSM6DSO_PUSH_PULL;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Polarity_Set(LSM6DSO_HL_Active val)
{
  LSM6DSO_Ctrl3_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.h_lactive = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Polarity_Get(LSM6DSO_HL_Active *val)
{
  LSM6DSO_Ctrl3_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.h_lactive) {
    case LSM6DSO_ACTIVE_HIGH:
      *val = LSM6DSO_ACTIVE_HIGH;
      break;
    case LSM6DSO_ACTIVE_LOW:
      *val = LSM6DSO_ACTIVE_LOW;
      break;
    default:
      *val = LSM6DSO_ACTIVE_HIGH;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_All_On_Int1_Set(uint8_t val)
{
  LSM6DSO_Ctrl4_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.int2_on_int1 = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_All_On_Int1_Get(uint8_t *val)
{
  LSM6DSO_Ctrl4_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.int2_on_int1;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Int_Notification_Set(LSM6DSO_LIR val)
{
  LSM6DSO_Tap_Cfg0 tap_cfg0;
  LSM6DSO_Page_RW page_rw;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG0, (uint8_t*) &tap_cfg0, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  tap_cfg0.lir = (uint8_t)val & 0x01U;
  tap_cfg0.int_clr_on_read = (uint8_t)val & 0x01U;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_TAP_CFG0, (uint8_t*) &tap_cfg0, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  page_rw.emb_func_lir = ((uint8_t)val & 0x02U) >> 1;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Int_Notification_Get(LSM6DSO_LIR *val)
{
  LSM6DSO_Tap_Cfg0 tap_cfg0;
  LSM6DSO_Page_RW page_rw;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG0, (uint8_t*) &tap_cfg0, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  switch ((page_rw.emb_func_lir << 1) | tap_cfg0.lir) {
    case LSM6DSO_ALL_INT_PULSED:
      *val = LSM6DSO_ALL_INT_PULSED;
      break;
    case LSM6DSO_BASE_LATCHED_EMB_PULSED:
      *val = LSM6DSO_BASE_LATCHED_EMB_PULSED;
      break;
    case LSM6DSO_BASE_PULSED_EMB_LATCHED:
      *val = LSM6DSO_BASE_PULSED_EMB_LATCHED;
      break;
    case LSM6DSO_ALL_INT_LATCHED:
      *val = LSM6DSO_ALL_INT_LATCHED;
      break;
    default:
      *val = LSM6DSO_ALL_INT_PULSED;
      break;
  }

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_WakeUp_Ths_Weight_Set(LSM6DSO_Wake_Ths_W val)
{
  LSM6DSO_Wake_Up_Dur reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_DUR, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.wake_ths_w = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_WAKE_UP_DUR, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_WakeUp_Ths_Weight_Get(LSM6DSO_Wake_Ths_W *val)
{
  LSM6DSO_Wake_Up_Dur reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_DUR, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.wake_ths_w) {
    case LSM6DSO_LSb_FS_DIV_64:
      *val = LSM6DSO_LSb_FS_DIV_64;
      break;
    case LSM6DSO_LSb_FS_DIV_256:
      *val = LSM6DSO_LSb_FS_DIV_256;
      break;
    default:
      *val = LSM6DSO_LSb_FS_DIV_64;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_WakeUp_Threshold_Set(uint8_t val)
{
  LSM6DSO_Wake_Up_Ths reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_THS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.wk_ths = val;
  
  return LSM6DSO_Write_Reg(LSM6DSO_WAKE_UP_THS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_WakeUp_Threshold_Get(uint8_t *val)
{
  LSM6DSO_Wake_Up_Ths reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_THS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.wk_ths;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_On_WakeUp_Set(uint8_t val)
{
  LSM6DSO_Wake_Up_Ths reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_THS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.usr_off_on_wu = val;

  return LSM6DSO_Write_Reg(LSM6DSO_WAKE_UP_THS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_On_WakeUp_Get(uint8_t *val)
{
  LSM6DSO_Wake_Up_Ths reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_THS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.usr_off_on_wu;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_WakeUp_Dur_Set(uint8_t val)
{
  LSM6DSO_Wake_Up_Dur reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_DUR, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.wake_dur = val;
  
  return LSM6DSO_Write_Reg(LSM6DSO_WAKE_UP_DUR, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_WakeUp_Dur_Get(uint8_t *val)
{
  LSM6DSO_Wake_Up_Dur reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_DUR, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.wake_dur;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Sleep_Mode_Set(uint8_t val)
{
  LSM6DSO_Ctrl4_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.sleep_g = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Sleep_Mode_Get(uint8_t *val)
{
  LSM6DSO_Ctrl4_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.sleep_g;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Act_Pin_Notification_Set(LSM6DSO_Sleep_Status_On_Int val)
{
  LSM6DSO_Tap_Cfg0 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.sleep_status_on_int = (uint8_t)val;
  
  return LSM6DSO_Write_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Act_Pin_Notification_Get(LSM6DSO_Sleep_Status_On_Int *val)
{
  LSM6DSO_Tap_Cfg0 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.sleep_status_on_int) {
    case LSM6DSO_DRIVE_SLEEP_CHG_EVENT:
      *val = LSM6DSO_DRIVE_SLEEP_CHG_EVENT;
      break;
    case LSM6DSO_DRIVE_SLEEP_STATUS:
      *val = LSM6DSO_DRIVE_SLEEP_STATUS;
      break;
    default:
      *val = LSM6DSO_DRIVE_SLEEP_CHG_EVENT;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Act_Mode_Set(LSM6DSO_InAct_En val)
{
  LSM6DSO_Tap_Cfg2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.inact_en = (uint8_t)val;
  
  return LSM6DSO_Write_Reg(LSM6DSO_TAP_CFG2, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Act_Mode_Get(LSM6DSO_InAct_En *val)
{
  LSM6DSO_Tap_Cfg2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.inact_en) {
    case LSM6DSO_XL_AND_GY_NOT_AFFECTED:
      *val = LSM6DSO_XL_AND_GY_NOT_AFFECTED;
      break;
    case LSM6DSO_XL_12Hz5_GY_NOT_AFFECTED:
      *val = LSM6DSO_XL_12Hz5_GY_NOT_AFFECTED;
      break;
    case LSM6DSO_XL_12Hz5_GY_SLEEP:
      *val = LSM6DSO_XL_12Hz5_GY_SLEEP;
      break;
    case LSM6DSO_XL_12Hz5_GY_PD:
      *val = LSM6DSO_XL_12Hz5_GY_PD;
      break;
    default:
      *val = LSM6DSO_XL_AND_GY_NOT_AFFECTED;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Act_Sleep_Dur_Set(uint8_t val)
{
  LSM6DSO_Wake_Up_Dur reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_DUR, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.sleep_dur = val;

  return LSM6DSO_Write_Reg(LSM6DSO_WAKE_UP_DUR, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Act_Sleep_Dur_Get(uint8_t *val)
{
  LSM6DSO_Wake_Up_Dur reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_DUR, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.sleep_dur;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Detection_On_Z_Set(uint8_t val)
{
  LSM6DSO_Tap_Cfg0 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.tap_z_en = val;

  return LSM6DSO_Write_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Detection_On_Z_Get(uint8_t *val)
{
  LSM6DSO_Tap_Cfg0 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.tap_z_en;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Detection_On_Y_Set(uint8_t val)
{
  LSM6DSO_Tap_Cfg0 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.tap_y_en = val;

  return LSM6DSO_Write_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Detection_On_Y_Get(uint8_t *val)
{
  LSM6DSO_Tap_Cfg0 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.tap_y_en;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Detection_On_X_Set(uint8_t val)
{
  LSM6DSO_Tap_Cfg0 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.tap_x_en = val;

  return LSM6DSO_Write_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Detection_On_X_Get(uint8_t *val)
{
  LSM6DSO_Tap_Cfg0 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG0, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.tap_x_en;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Threshold_X_Set(uint8_t val)
{
  LSM6DSO_Tap_Cfg1 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG1, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.tap_ths_x = val;
  
  return LSM6DSO_Write_Reg(LSM6DSO_TAP_CFG1, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Threshold_X_Get(uint8_t *val)
{
  LSM6DSO_Tap_Cfg1 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG1, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.tap_ths_x;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Axis_Priority_Set(LSM6DSO_Tap_Priority val)
{
  LSM6DSO_Tap_Cfg1 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG1, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.tap_priority = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_TAP_CFG1, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Axis_Priority_Get(LSM6DSO_Tap_Priority *val)
{
  LSM6DSO_Tap_Cfg1 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG1, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.tap_priority) {
    case LSM6DSO_XYZ:
      *val = LSM6DSO_XYZ;
      break;
    case LSM6DSO_YXZ:
      *val = LSM6DSO_YXZ;
      break;
    case LSM6DSO_XZY:
      *val = LSM6DSO_XZY;
      break;
    case LSM6DSO_ZYX:
      *val = LSM6DSO_ZYX;
      break;
    case LSM6DSO_YZX:
      *val = LSM6DSO_YZX;
      break;
    case LSM6DSO_ZXY:
      *val = LSM6DSO_ZXY;
      break;
    default:
      *val = LSM6DSO_XYZ;
      break;
  }
  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Threshold_Y_Set(uint8_t val)
{
  LSM6DSO_Tap_Cfg2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.tap_ths_y = val;

  return LSM6DSO_Write_Reg(LSM6DSO_TAP_CFG2, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Threshold_Y_Get(uint8_t *val)
{
  LSM6DSO_Tap_Cfg2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.tap_ths_y;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Threshold_Z_Set(uint8_t val)
{
  LSM6DSO_Tap_Ths_6D reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_THS_6D, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.tap_ths_z = val;

  return LSM6DSO_Write_Reg(LSM6DSO_TAP_THS_6D, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Threshold_Z_Get(uint8_t *val)
{
  LSM6DSO_Tap_Ths_6D reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_THS_6D, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.tap_ths_z;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Shock_Set(uint8_t val)
{
  LSM6DSO_Int_Dur2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT_DUR2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.shock = val;

  return LSM6DSO_Write_Reg(LSM6DSO_INT_DUR2, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Shock_Get(uint8_t *val)
{
  LSM6DSO_Int_Dur2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT_DUR2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.shock;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Quiet_Set(uint8_t val)
{
  LSM6DSO_Int_Dur2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT_DUR2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.quiet = val;

  return LSM6DSO_Write_Reg(LSM6DSO_INT_DUR2, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Quiet_Get(uint8_t *val)
{
  LSM6DSO_Int_Dur2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT_DUR2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.quiet;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Dur_Set(uint8_t val)
{
  LSM6DSO_Int_Dur2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT_DUR2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.dur = val;

  return LSM6DSO_Write_Reg(LSM6DSO_INT_DUR2, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Dur_Get(uint8_t *val)
{
  LSM6DSO_Int_Dur2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT_DUR2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.dur;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Mode_Set(LSM6DSO_Single_Double_Tap val)
{
  LSM6DSO_Wake_Up_Ths reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_THS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.single_double_tap = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_WAKE_UP_THS, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Mode_Get(LSM6DSO_Single_Double_Tap *val)
{
  LSM6DSO_Wake_Up_Ths reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_THS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.single_double_tap) {
    case LSM6DSO_ONLY_SINGLE:
      *val = LSM6DSO_ONLY_SINGLE;
      break;
    case LSM6DSO_BOTH_SINGLE_DOUBLE:
      *val = LSM6DSO_BOTH_SINGLE_DOUBLE;
      break;
    default:
      *val = LSM6DSO_ONLY_SINGLE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_6D_Threshold_Set(LSM6DSO_6D_Ths val)
{
  LSM6DSO_Tap_Ths_6D reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_THS_6D, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.sixd_ths = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_TAP_THS_6D, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_6D_Threshold_Get(LSM6DSO_6D_Ths *val)
{
  LSM6DSO_Tap_Ths_6D reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_THS_6D, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.sixd_ths) {
    case LSM6DSO_DEG_80:
      *val = LSM6DSO_DEG_80;
      break;
    case LSM6DSO_DEG_70:
      *val = LSM6DSO_DEG_70;
      break;
    case LSM6DSO_DEG_60:
      *val = LSM6DSO_DEG_60;
      break;
    case LSM6DSO_DEG_50:
      *val = LSM6DSO_DEG_50;
      break;
    default:
      *val = LSM6DSO_DEG_80;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_4D_Mode_Set(uint8_t val)
{
  LSM6DSO_Tap_Ths_6D reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_THS_6D, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.d4d_en = val;

  return LSM6DSO_Write_Reg(LSM6DSO_TAP_THS_6D, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_4D_Mode_Get(uint8_t *val)
{
  LSM6DSO_Tap_Ths_6D reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_THS_6D, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.d4d_en;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FF_Threshold_Set(LSM6DSO_FF_Ths val)
{
  LSM6DSO_Free_Fall reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FREE_FALL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.ff_ths = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_FREE_FALL, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FF_Threshold_Get(LSM6DSO_FF_Ths *val)
{
  LSM6DSO_Free_Fall reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FREE_FALL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.ff_ths) {
    case LSM6DSO_FF_TSH_156mg:
      *val = LSM6DSO_FF_TSH_156mg;
      break;
    case LSM6DSO_FF_TSH_219mg:
      *val = LSM6DSO_FF_TSH_219mg;
      break;
    case LSM6DSO_FF_TSH_250mg:
      *val = LSM6DSO_FF_TSH_250mg;
      break;
    case LSM6DSO_FF_TSH_312mg:
      *val = LSM6DSO_FF_TSH_312mg;
      break;
    case LSM6DSO_FF_TSH_344mg:
      *val = LSM6DSO_FF_TSH_344mg;
      break;
    case LSM6DSO_FF_TSH_406mg:
      *val = LSM6DSO_FF_TSH_406mg;
      break;
    case LSM6DSO_FF_TSH_469mg:
      *val = LSM6DSO_FF_TSH_469mg;
      break;
    case LSM6DSO_FF_TSH_500mg:
      *val = LSM6DSO_FF_TSH_500mg;
      break;
    default:
      *val = LSM6DSO_FF_TSH_156mg;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FF_Dur_Set(uint8_t val)
{
  LSM6DSO_Wake_Up_Dur wake_up_dur;
  LSM6DSO_Free_Fall free_fall;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_DUR, (uint8_t*)&wake_up_dur, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FREE_FALL, (uint8_t*)&free_fall, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  wake_up_dur.ff_dur = ((uint8_t)val & 0x20U) >> 5;
  free_fall.ff_dur = (uint8_t)val & 0x1FU;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_WAKE_UP_DUR, (uint8_t*)&wake_up_dur, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Write_Reg(LSM6DSO_FREE_FALL, (uint8_t*)&free_fall, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FF_Dur_Get(uint8_t *val)
{
  LSM6DSO_Wake_Up_Dur wake_up_dur;
  LSM6DSO_Free_Fall free_fall;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_WAKE_UP_DUR, (uint8_t*)&wake_up_dur, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FREE_FALL, (uint8_t*)&free_fall, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = (wake_up_dur.ff_dur << 5) + free_fall.ff_dur;
  
  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_WaterMark_Set(uint16_t val)
{
  LSM6DSO_FIFO_Ctrl1 fifo_ctrl1;
  LSM6DSO_FIFO_Ctrl2 fifo_ctrl2;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&fifo_ctrl2, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  fifo_ctrl1.wtm = 0x00FFU & (uint8_t)val;
  fifo_ctrl2.wtm = (uint8_t)(( 0x0100U & val ) >> 8);

  op_status = LSM6DSO_Write_Reg(LSM6DSO_FIFO_CTRL1, (uint8_t*)&fifo_ctrl1, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Write_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&fifo_ctrl2, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_WaterMark_Get(uint16_t *val)
{
  LSM6DSO_FIFO_Ctrl1 fifo_ctrl1;
  LSM6DSO_FIFO_Ctrl2 fifo_ctrl2;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL1, (uint8_t*)&fifo_ctrl1, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&fifo_ctrl2, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = ((uint16_t)fifo_ctrl2.wtm << 8) + (uint16_t)fifo_ctrl1.wtm;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Compression_Algo_Init_Set(uint8_t val)
{
  LSM6DSO_EMB_Func_Init_B reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_INIT_B, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.fifo_compr_init = val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_EMB_FUNC_INIT_B, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Compression_Algo_Init_Get(uint8_t *val)
{
  LSM6DSO_EMB_Func_Init_B reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_INIT_B, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.fifo_compr_init;

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Compression_Algo_Set(LSM6DSO_UnCoPtr_Rate val)
{
  LSM6DSO_FIFO_Ctrl2 fifo_ctrl2;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&fifo_ctrl2, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  fifo_ctrl2.fifo_compr_rt_en = ((uint8_t)val & 0x04U) >> 2;
  fifo_ctrl2.uncoptr_rate = (uint8_t)val & 0x03U;

  return LSM6DSO_Write_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&fifo_ctrl2, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Compression_Algo_Get(LSM6DSO_UnCoPtr_Rate *val)
{
  LSM6DSO_FIFO_Ctrl2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch ((reg.fifo_compr_rt_en<<2) | reg.uncoptr_rate) {
    case LSM6DSO_CMP_DISABLE:
      *val = LSM6DSO_CMP_DISABLE;
      break;
    case LSM6DSO_CMP_ALWAYS:
      *val = LSM6DSO_CMP_ALWAYS;
      break;
    case LSM6DSO_CMP_8_TO_1:
      *val = LSM6DSO_CMP_8_TO_1;
      break;
    case LSM6DSO_CMP_16_TO_1:
      *val = LSM6DSO_CMP_16_TO_1;
      break;
    case LSM6DSO_CMP_32_TO_1:
      *val = LSM6DSO_CMP_32_TO_1;
      break;
    default:
      *val = LSM6DSO_CMP_DISABLE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Virtual_Sens_ODR_CHG_Set(uint8_t val)
{
  LSM6DSO_FIFO_Ctrl2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.odrchg_en = val;

  return LSM6DSO_Write_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Virtual_Sens_ODR_CHG_Get(uint8_t *val)
{
  LSM6DSO_FIFO_Ctrl2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.odrchg_en;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Compression_Algo_Real_Time_Set(uint8_t val)
{
  LSM6DSO_FIFO_Ctrl2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.fifo_compr_rt_en = val;

  return LSM6DSO_Write_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Compression_Algo_Real_Time_Get(uint8_t *val)
{
  LSM6DSO_FIFO_Ctrl2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.fifo_compr_rt_en;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Stop_On_WTM_Set(uint8_t val)
{
  LSM6DSO_FIFO_Ctrl2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.stop_on_wtm = val;

  return LSM6DSO_Write_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Stop_On_WTM_Get(uint8_t *val)
{
  LSM6DSO_FIFO_Ctrl2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.stop_on_wtm;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_XL_Batch_Set(LSM6DSO_BDR_XL val)
{
  LSM6DSO_FIFO_Ctrl3 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL3, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.bdr_xl = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_FIFO_CTRL3, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_XL_Batch_Get(LSM6DSO_BDR_XL *val)
{
  LSM6DSO_FIFO_Ctrl3 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL3, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.bdr_xl) {
    case LSM6DSO_XL_NOT_BATCHED:
      *val = LSM6DSO_XL_NOT_BATCHED;
      break;
    case LSM6DSO_XL_BATCHED_AT_12Hz5:
      *val = LSM6DSO_XL_BATCHED_AT_12Hz5;
      break;
    case LSM6DSO_XL_BATCHED_AT_26Hz:
      *val = LSM6DSO_XL_BATCHED_AT_26Hz;
      break;
    case LSM6DSO_XL_BATCHED_AT_52Hz:
      *val = LSM6DSO_XL_BATCHED_AT_52Hz;
      break;
    case LSM6DSO_XL_BATCHED_AT_104Hz:
      *val = LSM6DSO_XL_BATCHED_AT_104Hz;
      break;
    case LSM6DSO_XL_BATCHED_AT_208Hz:
      *val = LSM6DSO_XL_BATCHED_AT_208Hz;
      break;
    case LSM6DSO_XL_BATCHED_AT_417Hz:
      *val = LSM6DSO_XL_BATCHED_AT_417Hz;
      break;
    case LSM6DSO_XL_BATCHED_AT_833Hz:
      *val = LSM6DSO_XL_BATCHED_AT_833Hz;
      break;
    case LSM6DSO_XL_BATCHED_AT_1667Hz:
      *val = LSM6DSO_XL_BATCHED_AT_1667Hz;
      break;
    case LSM6DSO_XL_BATCHED_AT_3333Hz:
      *val = LSM6DSO_XL_BATCHED_AT_3333Hz;
      break;
    case LSM6DSO_XL_BATCHED_AT_6667Hz:
      *val = LSM6DSO_XL_BATCHED_AT_6667Hz;
      break;
    case LSM6DSO_XL_BATCHED_AT_6Hz5:
      *val = LSM6DSO_XL_BATCHED_AT_6Hz5;
      break;
    default:
      *val = LSM6DSO_XL_NOT_BATCHED;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Gy_Batch_Set(LSM6DSO_BDR_Gy val)
{
  LSM6DSO_FIFO_Ctrl3 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL3, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.bdr_gy = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_FIFO_CTRL3, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Gy_Batch_Get(LSM6DSO_BDR_Gy *val)
{
  LSM6DSO_FIFO_Ctrl3 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL3, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.bdr_gy) {
    case LSM6DSO_GY_NOT_BATCHED:
      *val = LSM6DSO_GY_NOT_BATCHED;
      break;
    case LSM6DSO_GY_BATCHED_AT_12Hz5:
      *val = LSM6DSO_GY_BATCHED_AT_12Hz5;
      break;
    case LSM6DSO_GY_BATCHED_AT_26Hz:
      *val = LSM6DSO_GY_BATCHED_AT_26Hz;
      break;
    case LSM6DSO_GY_BATCHED_AT_52Hz:
      *val = LSM6DSO_GY_BATCHED_AT_52Hz;
      break;
    case LSM6DSO_GY_BATCHED_AT_104Hz:
      *val = LSM6DSO_GY_BATCHED_AT_104Hz;
      break;
    case LSM6DSO_GY_BATCHED_AT_208Hz:
      *val = LSM6DSO_GY_BATCHED_AT_208Hz;
      break;
    case LSM6DSO_GY_BATCHED_AT_417Hz:
      *val = LSM6DSO_GY_BATCHED_AT_417Hz;
      break;
    case LSM6DSO_GY_BATCHED_AT_833Hz:
      *val = LSM6DSO_GY_BATCHED_AT_833Hz;
      break;
    case LSM6DSO_GY_BATCHED_AT_1667Hz:
      *val = LSM6DSO_GY_BATCHED_AT_1667Hz;
      break;
    case LSM6DSO_GY_BATCHED_AT_3333Hz:
      *val = LSM6DSO_GY_BATCHED_AT_3333Hz;
      break;
    case LSM6DSO_GY_BATCHED_AT_6667Hz:
      *val = LSM6DSO_GY_BATCHED_AT_6667Hz;
      break;
    case LSM6DSO_GY_BATCHED_AT_6Hz5:
      *val = LSM6DSO_GY_BATCHED_AT_6Hz5;
      break;
    default:
      *val = LSM6DSO_GY_NOT_BATCHED;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Mode_Set(LSM6DSO_FIFO_Mode val)
{
  LSM6DSO_FIFO_Ctrl4 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL4, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.fifo_mode = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_FIFO_CTRL4, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Mode_Get(LSM6DSO_FIFO_Mode *val)
{
  LSM6DSO_FIFO_Ctrl4 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL4, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.fifo_mode) {
    case LSM6DSO_BYPASS_MODE:
      *val = LSM6DSO_BYPASS_MODE;
      break;
    case LSM6DSO_FIFO_MODE:
      *val = LSM6DSO_FIFO_MODE;
      break;
    case LSM6DSO_STREAM_TO_FIFO_MODE:
      *val = LSM6DSO_STREAM_TO_FIFO_MODE;
      break;
    case LSM6DSO_BYPASS_TO_STREAM_MODE:
      *val = LSM6DSO_BYPASS_TO_STREAM_MODE;
      break;
    case LSM6DSO_STREAM_MODE:
      *val = LSM6DSO_STREAM_MODE;
      break;
    case LSM6DSO_BYPASS_TO_FIFO_MODE:
      *val = LSM6DSO_BYPASS_TO_FIFO_MODE;
      break;
    default:
      *val = LSM6DSO_BYPASS_MODE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Temp_Batch_Set(LSM6DSO_ODR_Temp_Batch val)
{
  LSM6DSO_FIFO_Ctrl4 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL4, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.odr_t_batch = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_FIFO_CTRL4, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Temp_Batch_Get(LSM6DSO_ODR_Temp_Batch *val)
{
  LSM6DSO_FIFO_Ctrl4 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL4, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.odr_t_batch) {
    case LSM6DSO_TEMP_NOT_BATCHED:
      *val = LSM6DSO_TEMP_NOT_BATCHED;
      break;
    case LSM6DSO_TEMP_BATCHED_AT_1Hz6:
      *val = LSM6DSO_TEMP_BATCHED_AT_1Hz6;
      break;
    case LSM6DSO_TEMP_BATCHED_AT_12Hz5:
      *val = LSM6DSO_TEMP_BATCHED_AT_12Hz5;
      break;
    case LSM6DSO_TEMP_BATCHED_AT_52Hz:
      *val = LSM6DSO_TEMP_BATCHED_AT_52Hz;
      break;
    default:
      *val = LSM6DSO_TEMP_NOT_BATCHED;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_TimeStamp_Decimation_Set(LSM6DSO_ODR_TS_Batch val)
{
  LSM6DSO_FIFO_Ctrl4 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL4, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.odr_ts_batch = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_FIFO_CTRL4, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_TimeStamp_Decimation_Get(LSM6DSO_ODR_TS_Batch *val)
{
  LSM6DSO_FIFO_Ctrl4 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_CTRL4, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.odr_ts_batch) {
    case LSM6DSO_NO_DECIMATION:
      *val = LSM6DSO_NO_DECIMATION;
      break;
    case LSM6DSO_DEC_1:
      *val = LSM6DSO_DEC_1;
      break;
    case LSM6DSO_DEC_8:
      *val = LSM6DSO_DEC_8;
      break;
    case LSM6DSO_DEC_32:
      *val = LSM6DSO_DEC_32;
      break;
    default:
      *val = LSM6DSO_NO_DECIMATION;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Cnt_Event_Batch_Set(LSM6DSO_Trig_Counter_BDR val)
{
  LSM6DSO_Counter_BDR_Reg1 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_COUNTER_BDR_REG1, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.trig_counter_bdr = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_COUNTER_BDR_REG1, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Cnt_Event_Batch_Get(LSM6DSO_Trig_Counter_BDR *val)
{
  LSM6DSO_Counter_BDR_Reg1 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_COUNTER_BDR_REG1, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.trig_counter_bdr) {
    case LSM6DSO_XL_BATCH_EVENT:
      *val = LSM6DSO_XL_BATCH_EVENT;
      break;
    case LSM6DSO_GYRO_BATCH_EVENT:
      *val = LSM6DSO_GYRO_BATCH_EVENT;
      break;
    default:
      *val = LSM6DSO_XL_BATCH_EVENT;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Rst_Batch_Counter_Set(uint8_t val)
{
  LSM6DSO_Counter_BDR_Reg1 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_COUNTER_BDR_REG1, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.rst_counter_bdr = val;
  
  return LSM6DSO_Write_Reg(LSM6DSO_COUNTER_BDR_REG1, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Rst_Batch_Counter_Get(uint8_t *val)
{
  LSM6DSO_Counter_BDR_Reg1 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_COUNTER_BDR_REG1, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.rst_counter_bdr;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Batch_Counter_Threshold_Set(uint16_t val)
{
  LSM6DSO_Counter_BDR_Reg1 counter_bdr_reg1;
  LSM6DSO_Counter_BDR_Reg2 counter_bdr_reg2;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_COUNTER_BDR_REG1, (uint8_t*)&counter_bdr_reg1, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  counter_bdr_reg2.cnt_bdr_th =  0x00FFU & (uint8_t)val;
  counter_bdr_reg1.cnt_bdr_th = (uint8_t)(0x0700U & val) >> 8;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_COUNTER_BDR_REG1, (uint8_t*)&counter_bdr_reg1, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Write_Reg(LSM6DSO_COUNTER_BDR_REG2, (uint8_t*)&counter_bdr_reg2, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Batch_Counter_Threshold_Get(uint16_t *val)
{
  LSM6DSO_Counter_BDR_Reg1 counter_bdr_reg1;
  LSM6DSO_Counter_BDR_Reg2 counter_bdr_reg2;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_COUNTER_BDR_REG1, (uint8_t*)&counter_bdr_reg1, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_COUNTER_BDR_REG2, (uint8_t*)&counter_bdr_reg2, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = ((uint16_t)counter_bdr_reg1.cnt_bdr_th << 8) + (uint16_t)counter_bdr_reg2.cnt_bdr_th;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Data_Level_Get(uint16_t *val)
{
  LSM6DSO_FIFO_Status1 fifo_status1;
  LSM6DSO_FIFO_Status2 fifo_status2;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_STATUS1, (uint8_t*)&fifo_status1, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_STATUS2, (uint8_t*)&fifo_status2, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = ((uint16_t)fifo_status2.diff_fifo << 8) + (uint16_t)fifo_status1.diff_fifo;
  
  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Status_Get(LSM6DSO_FIFO_Status2 *val)
{
  return LSM6DSO_Read_Reg(LSM6DSO_FIFO_STATUS2, (uint8_t*) val, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Full_Flag_Get(uint8_t *val)
{
  LSM6DSO_FIFO_Status2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_STATUS2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.fifo_full_ia;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Ovr_Flag_Get(uint8_t *val)
{
  LSM6DSO_FIFO_Status2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_STATUS2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.fifo_ovr_ia;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_WTM_Flag_Get(uint8_t *val)
{
  LSM6DSO_FIFO_Status2 reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_STATUS2, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.fifo_wtm_ia;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Sensor_Tag_Get(LSM6DSO_FIFO_Tag *val)
{
  LSM6DSO_FIFO_Data_Out_Tag reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FIFO_DATA_OUT_TAG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.tag_sensor) {
    case LSM6DSO_GYRO_NC_TAG:
      *val = LSM6DSO_GYRO_NC_TAG;
      break;
    case LSM6DSO_XL_NC_TAG:
      *val = LSM6DSO_XL_NC_TAG;
      break;
    case LSM6DSO_TEMPERATURE_TAG:
      *val = LSM6DSO_TEMPERATURE_TAG;
      break;
    case LSM6DSO_CFG_CHANGE_TAG:
      *val = LSM6DSO_CFG_CHANGE_TAG;
      break;
    case LSM6DSO_XL_NC_T_2_TAG:
      *val = LSM6DSO_XL_NC_T_2_TAG;
      break;
    case LSM6DSO_XL_NC_T_1_TAG:
      *val = LSM6DSO_XL_NC_T_1_TAG;
      break;
    case LSM6DSO_XL_2XC_TAG:
      *val = LSM6DSO_XL_2XC_TAG;
      break;
    case LSM6DSO_XL_3XC_TAG:
      *val = LSM6DSO_XL_3XC_TAG;
      break;
    case LSM6DSO_GYRO_NC_T_2_TAG:
      *val = LSM6DSO_GYRO_NC_T_2_TAG;
      break;
    case LSM6DSO_GYRO_NC_T_1_TAG:
      *val = LSM6DSO_GYRO_NC_T_1_TAG;
      break;
    case LSM6DSO_GYRO_2XC_TAG:
      *val = LSM6DSO_GYRO_2XC_TAG;
      break;
    case LSM6DSO_GYRO_3XC_TAG:
      *val = LSM6DSO_GYRO_3XC_TAG;
      break;
    case LSM6DSO_SENSORHUB_SLAVE0_TAG:
      *val = LSM6DSO_SENSORHUB_SLAVE0_TAG;
      break;
    case LSM6DSO_SENSORHUB_SLAVE1_TAG:
      *val = LSM6DSO_SENSORHUB_SLAVE1_TAG;
      break;
    case LSM6DSO_SENSORHUB_SLAVE2_TAG:
      *val = LSM6DSO_SENSORHUB_SLAVE2_TAG;
      break;
    case LSM6DSO_SENSORHUB_SLAVE3_TAG:
      *val = LSM6DSO_SENSORHUB_SLAVE3_TAG;
      break;
    case LSM6DSO_STEP_CPUNTER_TAG:
      *val = LSM6DSO_STEP_CPUNTER_TAG;
      break;
    case LSM6DSO_GAME_ROTATION_TAG:
      *val = LSM6DSO_GAME_ROTATION_TAG;
      break;
    case LSM6DSO_GEOMAG_ROTATION_TAG:
      *val = LSM6DSO_GEOMAG_ROTATION_TAG;
      break;
    case LSM6DSO_ROTATION_TAG:
      *val = LSM6DSO_ROTATION_TAG;
      break;
    case LSM6DSO_SENSORHUB_NACK_TAG:
      *val = LSM6DSO_SENSORHUB_NACK_TAG;
      break;
    default:
      *val = LSM6DSO_GYRO_NC_TAG;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Pedo_Batch_Set(uint8_t val)
{
  LSM6DSO_EMB_Func_FIFO_Cfg reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_FIFO_CFG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.pedo_fifo_en = val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_EMB_FUNC_FIFO_CFG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Pedo_Batch_Get(uint8_t *val)
{
  LSM6DSO_EMB_Func_FIFO_Cfg reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_FIFO_CFG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  *val = reg.pedo_fifo_en;

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_0_Set(uint8_t val)
{
  LSM6DSO_Slv0_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_SLV0_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.batch_ext_sens_0_en = val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV0_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_0_Get(uint8_t *val)
{
  LSM6DSO_Slv0_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_SLV0_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.batch_ext_sens_0_en;

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_1_Set(uint8_t val)
{
  LSM6DSO_Slv1_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_SLV1_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.batch_ext_sens_1_en = val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV1_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_1_Get(uint8_t *val)
{
  LSM6DSO_Slv1_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_SLV1_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.batch_ext_sens_1_en;
    
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_2_Set(uint8_t val)
{
  LSM6DSO_Slv2_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_SLV2_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.batch_ext_sens_2_en = val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV2_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_2_Get(uint8_t *val)
{
  LSM6DSO_Slv2_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_SLV2_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  *val = reg.batch_ext_sens_2_en;

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_3_Set(uint8_t val)
{
  LSM6DSO_Slv3_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_SLV3_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.batch_ext_sens_3_en = val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV3_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_3_Get(uint8_t *val)
{
  LSM6DSO_Slv3_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_SLV3_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.batch_ext_sens_3_en;

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mode_Set(LSM6DSO_Den_Mode val)
{
  LSM6DSO_Ctrl6_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL6_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.den_mode = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL6_C, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mode_Get(LSM6DSO_Den_Mode *val)
{
  LSM6DSO_Ctrl6_C reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL6_C, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.den_mode) {
    case LSM6DSO_DEN_DISABLE:
      *val = LSM6DSO_DEN_DISABLE;
      break;
    case LSM6DSO_LEVEL_FIFO:
      *val = LSM6DSO_LEVEL_FIFO;
      break;
    case LSM6DSO_LEVEL_LETCHED:
      *val = LSM6DSO_LEVEL_LETCHED;
      break;
    case LSM6DSO_LEVEL_TRIGGER:
      *val = LSM6DSO_LEVEL_TRIGGER;
      break;
    case LSM6DSO_EDGE_TRIGGER:
      *val = LSM6DSO_EDGE_TRIGGER;
      break;
    default:
      *val = LSM6DSO_DEN_DISABLE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Polarity_Set(LSM6DSO_Den_LH val)
{
  LSM6DSO_Ctrl9_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.den_lh = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Polarity_Get(LSM6DSO_Den_LH *val)
{
  LSM6DSO_Ctrl9_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.den_lh) {
    case LSM6DSO_DEN_ACT_LOW:
      *val = LSM6DSO_DEN_ACT_LOW;
      break;
    case LSM6DSO_DEN_ACT_HIGH:
      *val = LSM6DSO_DEN_ACT_HIGH;
      break;
    default:
      *val = LSM6DSO_DEN_ACT_LOW;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Enable_Set(LSM6DSO_Den_XL_G val)
{
  LSM6DSO_Ctrl9_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.den_xl_g = (uint8_t)val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Enable_Get(LSM6DSO_Den_XL_G *val)
{
  LSM6DSO_Ctrl9_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.den_xl_g) {
    case LSM6DSO_STAMP_IN_GY_DATA:
      *val = LSM6DSO_STAMP_IN_GY_DATA;
      break;
    case LSM6DSO_STAMP_IN_XL_DATA:
      *val = LSM6DSO_STAMP_IN_XL_DATA;
      break;
    case LSM6DSO_STAMP_IN_GY_XL_DATA:
      *val = LSM6DSO_STAMP_IN_GY_XL_DATA;
      break;
    default:
      *val = LSM6DSO_STAMP_IN_GY_DATA;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mark_Axis_X_Set(uint8_t val)
{
  LSM6DSO_Ctrl9_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.den_z = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mark_Axis_X_Get(uint8_t *val)
{
  LSM6DSO_Ctrl9_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.den_z;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mark_Axis_Y_Set(uint8_t val)
{
  LSM6DSO_Ctrl9_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.den_y = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mark_Axis_Y_Get(uint8_t *val)
{
  LSM6DSO_Ctrl9_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.den_y;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mark_Axis_Z_Set(uint8_t val)
{
  LSM6DSO_Ctrl9_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.den_x = val;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mark_Axis_Z_Get(uint8_t *val)
{
  LSM6DSO_Ctrl9_XL reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.den_x;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Sens_Set(LSM6DSO_Pedo_MD val)
{
  LSM6DSO_Pedo_Cmd_Reg pedo_cmd_reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_PEDO_CMD_REG, (uint8_t*)&pedo_cmd_reg);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  pedo_cmd_reg.fp_rejection_en = ((uint8_t)val & 0x10U)>>4;
  pedo_cmd_reg.ad_det_en = ((uint8_t)val & 0x20U)>>5;
  
  return LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_PEDO_CMD_REG, (uint8_t*)&pedo_cmd_reg);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Sens_Get(LSM6DSO_Pedo_MD *val)
{
  LSM6DSO_Pedo_Cmd_Reg pedo_cmd_reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_PEDO_CMD_REG, (uint8_t*)&pedo_cmd_reg);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch ( (pedo_cmd_reg.ad_det_en <<5) | (pedo_cmd_reg.fp_rejection_en << 4) ) {
    case LSM6DSO_PEDO_BASE_MODE:
      *val = LSM6DSO_PEDO_BASE_MODE;
      break;
    case LSM6DSO_FALSE_STEP_REJ:
      *val = LSM6DSO_FALSE_STEP_REJ;
      break;
    case LSM6DSO_FALSE_STEP_REJ_ADV_MODE:
      *val = LSM6DSO_FALSE_STEP_REJ_ADV_MODE;
      break;
    default:
      *val = LSM6DSO_PEDO_BASE_MODE;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Step_Detect_Get(uint8_t *val)
{
  LSM6DSO_EMB_Func_Status reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_STATUS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  *val = reg.is_step_det;

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Debounce_Steps_Set(uint8_t *buff)
{
  return LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_PEDO_DEB_STEPS_CONF, buff);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Debounce_Steps_Get(uint8_t *buff)
{
  return LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_PEDO_DEB_STEPS_CONF, buff);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Steps_Period_Set(uint8_t *buff)
{
  LSM6DSO_OpStatus op_status;
  uint8_t index = 0x00U;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_PEDO_SC_DELTAT_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;
  
  return LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_PEDO_SC_DELTAT_H, &buff[index]);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Steps_Period_Get(uint8_t *buff)
{
  LSM6DSO_OpStatus op_status;
  uint8_t index = 0x00U;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_PEDO_SC_DELTAT_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  return LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_PEDO_SC_DELTAT_H, &buff[index]);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Int_Mode_Set(LSM6DSO_Carry_Count_En val)
{
  LSM6DSO_Pedo_Cmd_Reg reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_PEDO_CMD_REG, (uint8_t*)&reg);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.carry_count_en = (uint8_t)val;

  return LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_PEDO_CMD_REG, (uint8_t*)&reg);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Int_Mode_Get(LSM6DSO_Carry_Count_En *val)
{
  LSM6DSO_Pedo_Cmd_Reg reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_PEDO_CMD_REG, (uint8_t*)&reg);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.carry_count_en) {
    case LSM6DSO_EVERY_STEP:
      *val = LSM6DSO_EVERY_STEP;
      break;
    case LSM6DSO_COUNT_OVERFLOW:
      *val = LSM6DSO_COUNT_OVERFLOW;
      break;
    default:
      *val = LSM6DSO_EVERY_STEP;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Motion_Flag_Data_Ready_Get(uint8_t *val)
{
  LSM6DSO_EMB_Func_Status reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_STATUS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  *val = reg.is_sigmot;

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Tilt_Flag_Data_Ready_Get(uint8_t *val)
{
  LSM6DSO_EMB_Func_Status reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_STATUS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.is_tilt;

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Sensitivity_Set(uint8_t *buff)
{
  LSM6DSO_OpStatus op_status;
  uint8_t index = 0x00U;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_SENSITIVITY_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  return LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_SENSITIVITY_H, &buff[index]);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Sensitivity_Get(uint8_t *buff)
{
  LSM6DSO_OpStatus op_status;
  uint8_t index = 0x00U;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_SENSITIVITY_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  return LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_SENSITIVITY_H, &buff[index]);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Offset_Set(uint8_t *buff)
{
  LSM6DSO_OpStatus op_status;
  uint8_t index = 0x00U;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_OFFX_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_OFFX_H, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_OFFY_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_OFFY_H, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_OFFZ_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  return LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_OFFZ_H, &buff[index]);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Offset_Get(uint8_t *buff)
{
  LSM6DSO_OpStatus op_status;
  uint8_t index = 0x00U;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_OFFX_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_OFFX_H, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_OFFY_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_OFFY_H, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_OFFZ_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  return LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_OFFZ_H, &buff[index]);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Soft_Iron_Set(uint8_t *buff)
{
  LSM6DSO_OpStatus op_status;
  uint8_t index = 0x00U;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_SI_XX_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_SI_XX_H, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_SI_XY_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;
  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_SI_XY_H, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_SI_XZ_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_SI_XZ_H, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_SI_YY_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_SI_YY_H, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_SI_YZ_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_SI_YZ_H, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_SI_ZZ_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  return LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_SI_ZZ_H, &buff[index]);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Soft_Iron_Get(uint8_t *buff)
{
  LSM6DSO_OpStatus op_status;
  uint8_t index = 0x00U;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_SI_XX_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_SI_XX_H, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_SI_XY_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_SI_XY_H, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_SI_XZ_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_SI_XZ_H, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_SI_YY_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_SI_YY_H, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_SI_YZ_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_SI_YZ_H, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_SI_ZZ_L, &buff[index]);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  index++;

  return LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_SI_ZZ_H, &buff[index]);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Z_Orient_Set(LSM6DSO_Mag_Z_Axis val)
{
  LSM6DSO_Mag_Cfg_A reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_CFG_A, (uint8_t*)&reg);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.mag_z_axis = (uint8_t) val;

  return LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_CFG_A, (uint8_t*)&reg);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Z_Orient_Get(LSM6DSO_Mag_Z_Axis *val)
{
  LSM6DSO_Mag_Cfg_A reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_CFG_A, (uint8_t*)&reg);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.mag_z_axis) {
    case LSM6DSO_Z_EQ_Y:
      *val = LSM6DSO_Z_EQ_Y;
      break;
    case LSM6DSO_Z_EQ_MIN_Y:
      *val = LSM6DSO_Z_EQ_MIN_Y;
      break;
    case LSM6DSO_Z_EQ_X:
      *val = LSM6DSO_Z_EQ_X;
      break;
    case LSM6DSO_Z_EQ_MIN_X:
      *val = LSM6DSO_Z_EQ_MIN_X;
      break;
    case LSM6DSO_Z_EQ_MIN_Z:
      *val = LSM6DSO_Z_EQ_MIN_Z;
      break;
    case LSM6DSO_Z_EQ_Z:
      *val = LSM6DSO_Z_EQ_Z;
      break;
    default:
      *val = LSM6DSO_Z_EQ_Y;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Y_Orient_Set(LSM6DSO_Mag_Y_Axis val)
{
  LSM6DSO_Mag_Cfg_A reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_CFG_A, (uint8_t*)&reg);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.mag_y_axis = (uint8_t)val;

  return LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_CFG_A,(uint8_t*) &reg);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Y_Orient_Get(LSM6DSO_Mag_Y_Axis *val)
{
  LSM6DSO_Mag_Cfg_A reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_CFG_A, (uint8_t*)&reg);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.mag_y_axis) {
    case LSM6DSO_Y_EQ_Y:
      *val = LSM6DSO_Y_EQ_Y;
      break;
    case LSM6DSO_Y_EQ_MIN_Y:
      *val = LSM6DSO_Y_EQ_MIN_Y;
      break;
    case LSM6DSO_Y_EQ_X:
      *val = LSM6DSO_Y_EQ_X;
      break;
    case LSM6DSO_Y_EQ_MIN_X:
      *val = LSM6DSO_Y_EQ_MIN_X;
      break;
    case LSM6DSO_Y_EQ_MIN_Z:
      *val = LSM6DSO_Y_EQ_MIN_Z;
      break;
    case LSM6DSO_Y_EQ_Z:
      *val = LSM6DSO_Y_EQ_Z;
      break;
    default:
      *val = LSM6DSO_Y_EQ_Y;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_X_Orient_Set(LSM6DSO_Mag_X_Axis val)
{
  LSM6DSO_Mag_Cfg_B reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_CFG_B, (uint8_t*)&reg);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.mag_x_axis = (uint8_t)val;

  return LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_MAG_CFG_B, (uint8_t*)&reg);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_X_Orient_Get(LSM6DSO_Mag_X_Axis *val)
{
  LSM6DSO_Mag_Cfg_B reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_MAG_CFG_B, (uint8_t*)&reg);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.mag_x_axis) {
    case LSM6DSO_X_EQ_Y:
      *val = LSM6DSO_X_EQ_Y;
      break;
    case LSM6DSO_X_EQ_MIN_Y:
      *val = LSM6DSO_X_EQ_MIN_Y;
      break;
    case LSM6DSO_X_EQ_X:
      *val = LSM6DSO_X_EQ_X;
      break;
    case LSM6DSO_X_EQ_MIN_X:
      *val = LSM6DSO_X_EQ_MIN_X;
      break;
    case LSM6DSO_X_EQ_MIN_Z:
      *val = LSM6DSO_X_EQ_MIN_Z;
      break;
    case LSM6DSO_X_EQ_Z:
      *val = LSM6DSO_X_EQ_Z;
      break;
    default:
      *val = LSM6DSO_X_EQ_Y;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Long_Cnt_Flag_Data_Ready_Get(uint8_t *val)
{
  LSM6DSO_EMB_Func_Status reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_STATUS, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = reg.is_fsm_lc;

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Enable_Set(LSM6DSO_Emb_FSM_Enable *val)
{
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Write_Reg(LSM6DSO_FSM_ENABLE_A, (uint8_t*)&val->fsm_enable_a, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Write_Reg(LSM6DSO_FSM_ENABLE_B, (uint8_t*)&val->fsm_enable_b, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Enable_Get(LSM6DSO_Emb_FSM_Enable *val)
{
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FSM_ENABLE_A, (uint8_t*) val, 2);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Long_Cnt_Set(uint8_t *buff)
{
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Write_Reg(LSM6DSO_FSM_LONG_COUNTER_L, buff, 2);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Long_Cnt_Get(uint8_t *buff)
{
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FSM_LONG_COUNTER_L, buff, 2);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Long_Clr_Set(LSM6DSO_FSM_LC_Clr val)
{
  LSM6DSO_FSM_Long_Counter_Clear reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FSM_LONG_COUNTER_CLEAR, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg. fsm_lc_clr = (uint8_t)val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_FSM_LONG_COUNTER_CLEAR, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Long_Clr_Get(LSM6DSO_FSM_LC_Clr *val)
{
  LSM6DSO_FSM_Long_Counter_Clear reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FSM_LONG_COUNTER_CLEAR, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.fsm_lc_clr) {
    case LSM6DSO_LC_NORMAL:
      *val = LSM6DSO_LC_NORMAL;
      break;
    case LSM6DSO_LC_CLEAR:
      *val = LSM6DSO_LC_CLEAR;
      break;
    case LSM6DSO_LC_CLEAR_DONE:
      *val = LSM6DSO_LC_CLEAR_DONE;
      break;
    default:
      *val = LSM6DSO_LC_NORMAL;
      break;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Out_Get(LSM6DSO_FSM_Out *val)
{
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FSM_OUTS1, (uint8_t*)val, 16);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Data_Rate_Set(LSM6DSO_FSM_ODR val)
{
  LSM6DSO_EMB_Func_ODR_Cfg_B reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_ODR_CFG_B, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.not_used_01 = 3; /* set default values */
  reg.not_used_02 = 2; /* set default values */
  reg.fsm_odr = (uint8_t)val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_EMB_FUNC_ODR_CFG_B, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Data_Rate_Get(LSM6DSO_FSM_ODR *val)
{
  LSM6DSO_EMB_Func_ODR_Cfg_B reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_ODR_CFG_B, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch (reg.fsm_odr) {
    case LSM6DSO_ODR_FSM_12Hz5:
      *val = LSM6DSO_ODR_FSM_12Hz5;
      break;
    case LSM6DSO_ODR_FSM_26Hz:
      *val = LSM6DSO_ODR_FSM_26Hz;
      break;
    case LSM6DSO_ODR_FSM_52Hz:
      *val = LSM6DSO_ODR_FSM_52Hz;
      break;
    case LSM6DSO_ODR_FSM_104Hz:
      *val = LSM6DSO_ODR_FSM_104Hz;
      break;
    default:
      *val = LSM6DSO_ODR_FSM_12Hz5;
      break;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Init_Set(uint8_t val)
{
  LSM6DSO_EMB_Func_Init_B reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_INIT_B, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  reg.fsm_init = val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_EMB_FUNC_INIT_B, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Init_Get(uint8_t *val)
{
  LSM6DSO_EMB_Func_Init_B reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_INIT_B, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }


  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Long_Cnt_Int_Value_Set(uint16_t val)
{
  LSM6DSO_OpStatus op_status;
  uint8_t add_l;
  uint8_t add_h;

  add_h = (uint8_t)( ( val & 0xFF00U ) >> 8 );
  add_l = (uint8_t)( val & 0x00FFU );

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_FSM_LC_TIMEOUT_L, &add_l);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_FSM_LC_TIMEOUT_H, &add_h);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Long_Cnt_Int_Value_Get(uint16_t *val)
{
  LSM6DSO_OpStatus op_status;
  uint8_t add_l;
  uint8_t add_h;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_FSM_LC_TIMEOUT_L, &add_l);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_FSM_LC_TIMEOUT_H, &add_h);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  *val = add_h;
  *val = *val << 8;
  *val += add_l;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Number_Of_Programs_Set(uint8_t val)
{
  return LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_FSM_PROGRAMS, &val);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Number_Of_Programs_Get(uint8_t *val)
{
  return LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_FSM_PROGRAMS, val);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Start_Address_Set(uint16_t val)
{
  LSM6DSO_OpStatus op_status;
  uint8_t add_l;
  uint8_t add_h;

  add_h = (uint8_t)( ( val & 0xFF00U ) >> 8 );
  add_l = (uint8_t)( val & 0x00FFU );

  op_status = LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_FSM_START_ADD_L, &add_l);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Ln_Page_Write_Byte(LSM6DSO_FSM_START_ADD_H, &add_h);
}

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Start_Address_Get(uint16_t *val)
{
  LSM6DSO_OpStatus op_status;
  uint8_t add_l;
  uint8_t add_h;

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_FSM_START_ADD_L, &add_l);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  op_status = LSM6DSO_Reg_Ln_Page_Read_Byte(LSM6DSO_FSM_START_ADD_H, &add_h);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  *val = add_h;
  *val = *val << 8;
  *val += add_l;
    
  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Read_Data_Raw_Get(uint8_t *val, uint8_t len)
{
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_SENSOR_HUB_1, (uint8_t*) val, len);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Slave_Connected_Set(LSM6DSO_Aux_Sens_On val)
{
  LSM6DSO_Master_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.aux_sens_on = (uint8_t)val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Slave_Connected_Get(LSM6DSO_Aux_Sens_On *val)
{
  LSM6DSO_Master_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  switch (reg.aux_sens_on) {
    case LSM6DSO_SLV_0:
      *val = LSM6DSO_SLV_0;
      break;
    case LSM6DSO_SLV_0_1:
      *val = LSM6DSO_SLV_0_1;
      break;
    case LSM6DSO_SLV_0_1_2:
      *val = LSM6DSO_SLV_0_1_2;
      break;
    case LSM6DSO_SLV_0_1_2_3:
      *val = LSM6DSO_SLV_0_1_2_3;
      break;
    default:
      *val = LSM6DSO_SLV_0;
      break;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Master_Set(uint8_t val)
{
  LSM6DSO_Master_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.master_on = val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Master_Get(uint8_t *val)
{
  LSM6DSO_Master_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  *val = reg.master_on;

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Pin_Mode_Set(LSM6DSO_Shub_PU_En val)
{
  LSM6DSO_Master_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.shub_pu_en = (uint8_t)val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Pin_Mode_Get(LSM6DSO_Shub_PU_En *val)
{
  LSM6DSO_Master_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  switch (reg.shub_pu_en) {
    case LSM6DSO_EXT_PULL_UP:
      *val = LSM6DSO_EXT_PULL_UP;
      break;
    case LSM6DSO_INTERNAL_PULL_UP:
      *val = LSM6DSO_INTERNAL_PULL_UP;
      break;
    default:
      *val = LSM6DSO_EXT_PULL_UP;
      break;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Pass_Through_Set(uint8_t val)
{
  LSM6DSO_Master_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.pass_through_mode = val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Pass_Through_Get(uint8_t *val)
{
  LSM6DSO_Master_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  *val = reg.pass_through_mode;

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Syncro_Mode_Set(LSM6DSO_Start_Config val)
{
  LSM6DSO_Master_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.start_config = (uint8_t)val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Syncro_Mode_Get(LSM6DSO_Start_Config *val)
{
  LSM6DSO_Master_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  switch (reg.start_config) {
    case LSM6DSO_EXT_ON_INT2_PIN:
      *val = LSM6DSO_EXT_ON_INT2_PIN;
      break;
    case LSM6DSO_XL_GY_DRDY:
      *val = LSM6DSO_XL_GY_DRDY;
      break;
    default:
      *val = LSM6DSO_EXT_ON_INT2_PIN;
      break;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Write_Mode_Set(LSM6DSO_Write_Once val)
{
  LSM6DSO_Master_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.write_once = (uint8_t)val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Write_Mode_Get(LSM6DSO_Write_Once *val)
{
  LSM6DSO_Master_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  switch (reg.write_once) {
    case LSM6DSO_EACH_SH_CYCLE:
      *val = LSM6DSO_EACH_SH_CYCLE;
      break;
    case LSM6DSO_ONLY_FIRST_CYCLE:
      *val = LSM6DSO_ONLY_FIRST_CYCLE;
      break;
    default:
      *val = LSM6DSO_EACH_SH_CYCLE;
      break;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_reset_Set()
{
  LSM6DSO_Master_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.rst_master_regs = PROPERTY_ENABLE;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.rst_master_regs = PROPERTY_DISABLE;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_reset_Get(uint8_t *val)
{
  LSM6DSO_Master_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MASTER_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  *val = reg.rst_master_regs;

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Data_Rate_Set(LSM6DSO_Shub_ODR val)
{
  LSM6DSO_Slv0_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_SLV1_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.shub_odr = (uint8_t)val;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV1_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Data_Rate_Get(LSM6DSO_Shub_ODR *val)
{
  LSM6DSO_Slv0_Config reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_SLV1_CONFIG, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  switch (reg.shub_odr) {
    case LSM6DSO_SH_ODR_104Hz:
      *val = LSM6DSO_SH_ODR_104Hz;
      break;
    case LSM6DSO_SH_ODR_52Hz:
      *val = LSM6DSO_SH_ODR_52Hz;
      break;
    case LSM6DSO_SH_ODR_26Hz:
      *val = LSM6DSO_SH_ODR_26Hz;
      break;
    case LSM6DSO_SH_ODR_13Hz:
      *val = LSM6DSO_SH_ODR_13Hz;
      break;
    default:
      *val = LSM6DSO_SH_ODR_104Hz;
      break;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Cfg_Write(LSM6DSO_Sh_Cfg_Write *val)
{
  LSM6DSO_Slv0_Add reg;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  reg.slave0 = val->slv0_add;
  reg.rw_0 = 0;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV0_ADD, (uint8_t*)&reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV0_SUBADD, &(val->slv0_subadd), 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_DATAWRITE_SLV0, &(val->slv0_data), 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Slv0_Cfg_Read(LSM6DSO_Sh_Cfg_Read *val)
{
  LSM6DSO_Slv0_Add slv0_add;
  LSM6DSO_Slv0_Config slv0_config;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  slv0_add.slave0 = val->slv_add;
  slv0_add.rw_0 = 1;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV0_ADD, (uint8_t*)&slv0_add, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV0_SUBADD, &(val->slv_subadd), 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_SLV0_CONFIG, (uint8_t*)&slv0_config, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  slv0_config.slave0_numop = val->slv_len;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV0_CONFIG, (uint8_t*)&slv0_config, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Slv1_Cfg_Read(LSM6DSO_Sh_Cfg_Read *val)
{
  LSM6DSO_Slv1_Add slv1_add;
  LSM6DSO_Slv1_Config slv1_config;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  slv1_add.slave1_add = val->slv_add;
  slv1_add.r_1 = 1;
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV1_ADD, (uint8_t*)&slv1_add, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV1_SUBADD, &(val->slv_subadd), 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_SLV1_CONFIG, (uint8_t*)&slv1_config, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  slv1_config.slave1_numop = val->slv_len;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV1_CONFIG, (uint8_t*)&slv1_config, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Slv2_Cfg_Read(LSM6DSO_Sh_Cfg_Read *val)
{
  LSM6DSO_Slv2_Add slv2_add;
  LSM6DSO_Slv2_Config slv2_config;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  slv2_add.slave2_add = val->slv_add;
  slv2_add.r_2 = 1;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV2_ADD, (uint8_t*)&slv2_add, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV2_SUBADD, &(val->slv_subadd), 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_SLV2_CONFIG, (uint8_t*)&slv2_config, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  slv2_config.slave2_numop = val->slv_len;
  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV2_CONFIG, (uint8_t*)&slv2_config, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Slv3_Cfg_Read(LSM6DSO_Sh_Cfg_Read *val)
{
  LSM6DSO_Slv3_Add slv3_add;
  LSM6DSO_Slv3_Config slv3_config;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  slv3_add.slave3_add = val->slv_add;
  slv3_add.r_3 = 1;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV3_ADD, (uint8_t*)&slv3_add, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV3_SUBADD, &(val->slv_subadd), 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_SLV3_CONFIG, (uint8_t*)&slv3_config, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  slv3_config.slave3_numop = val->slv_len;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_SLV3_CONFIG, (uint8_t*)&slv3_config, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Status_Get(LSM6DSO_Status_Master *val)
{
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_SENSOR_HUB_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_STATUS_MASTER, (uint8_t*) val, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_ID_Get(LSM6DSO_ID *val)
{
  return LSM6DSO_Read_Reg(LSM6DSO_WHO_AM_I, (uint8_t*)&(val->ui), 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Init_Set(LSM6DSO_Init_Set val)
{
  LSM6DSO_EMB_Func_Init_A emb_func_init_a;
  LSM6DSO_EMB_Func_Init_B emb_func_init_b;
  LSM6DSO_Ctrl3_C ctrl3_c;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_INIT_B, (uint8_t*)&emb_func_init_b, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  emb_func_init_b.fifo_compr_init = (uint8_t)val & ( (uint8_t)LSM6DSO_FIFO_COMP >> 2 );
  emb_func_init_b.fsm_init = (uint8_t)val & ( (uint8_t)LSM6DSO_FSM >> 3 );

  op_status = LSM6DSO_Write_Reg(LSM6DSO_EMB_FUNC_INIT_B, (uint8_t*)&emb_func_init_b, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_INIT_A, (uint8_t*)&emb_func_init_a, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  emb_func_init_a.step_det_init = ( (uint8_t)val & (uint8_t)LSM6DSO_PEDO ) >> 5;
  emb_func_init_a.tilt_init = ( (uint8_t)val & (uint8_t)LSM6DSO_TILT ) >> 6;
  emb_func_init_a.sig_mot_init = ( (uint8_t)val & (uint8_t)LSM6DSO_SMOTION ) >> 7;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_EMB_FUNC_INIT_A, (uint8_t*)&emb_func_init_a, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&ctrl3_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  if ( (val == LSM6DSO_BOOT) || (val == LSM6DSO_RESET) )
  {
    ctrl3_c.boot = (uint8_t)val & (uint8_t)LSM6DSO_BOOT;
    ctrl3_c.sw_reset = ( (uint8_t)val & (uint8_t)LSM6DSO_RESET) >> 1;

    op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&ctrl3_c, 1);
  }

  if ( ( val == LSM6DSO_DRV_RDY ) && ( (ctrl3_c.bdu == PROPERTY_DISABLE) || (ctrl3_c.if_inc == PROPERTY_DISABLE) ) )
  {
    ctrl3_c.bdu = PROPERTY_ENABLE;
    ctrl3_c.if_inc = PROPERTY_ENABLE;
    op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&ctrl3_c, 1);
  }

  return op_status;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Bus_Mode_Set(LSM6DSO_Bus_Mode val)
{
  LSM6DSO_I3C_Bus_Avb i3c_bus_avb;
  LSM6DSO_Ctrl9_XL ctrl9_xl;
  LSM6DSO_Ctrl3_C ctrl3_c;
  LSM6DSO_Ctrl4_C ctrl4_c;
  LSM6DSO_OpStatus op_status;
  uint8_t bit_val;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&ctrl9_xl, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  bit_val = ((uint8_t)val.ui_bus_md & 0x04U) >> 2;
  if ( ctrl9_xl.i3c_disable != bit_val )
  {
    ctrl9_xl.i3c_disable = bit_val;

    op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&ctrl9_xl, 1);
    if (op_status < LSM6DSO_OK) {
      return op_status;
    }
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_I3C_BUS_AVB, (uint8_t*)&i3c_bus_avb, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  bit_val = ((uint8_t)val.ui_bus_md & 0x30U) >> 4;

  if ( i3c_bus_avb.i3c_bus_avb_sel != bit_val )
  {
    i3c_bus_avb.i3c_bus_avb_sel = bit_val;

    op_status = LSM6DSO_Write_Reg(LSM6DSO_I3C_BUS_AVB, (uint8_t*)&i3c_bus_avb, 1);
    if (op_status < LSM6DSO_OK) {
      return op_status;
    }
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&ctrl4_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  bit_val = ( (uint8_t)val.ui_bus_md & 0x02U ) >> 1;
  if ( ctrl4_c.i2c_disable != bit_val )
  {
    ctrl4_c.i2c_disable = bit_val;

    op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&ctrl4_c, 1);
    if (op_status < LSM6DSO_OK) {
      return op_status;
    }
  }

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&ctrl3_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  bit_val = (uint8_t)val.ui_bus_md & 0x01U;

  if ( ctrl3_c.sim != bit_val )
  {
    ctrl3_c.sim = bit_val;

    op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&ctrl3_c, 1);
  }

  return op_status;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Bus_Mode_Get(LSM6DSO_Bus_Mode *val)
{
  LSM6DSO_I3C_Bus_Avb i3c_bus_avb;
  LSM6DSO_Ctrl9_XL ctrl9_xl;
  LSM6DSO_Ctrl3_C ctrl3_c;
  LSM6DSO_Ctrl4_C ctrl4_c;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL9_XL, (uint8_t*)&ctrl9_xl, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_I3C_BUS_AVB, (uint8_t*)&i3c_bus_avb, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&ctrl4_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&ctrl3_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  switch ( ( i3c_bus_avb.i3c_bus_avb_sel << 4 ) &
            ( ctrl9_xl.i3c_disable << 2 ) &
            ( ctrl4_c.i2c_disable << 1) & ctrl3_c.sim )
            {
              case LSM6DSO_SEL_BY_HW:
                val->ui_bus_md = LSM6DSO_SEL_BY_HW;
                break;
              case LSM6DSO_SPI_4W:
                val->ui_bus_md = LSM6DSO_SPI_4W;
                break;
              case LSM6DSO_SPI_3W:
                val->ui_bus_md = LSM6DSO_SPI_3W;
                break;
              case LSM6DSO_I2C:
                val->ui_bus_md = LSM6DSO_I2C;
                break;
              case LSM6DSO_I3C_T_50us:
                val->ui_bus_md = LSM6DSO_I3C_T_50us;
                break;
              case LSM6DSO_I3C_T_2us:
                val->ui_bus_md = LSM6DSO_I3C_T_2us;
                break;
              case LSM6DSO_I3C_T_1ms:
                val->ui_bus_md = LSM6DSO_I3C_T_1ms;
                break;
              case LSM6DSO_I3C_T_25ms:
                val->ui_bus_md = LSM6DSO_I3C_T_25ms;
                break;
              default:
                val->ui_bus_md = LSM6DSO_SEL_BY_HW;
                break;
  }
  
  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Status_Get(LSM6DSO_Status *val)
{
  LSM6DSO_Status_Reg          status_reg;
  LSM6DSO_Ctrl3_C             ctrl3_c;
  LSM6DSO_OpStatus        op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&ctrl3_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  val->sw_reset = ctrl3_c.sw_reset;
  val->boot = ctrl3_c.boot;

  if ( ( ctrl3_c.sw_reset == PROPERTY_DISABLE ) && ( ctrl3_c.boot == PROPERTY_DISABLE ) )
  {
    op_status = LSM6DSO_Read_Reg(LSM6DSO_STATUS_REG, (uint8_t*)&status_reg, 1);
    if (op_status < LSM6DSO_OK) {
      return op_status;
    }

    val->drdy_xl   = status_reg.xlda;
    val->drdy_g    = status_reg.gda;
    val->drdy_temp = status_reg.tda;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Conf_Set(LSM6DSO_Pin_Conf val)
{
  LSM6DSO_I3C_Bus_Avb i3c_bus_avb;
  LSM6DSO_Pin_Ctrl pin_ctrl;
  LSM6DSO_Ctrl3_C ctrl3_c;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_PIN_CTRL, (uint8_t*)&pin_ctrl, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  pin_ctrl.ois_pu_dis = ~val.aux_sdo_ocs_pull_up;
  pin_ctrl.sdo_pu_en  = val.sdo_sa0_pull_up;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PIN_CTRL, (uint8_t*)&pin_ctrl, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&ctrl3_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  ctrl3_c.pp_od = ~val.int1_int2_push_pull;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&ctrl3_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_I3C_BUS_AVB, (uint8_t*)&i3c_bus_avb, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  i3c_bus_avb.pd_dis_int1 = ~val.int1_pull_down;

  return LSM6DSO_Write_Reg(LSM6DSO_I3C_BUS_AVB, (uint8_t*)&i3c_bus_avb, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Conf_Get(LSM6DSO_Pin_Conf *val)
{
  LSM6DSO_I3C_Bus_Avb i3c_bus_avb;
  LSM6DSO_Pin_Ctrl pin_ctrl;
  LSM6DSO_Ctrl3_C ctrl3_c;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_PIN_CTRL, (uint8_t*)&pin_ctrl, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  val->aux_sdo_ocs_pull_up = ~pin_ctrl.ois_pu_dis;
  val->aux_sdo_ocs_pull_up =  pin_ctrl.sdo_pu_en;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&ctrl3_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  val->int1_int2_push_pull = ~ctrl3_c.pp_od;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_I3C_BUS_AVB, (uint8_t*)&i3c_bus_avb, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  val->int1_pull_down = ~i3c_bus_avb.pd_dis_int1;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Interrupt_Mode_Set(LSM6DSO_Int_Mode val)
{
  LSM6DSO_Tap_Cfg0 tap_cfg0;
  LSM6DSO_Page_RW page_rw;
  LSM6DSO_Ctrl3_C ctrl3_c;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&ctrl3_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  ctrl3_c.h_lactive = val.active_low;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&ctrl3_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG0, (uint8_t*) &tap_cfg0, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  tap_cfg0.lir = val.base_latched;
  tap_cfg0.int_clr_on_read = val.base_latched | val.emb_latched;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_TAP_CFG0, (uint8_t*) &tap_cfg0, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  page_rw.emb_func_lir = val.emb_latched;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Interrupt_Mode_Get(LSM6DSO_Int_Mode *val)
{
  LSM6DSO_Tap_Cfg0 tap_cfg0;
  LSM6DSO_Page_RW page_rw;
  LSM6DSO_Ctrl3_C ctrl3_c;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL3_C, (uint8_t*)&ctrl3_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  ctrl3_c.h_lactive = val->active_low;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG0, (uint8_t*) &tap_cfg0, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  tap_cfg0.lir = val->base_latched;
  tap_cfg0.int_clr_on_read = val->base_latched | val->emb_latched;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  page_rw.emb_func_lir = val->emb_latched;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_PAGE_RW, (uint8_t*) &page_rw, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Int1_Route_Set(LSM6DSO_Pin_Int1_Route val)
{
  LSM6DSO_Pin_Int2_Route  pin_int2_route;
  LSM6DSO_EMB_Func_Int1   emb_func_int1;
  LSM6DSO_FSM_Int1_A      fsm_int1_a;
  LSM6DSO_FSM_Int1_B      fsm_int1_b;
  LSM6DSO_Int1_Ctrl       int1_ctrl;
  LSM6DSO_Int2_Ctrl       int2_ctrl;
  LSM6DSO_Tap_Cfg2        tap_cfg2;
  LSM6DSO_MD2_Cfg         md2_cfg;
  LSM6DSO_MD1_Cfg         md1_cfg;
  LSM6DSO_Ctrl4_C         ctrl4_c;
  LSM6DSO_OpStatus    op_status;

  int1_ctrl.int1_drdy_xl   = val.drdy_xl;
  int1_ctrl.int1_drdy_g    = val.drdy_g;
  int1_ctrl.int1_boot      = val.boot;
  int1_ctrl.int1_fifo_th   = val.fifo_th;
  int1_ctrl.int1_fifo_ovr  = val.fifo_ovr;
  int1_ctrl.int1_fifo_full = val.fifo_full;
  int1_ctrl.int1_cnt_bdr   = val.fifo_bdr;
  int1_ctrl.den_drdy_flag  = val.den_flag;

  md1_cfg.int1_shub         = val.sh_endop;
  md1_cfg.int1_6d           = val.six_d;
  md1_cfg.int1_double_tap   = val.double_tap;
  md1_cfg.int1_ff           = val.free_fall;
  md1_cfg.int1_wu           = val.wake_up;
  md1_cfg.int1_single_tap   = val.single_tap;
  md1_cfg.int1_sleep_change = val.sleep_change;

  emb_func_int1.not_used_01 = 0;
  emb_func_int1.int1_step_detector = val.step_detector;
  emb_func_int1.int1_tilt          = val.tilt;
  emb_func_int1.int1_sig_mot       = val.sig_mot;
  emb_func_int1.not_used_02 = 0;
  emb_func_int1.int1_fsm_lc        = val.fsm_lc;

  fsm_int1_a.int1_fsm1 = val.fsm1;
  fsm_int1_a.int1_fsm2 = val.fsm2;
  fsm_int1_a.int1_fsm3 = val.fsm3;
  fsm_int1_a.int1_fsm4 = val.fsm4;
  fsm_int1_a.int1_fsm5 = val.fsm5;
  fsm_int1_a.int1_fsm6 = val.fsm6;
  fsm_int1_a.int1_fsm7 = val.fsm7;
  fsm_int1_a.int1_fsm8 = val.fsm8;

  fsm_int1_b.int1_fsm9  = val.fsm9 ;
  fsm_int1_b.int1_fsm10 = val.fsm10;
  fsm_int1_b.int1_fsm11 = val.fsm11;
  fsm_int1_b.int1_fsm12 = val.fsm12;
  fsm_int1_b.int1_fsm13 = val.fsm13;
  fsm_int1_b.int1_fsm14 = val.fsm14;
  fsm_int1_b.int1_fsm15 = val.fsm15;
  fsm_int1_b.int1_fsm16 = val.fsm16;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&ctrl4_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  if( ( val.drdy_temp | val.timestamp ) != PROPERTY_DISABLE)
  {
    ctrl4_c.int2_on_int1 = PROPERTY_ENABLE;
  }
  else{
    ctrl4_c.int2_on_int1 = PROPERTY_DISABLE;
  }

  op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&ctrl4_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_EMB_FUNC_INT1, (uint8_t*)&emb_func_int1, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_FSM_INT1_A, (uint8_t*)&fsm_int1_a, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_FSM_INT1_B, (uint8_t*)&fsm_int1_b, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  if ( ( emb_func_int1.int1_fsm_lc
        | emb_func_int1.int1_sig_mot
        | emb_func_int1.int1_step_detector
        | emb_func_int1.int1_tilt
        | fsm_int1_a.int1_fsm1
        | fsm_int1_a.int1_fsm2
        | fsm_int1_a.int1_fsm3
        | fsm_int1_a.int1_fsm4
        | fsm_int1_a.int1_fsm5
        | fsm_int1_a.int1_fsm6
        | fsm_int1_a.int1_fsm7
        | fsm_int1_a.int1_fsm8
        | fsm_int1_b.int1_fsm9
        | fsm_int1_b.int1_fsm10
        | fsm_int1_b.int1_fsm11
        | fsm_int1_b.int1_fsm12
        | fsm_int1_b.int1_fsm13
        | fsm_int1_b.int1_fsm14
        | fsm_int1_b.int1_fsm15
        | fsm_int1_b.int1_fsm16) != PROPERTY_DISABLE)
        {
          md1_cfg.int1_emb_func = PROPERTY_ENABLE;
  }
  else{
    md1_cfg.int1_emb_func = PROPERTY_DISABLE;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_INT1_CTRL, (uint8_t*)&int1_ctrl, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_MD1_CFG, (uint8_t*)&md1_cfg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT2_CTRL, (uint8_t*)&int2_ctrl, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  int2_ctrl.int2_drdy_temp = val.drdy_temp;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_INT2_CTRL, (uint8_t*)&int2_ctrl, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MD2_CFG, (uint8_t*)&md2_cfg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  md2_cfg.int2_timestamp = val.timestamp;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_MD2_CFG, (uint8_t*)&md2_cfg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG2, (uint8_t*) &tap_cfg2, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Reg_Pin_Int2_Route_Get(&pin_int2_route);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  if ( ( pin_int2_route.fifo_bdr
        | pin_int2_route.drdy_g
        | pin_int2_route.drdy_temp
        | pin_int2_route.drdy_xl
        | pin_int2_route.fifo_full
        | pin_int2_route.fifo_ovr
        | pin_int2_route.fifo_th
        | pin_int2_route.six_d
        | pin_int2_route.double_tap
        | pin_int2_route.free_fall
        | pin_int2_route.wake_up
        | pin_int2_route.single_tap
        | pin_int2_route.sleep_change
        | int1_ctrl.den_drdy_flag
        | int1_ctrl.int1_boot
        | int1_ctrl.int1_cnt_bdr
        | int1_ctrl.int1_drdy_g
        | int1_ctrl.int1_drdy_xl
        | int1_ctrl.int1_fifo_full
        | int1_ctrl.int1_fifo_ovr
        | int1_ctrl.int1_fifo_th
        | md1_cfg.int1_shub
        | md1_cfg.int1_6d
        | md1_cfg.int1_double_tap
        | md1_cfg.int1_ff
        | md1_cfg.int1_wu
        | md1_cfg.int1_single_tap
        | md1_cfg.int1_sleep_change) != PROPERTY_DISABLE)
        {
          tap_cfg2.interrupts_enable = PROPERTY_ENABLE;
    }
    else{
      tap_cfg2.interrupts_enable = PROPERTY_DISABLE;
    }

    return LSM6DSO_Write_Reg(LSM6DSO_TAP_CFG2, (uint8_t*) &tap_cfg2, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Int1_Route_Get(LSM6DSO_Pin_Int1_Route *val)
{
  LSM6DSO_EMB_Func_Int1   emb_func_int1;
  LSM6DSO_FSM_Int1_A      fsm_int1_a;
  LSM6DSO_FSM_Int1_B      fsm_int1_b;
  LSM6DSO_Int1_Ctrl       int1_ctrl;
  LSM6DSO_Int2_Ctrl       int2_ctrl;
  LSM6DSO_MD2_Cfg         md2_cfg;
  LSM6DSO_MD1_Cfg         md1_cfg;
  LSM6DSO_Ctrl4_C         ctrl4_c;
  LSM6DSO_OpStatus    op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_INT1, (uint8_t*)&emb_func_int1, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_FSM_INT1_A, (uint8_t*)&fsm_int1_a, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_FSM_INT1_B, (uint8_t*)&fsm_int1_b, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT1_CTRL, (uint8_t*)&int1_ctrl, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MD1_CFG, (uint8_t*)&md1_cfg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&ctrl4_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  if (ctrl4_c.int2_on_int1 == PROPERTY_ENABLE)
  {
    op_status = LSM6DSO_Read_Reg(LSM6DSO_INT2_CTRL, (uint8_t*)&int2_ctrl, 1);
    if (op_status < LSM6DSO_OK) {
      return op_status;
    }
  
    val->drdy_temp = int2_ctrl.int2_drdy_temp;
      
    op_status = LSM6DSO_Read_Reg(LSM6DSO_MD2_CFG, (uint8_t*)&md2_cfg, 1);
    if (op_status < LSM6DSO_OK) {
      return op_status;
    }
  
    val->timestamp = md2_cfg.int2_timestamp;
  }
  else {
    val->drdy_temp = PROPERTY_DISABLE;
    val->timestamp = PROPERTY_DISABLE;
  }

  val->drdy_xl   = int1_ctrl.int1_drdy_xl;
  val->drdy_g    = int1_ctrl.int1_drdy_g;
  val->boot      = int1_ctrl.int1_boot;
  val->fifo_th   = int1_ctrl.int1_fifo_th;
  val->fifo_ovr  = int1_ctrl.int1_fifo_ovr;
  val->fifo_full = int1_ctrl.int1_fifo_full;
  val->fifo_bdr  = int1_ctrl.int1_cnt_bdr;
  val->den_flag  = int1_ctrl.den_drdy_flag;

  val->sh_endop     = md1_cfg.int1_shub;
  val->six_d        = md1_cfg.int1_6d;
  val->double_tap   = md1_cfg.int1_double_tap;
  val->free_fall    = md1_cfg.int1_ff;
  val->wake_up      = md1_cfg.int1_wu;
  val->single_tap   = md1_cfg.int1_single_tap;
  val->sleep_change = md1_cfg.int1_sleep_change;

  val->step_detector = emb_func_int1.int1_step_detector;
  val->tilt          = emb_func_int1.int1_tilt;
  val->sig_mot       = emb_func_int1.int1_sig_mot;
  val->fsm_lc        = emb_func_int1.int1_fsm_lc;

  val->fsm1 = fsm_int1_a.int1_fsm1;
  val->fsm2 = fsm_int1_a.int1_fsm2;
  val->fsm3 = fsm_int1_a.int1_fsm3;
  val->fsm4 = fsm_int1_a.int1_fsm4;
  val->fsm5 = fsm_int1_a.int1_fsm5;
  val->fsm6 = fsm_int1_a.int1_fsm6;
  val->fsm7 = fsm_int1_a.int1_fsm7;
  val->fsm8 = fsm_int1_a.int1_fsm8;

  val->fsm9  = fsm_int1_b.int1_fsm9;
  val->fsm10 = fsm_int1_b.int1_fsm10;
  val->fsm11 = fsm_int1_b.int1_fsm11;
  val->fsm12 = fsm_int1_b.int1_fsm12;
  val->fsm13 = fsm_int1_b.int1_fsm13;
  val->fsm14 = fsm_int1_b.int1_fsm14;
  val->fsm15 = fsm_int1_b.int1_fsm15;
  val->fsm16 = fsm_int1_b.int1_fsm16;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Int2_Route_Set(LSM6DSO_Pin_Int2_Route val)
{
  LSM6DSO_Pin_Int1_Route pin_int1_route;
  LSM6DSO_EMB_Func_Int2  emb_func_int2;
  LSM6DSO_FSM_Int2_A     fsm_int2_a;
  LSM6DSO_FSM_Int2_B     fsm_int2_b;
  LSM6DSO_Int2_Ctrl      int2_ctrl;
  LSM6DSO_Tap_Cfg2       tap_cfg2;
  LSM6DSO_MD2_Cfg        md2_cfg;
  LSM6DSO_Ctrl4_C        ctrl4_c;
  LSM6DSO_OpStatus   op_status;

  int2_ctrl.int2_drdy_xl   = val.drdy_xl;
  int2_ctrl.int2_drdy_g    = val.drdy_g;
  int2_ctrl.int2_drdy_temp = val.drdy_temp;
  int2_ctrl.int2_fifo_th   = val.fifo_th;
  int2_ctrl.int2_fifo_ovr  = val.fifo_ovr;
  int2_ctrl.int2_fifo_full = val.fifo_full;
  int2_ctrl.int2_cnt_bdr   = val.fifo_bdr;
  int2_ctrl.not_used_01    = 0;

  md2_cfg.int2_timestamp    = val.timestamp;
  md2_cfg.int2_6d           = val.six_d;
  md2_cfg.int2_double_tap   = val.double_tap;
  md2_cfg.int2_ff           = val.free_fall;
  md2_cfg.int2_wu           = val.wake_up;
  md2_cfg.int2_single_tap   = val.single_tap;
  md2_cfg.int2_sleep_change = val.sleep_change;

  emb_func_int2.not_used_01 = 0;
  emb_func_int2. int2_step_detector = val.step_detector;
  emb_func_int2.int2_tilt           = val.tilt;
  emb_func_int2.int2_sig_mot        = val.sig_mot;
  emb_func_int2.not_used_02 = 0;
  emb_func_int2.int2_fsm_lc         = val.fsm_lc;

  fsm_int2_a.int2_fsm1 = val.fsm1;
  fsm_int2_a.int2_fsm2 = val.fsm2;
  fsm_int2_a.int2_fsm3 = val.fsm3;
  fsm_int2_a.int2_fsm4 = val.fsm4;
  fsm_int2_a.int2_fsm5 = val.fsm5;
  fsm_int2_a.int2_fsm6 = val.fsm6;
  fsm_int2_a.int2_fsm7 = val.fsm7;
  fsm_int2_a.int2_fsm8 = val.fsm8;

  fsm_int2_b.int2_fsm9  = val.fsm9 ;
  fsm_int2_b.int2_fsm10 = val.fsm10;
  fsm_int2_b.int2_fsm11 = val.fsm11;
  fsm_int2_b.int2_fsm12 = val.fsm12;
  fsm_int2_b.int2_fsm13 = val.fsm13;
  fsm_int2_b.int2_fsm14 = val.fsm14;
  fsm_int2_b.int2_fsm15 = val.fsm15;
  fsm_int2_b.int2_fsm16 = val.fsm16;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&ctrl4_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  if ( ( val.drdy_temp | val.timestamp ) != PROPERTY_DISABLE )
  {
    ctrl4_c.int2_on_int1 = PROPERTY_DISABLE;
  }

  op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&ctrl4_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_EMB_FUNC_INT2, (uint8_t*)&emb_func_int2, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_FSM_INT2_A, (uint8_t*)&fsm_int2_a, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_FSM_INT2_B, (uint8_t*)&fsm_int2_b, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  if (( emb_func_int2.int2_fsm_lc
      | emb_func_int2.int2_sig_mot
      | emb_func_int2.int2_step_detector
      | emb_func_int2.int2_tilt
      | fsm_int2_a.int2_fsm1
      | fsm_int2_a.int2_fsm2
      | fsm_int2_a.int2_fsm3
      | fsm_int2_a.int2_fsm4
      | fsm_int2_a.int2_fsm5
      | fsm_int2_a.int2_fsm6
      | fsm_int2_a.int2_fsm7
      | fsm_int2_a.int2_fsm8
      | fsm_int2_b.int2_fsm9
      | fsm_int2_b.int2_fsm10
      | fsm_int2_b.int2_fsm11
      | fsm_int2_b.int2_fsm12
      | fsm_int2_b.int2_fsm13
      | fsm_int2_b.int2_fsm14
      | fsm_int2_b.int2_fsm15
      | fsm_int2_b.int2_fsm16)!= PROPERTY_DISABLE )
      {
        md2_cfg.int2_emb_func = PROPERTY_ENABLE;
  }
  else{
    md2_cfg.int2_emb_func = PROPERTY_DISABLE;
  }

  op_status = LSM6DSO_Write_Reg(LSM6DSO_INT2_CTRL, (uint8_t*)&int2_ctrl, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_MD2_CFG, (uint8_t*)&md2_cfg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_TAP_CFG2, (uint8_t*) &tap_cfg2, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Reg_Pin_Int1_Route_Get(&pin_int1_route);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  if ( ( val.fifo_bdr
        | val.drdy_g
        | val.drdy_temp
        | val.drdy_xl
        | val.fifo_full
        | val.fifo_ovr
        | val.fifo_th
        | val.six_d
        | val.double_tap
        | val.free_fall
        | val.wake_up
        | val.single_tap
        | val.sleep_change
        | pin_int1_route.den_flag
        | pin_int1_route.boot
        | pin_int1_route.fifo_bdr
        | pin_int1_route.drdy_g
        | pin_int1_route.drdy_xl
        | pin_int1_route.fifo_full
        | pin_int1_route.fifo_ovr
        | pin_int1_route.fifo_th
        | pin_int1_route.six_d
        | pin_int1_route.double_tap
        | pin_int1_route.free_fall
        | pin_int1_route.wake_up
        | pin_int1_route.single_tap
        | pin_int1_route.sleep_change ) != PROPERTY_DISABLE)
        {
          tap_cfg2.interrupts_enable = PROPERTY_ENABLE;
  }
  else{
    tap_cfg2.interrupts_enable = PROPERTY_DISABLE;
  }
  
  return LSM6DSO_Write_Reg(LSM6DSO_TAP_CFG2, (uint8_t*) &tap_cfg2, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Int2_Route_Get(LSM6DSO_Pin_Int2_Route *val)
{
  LSM6DSO_EMB_Func_Int2  emb_func_int2;
  LSM6DSO_FSM_Int2_A     fsm_int2_a;
  LSM6DSO_FSM_Int2_B     fsm_int2_b;
  LSM6DSO_Int2_Ctrl      int2_ctrl;
  LSM6DSO_MD2_Cfg        md2_cfg;
  LSM6DSO_Ctrl4_C        ctrl4_c;
  LSM6DSO_OpStatus   op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_INT2, (uint8_t*)&emb_func_int2, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_FSM_INT2_A, (uint8_t*)&fsm_int2_a, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_FSM_INT2_B, (uint8_t*)&fsm_int2_b, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_INT2_CTRL, (uint8_t*)&int2_ctrl, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_MD2_CFG, (uint8_t*)&md2_cfg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL4_C, (uint8_t*)&ctrl4_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  if (ctrl4_c.int2_on_int1 == PROPERTY_DISABLE)
  {
    op_status = LSM6DSO_Read_Reg(LSM6DSO_INT2_CTRL, (uint8_t*)&int2_ctrl, 1);
    if (op_status < LSM6DSO_OK) {
      return op_status;
    }

    val->drdy_temp = int2_ctrl.int2_drdy_temp;

    op_status = LSM6DSO_Read_Reg(LSM6DSO_MD2_CFG, (uint8_t*)&md2_cfg, 1);
    if (op_status < LSM6DSO_OK) {
      return op_status;
    }
  
    val->timestamp = md2_cfg.int2_timestamp;
  }
  else {
    val->drdy_temp = PROPERTY_DISABLE;
    val->timestamp = PROPERTY_DISABLE;
  }

  val->drdy_xl   = int2_ctrl.int2_drdy_xl;
  val->drdy_g    = int2_ctrl.int2_drdy_g;
  val->drdy_temp = int2_ctrl.int2_drdy_temp;
  val->fifo_th   = int2_ctrl.int2_fifo_th;
  val->fifo_ovr  = int2_ctrl.int2_fifo_ovr;
  val->fifo_full = int2_ctrl.int2_fifo_full;
  val->fifo_bdr   = int2_ctrl.int2_cnt_bdr;

  val->timestamp    = md2_cfg.int2_timestamp;
  val->six_d        = md2_cfg.int2_6d;
  val->double_tap   = md2_cfg.int2_double_tap;
  val->free_fall    = md2_cfg.int2_ff;
  val->wake_up      = md2_cfg.int2_wu;
  val->single_tap   = md2_cfg.int2_single_tap;
  val->sleep_change = md2_cfg.int2_sleep_change;

  val->step_detector = emb_func_int2. int2_step_detector;
  val->tilt          = emb_func_int2.int2_tilt;
  val->fsm_lc        = emb_func_int2.int2_fsm_lc;

  val->fsm1 = fsm_int2_a.int2_fsm1;
  val->fsm2 = fsm_int2_a.int2_fsm2;
  val->fsm3 = fsm_int2_a.int2_fsm3;
  val->fsm4 = fsm_int2_a.int2_fsm4;
  val->fsm5 = fsm_int2_a.int2_fsm5;
  val->fsm6 = fsm_int2_a.int2_fsm6;
  val->fsm7 = fsm_int2_a.int2_fsm7;
  val->fsm8 = fsm_int2_a.int2_fsm8;

  val->fsm9  = fsm_int2_b.int2_fsm9;
  val->fsm10 = fsm_int2_b.int2_fsm10;
  val->fsm11 = fsm_int2_b.int2_fsm11;
  val->fsm12 = fsm_int2_b.int2_fsm12;
  val->fsm13 = fsm_int2_b.int2_fsm13;
  val->fsm14 = fsm_int2_b.int2_fsm14;
  val->fsm15 = fsm_int2_b.int2_fsm15;
  val->fsm16 = fsm_int2_b.int2_fsm16;

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_All_Sources_Get(LSM6DSO_All_Sources *val)
{
  LSM6DSO_EMB_Func_Status_MainPage emb_func_status_mainpage;
  LSM6DSO_Status_Master_MainPage   status_master_mainpage;
  LSM6DSO_FSM_Status_A_MainPage    fsm_status_a_mainpage;
  LSM6DSO_FSM_Status_B_MainPage    fsm_status_b_mainpage;
  LSM6DSO_FIFO_Status1             fifo_status1;
  LSM6DSO_FIFO_Status2             fifo_status2;
  LSM6DSO_All_Int_Src              all_int_src;
  LSM6DSO_Wake_Up_Src              wake_up_src;
  LSM6DSO_Status_Reg               status_reg;
  LSM6DSO_Tap_Src                  tap_src;
  LSM6DSO_D6D_Src                  d6d_src;
  LSM6DSO_Ctrl5_C                  ctrl5_c;
  uint8_t                          reg[12];
  LSM6DSO_OpStatus             op_status;

  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL5_C, (uint8_t*)&ctrl5_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  ctrl5_c.not_used_01 = PROPERTY_ENABLE;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL5_C, (uint8_t*)&ctrl5_c, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_ALL_INT_SRC, reg, 12);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&all_int_src, &reg[0]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&wake_up_src, &reg[1]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&tap_src, &reg[2]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&d6d_src, &reg[3]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&status_reg, &reg[4]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&emb_func_status_mainpage, &reg[5]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&fsm_status_a_mainpage, &reg[6]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&fsm_status_b_mainpage, &reg[7]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&status_master_mainpage, &reg[9]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&fifo_status1, &reg[10]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&fifo_status2, &reg[11]);

  val->timestamp = all_int_src.timestamp_endcount;

  val->wake_up_z    = wake_up_src.z_wu;
  val->wake_up_y    = wake_up_src.y_wu;
  val->wake_up_x    = wake_up_src.x_wu;
  val->wake_up      = wake_up_src.wu_ia;
  val->sleep_state  = wake_up_src.sleep_state;
  val->free_fall    = wake_up_src.ff_ia;
  val->sleep_change = wake_up_src.sleep_change_ia;

  val->tap_x      = tap_src.x_tap;
  val->tap_y      = tap_src.y_tap;
  val->tap_z      = tap_src.z_tap;
  val->tap_sign   = tap_src.tap_sign;
  val->double_tap = tap_src.double_tap;
  val->single_tap = tap_src.single_tap;

  val->six_d_xl = d6d_src.xl;
  val->six_d_xh = d6d_src.xh;
  val->six_d_yl = d6d_src.yl;
  val->six_d_yh = d6d_src.yh;
  val->six_d_zl = d6d_src.zl;
  val->six_d_zh = d6d_src.zh;
  val->six_d    = d6d_src.d6d_ia;
  val->den_flag = d6d_src.den_drdy;

  val->drdy_xl   = status_reg.xlda;
  val->drdy_g    = status_reg.gda;
  val->drdy_temp = status_reg.tda;

  val->step_detector = emb_func_status_mainpage.is_step_det;
  val->tilt          = emb_func_status_mainpage.is_tilt;
  val->sig_mot       = emb_func_status_mainpage.is_sigmot;
  val->fsm_lc        = emb_func_status_mainpage.is_fsm_lc;

  val->fsm1 = fsm_status_a_mainpage.is_fsm1;
  val->fsm2 = fsm_status_a_mainpage.is_fsm2;
  val->fsm3 = fsm_status_a_mainpage.is_fsm3;
  val->fsm4 = fsm_status_a_mainpage.is_fsm4;
  val->fsm5 = fsm_status_a_mainpage.is_fsm5;
  val->fsm6 = fsm_status_a_mainpage.is_fsm6;
  val->fsm7 = fsm_status_a_mainpage.is_fsm7;
  val->fsm8 = fsm_status_a_mainpage.is_fsm8;

  val->fsm9  = fsm_status_b_mainpage.is_fsm9;
  val->fsm10 = fsm_status_b_mainpage.is_fsm10;
  val->fsm11 = fsm_status_b_mainpage.is_fsm11;
  val->fsm12 = fsm_status_b_mainpage.is_fsm12;
  val->fsm13 = fsm_status_b_mainpage.is_fsm13;
  val->fsm14 = fsm_status_b_mainpage.is_fsm14;
  val->fsm15 = fsm_status_b_mainpage.is_fsm15;
  val->fsm16 = fsm_status_b_mainpage.is_fsm16;

  val->sh_endop       = status_master_mainpage.sens_hub_endop;
  val->sh_slave0_nack = status_master_mainpage.slave0_nack;
  val->sh_slave1_nack = status_master_mainpage.slave1_nack;
  val->sh_slave2_nack = status_master_mainpage.slave2_nack;
  val->sh_slave3_nack = status_master_mainpage.slave3_nack;
  val->sh_wr_once     = status_master_mainpage.wr_once_done;

  val->fifo_diff = (256U * fifo_status2.diff_fifo) + fifo_status1.diff_fifo;

  val->fifo_ovr_latched = fifo_status2.over_run_latched;
  val->fifo_bdr         = fifo_status2.counter_bdr_ia;
  val->fifo_full        = fifo_status2.fifo_full_ia;
  val->fifo_ovr         = fifo_status2.fifo_ovr_ia;
  val->fifo_th          = fifo_status2.fifo_wtm_ia;

  ctrl5_c.not_used_01 = PROPERTY_DISABLE;

  return LSM6DSO_Write_Reg(LSM6DSO_CTRL5_C, (uint8_t*)&ctrl5_c, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mode_Set(LSM6DSO_Mode *val)
{
  LSM6DSO_Func_Cfg_Access func_cfg_access;
  LSM6DSO_Ctrl1_Ois ctrl1_ois;
  LSM6DSO_Ctrl2_Ois ctrl2_ois;
  LSM6DSO_Ctrl3_Ois ctrl3_ois;
  LSM6DSO_Ctrl1_XL ctrl1_xl;
  LSM6DSO_Ctrl8_XL ctrl8_xl;
  LSM6DSO_Ctrl2_G ctrl2_g;
  LSM6DSO_Ctrl3_C ctrl3_c;
  LSM6DSO_Ctrl4_C ctrl4_c;
  LSM6DSO_Ctrl5_C ctrl5_c;
  LSM6DSO_Ctrl6_C ctrl6_c;
  LSM6DSO_Ctrl7_G ctrl7_g;
  uint8_t xl_hm_mode;
  uint8_t g_hm_mode;
  uint8_t xl_ulp_en;
  uint8_t odr_gy;
  uint8_t odr_xl;
  uint8_t reg[8];
  LSM6DSO_OpStatus op_status;

  /* reading input configuration */
  xl_hm_mode = ( (uint8_t)val->ui.xl.odr & 0x10U ) >> 4;
  xl_ulp_en = ( (uint8_t)val->ui.xl.odr & 0x20U ) >> 5;
  odr_xl = (uint8_t)val->ui.xl.odr & 0x0FU;

  /* if enable xl ultra low power mode disable gy and OIS chain */
  if (xl_ulp_en == PROPERTY_ENABLE)
  {
    val->ois.xl.odr = LSM6DSO_XL_OIS_OFF;
    val->ois.gy.odr = LSM6DSO_GY_OIS_OFF;
    val->ui.gy.odr  = LSM6DSO_GY_UI_OFF;
  }

  /* if OIS xl is enabled also gyro OIS is enabled */
  if (val->ois.xl.odr == LSM6DSO_XL_OIS_6667Hz_HP)
  {
    val->ois.gy.odr = LSM6DSO_GY_OIS_6667Hz_HP;
  }

  g_hm_mode = ( (uint8_t)val->ui.gy.odr & 0x10U ) >> 4;
  odr_gy = (uint8_t)val->ui.gy.odr & 0x0FU;

  /* reading registers to be configured */
  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_XL, reg, 8);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl1_xl, &reg[0]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl2_g,  &reg[1]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl3_c,  &reg[2]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl4_c,  &reg[3]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl5_c,  &reg[4]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl6_c,  &reg[5]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl7_g,  &reg[6]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl8_xl, &reg[7]);

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FUNC_CFG_ACCESS, (uint8_t*)&func_cfg_access, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  /* if toggle xl ultra low power mode, turn off xl before reconfigure */
  if (ctrl5_c.xl_ulp_en != xl_ulp_en)
  {
    ctrl1_xl.odr_xl = (uint8_t) 0x00U;
    op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL1_XL, (uint8_t*)&ctrl1_xl, 1);
    if (op_status < LSM6DSO_OK) {
      return op_status;
    }
  }

  /* reading OIS registers to be configured */
  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_OIS, reg, 3);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl1_ois, &reg[0]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl2_ois, &reg[1]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl3_ois, &reg[2]);

  /* Check the Finite State Machine data rate constraints */
  if (val->fsm.sens != LSM6DSO_FSM_DISABLE) {
    switch (val->fsm.odr) {
      case LSM6DSO_FSM_12Hz5:
        if ( (val->fsm.sens != LSM6DSO_FSM_GY) && (odr_xl == 0x00U) )
        {
          odr_xl = 0x01U;
        }
        if ( (val->fsm.sens != LSM6DSO_FSM_XL) && (odr_gy == 0x00U) )
        {
          xl_ulp_en = PROPERTY_DISABLE;
          odr_gy = 0x01U;
        }
        break;
      case LSM6DSO_FSM_26Hz:
        if ( (val->fsm.sens != LSM6DSO_FSM_GY) && (odr_xl < 0x02U) )
        {
          odr_xl = 0x02U;
        }
        if ( (val->fsm.sens != LSM6DSO_FSM_XL) && (odr_gy < 0x02U) )
        {
          xl_ulp_en = PROPERTY_DISABLE;
          odr_gy = 0x02U;
        }
        break;
      case LSM6DSO_FSM_52Hz:
        if ( (val->fsm.sens != LSM6DSO_FSM_GY) && (odr_xl < 0x03U) )
        {
          odr_xl = 0x03U;
        }
        if ( (val->fsm.sens != LSM6DSO_FSM_XL) && (odr_gy < 0x03U) )
        {
          xl_ulp_en = PROPERTY_DISABLE;
          odr_gy = 0x03U;
        }
        break;
      case LSM6DSO_FSM_104Hz:
        if ( (val->fsm.sens != LSM6DSO_FSM_GY) && (odr_xl < 0x04U) )
        {
          odr_xl = 0x04U;
        }
        if ( (val->fsm.sens != LSM6DSO_FSM_XL) && (odr_gy < 0x04U) )
        {
          xl_ulp_en = PROPERTY_DISABLE;
          odr_gy = 0x04U;
        }
        break;
      default:
        odr_xl = 0x00U;
        odr_gy = 0x00U;
        break;
    }
  }

  /* Updating the accelerometer data rate configuration */
  switch ( ( ctrl5_c.xl_ulp_en << 5 ) | ( ctrl6_c.xl_hm_mode << 4 ) | ctrl1_xl.odr_xl )
  {
    case LSM6DSO_XL_UI_OFF:
      val->ui.xl.odr = LSM6DSO_XL_UI_OFF;
      break;
    case LSM6DSO_XL_UI_12Hz5_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_12Hz5_HP;
      break;
    case LSM6DSO_XL_UI_26Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_26Hz_HP;
      break;
    case LSM6DSO_XL_UI_52Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_52Hz_HP;
      break;
    case LSM6DSO_XL_UI_104Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_104Hz_HP;
      break;
    case LSM6DSO_XL_UI_208Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_208Hz_HP;
      break;
    case LSM6DSO_XL_UI_416Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_416Hz_HP;
      break;
    case LSM6DSO_XL_UI_833Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_833Hz_HP;
      break;
    case LSM6DSO_XL_UI_1667Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_1667Hz_HP;
      break;
    case LSM6DSO_XL_UI_3333Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_3333Hz_HP;
      break;
    case LSM6DSO_XL_UI_6667Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_6667Hz_HP;
      break;
    case LSM6DSO_XL_UI_1Hz6_LP:
      val->ui.xl.odr = LSM6DSO_XL_UI_1Hz6_LP;
      break;
    case LSM6DSO_XL_UI_12Hz5_LP:
      val->ui.xl.odr = LSM6DSO_XL_UI_12Hz5_LP;
      break;
    case LSM6DSO_XL_UI_26Hz_LP:
      val->ui.xl.odr = LSM6DSO_XL_UI_26Hz_LP;
      break;
    case LSM6DSO_XL_UI_52Hz_LP:
      val->ui.xl.odr = LSM6DSO_XL_UI_52Hz_LP;
      break;
    case LSM6DSO_XL_UI_104Hz_NM:
      val->ui.xl.odr = LSM6DSO_XL_UI_104Hz_NM;
      break;
    case LSM6DSO_XL_UI_208Hz_NM:
      val->ui.xl.odr = LSM6DSO_XL_UI_208Hz_NM;
      break;
    case LSM6DSO_XL_UI_1Hz6_ULP:
      val->ui.xl.odr = LSM6DSO_XL_UI_1Hz6_ULP;
      break;
    case LSM6DSO_XL_UI_12Hz5_ULP:
      val->ui.xl.odr = LSM6DSO_XL_UI_12Hz5_ULP;
      break;
    case LSM6DSO_XL_UI_26Hz_ULP:
      val->ui.xl.odr = LSM6DSO_XL_UI_26Hz_ULP;
      break;
    case LSM6DSO_XL_UI_52Hz_ULP:
      val->ui.xl.odr = LSM6DSO_XL_UI_52Hz_ULP;
      break;
    case LSM6DSO_XL_UI_104Hz_ULP:
      val->ui.xl.odr = LSM6DSO_XL_UI_104Hz_ULP;
      break;
    case LSM6DSO_XL_UI_208Hz_ULP:
      val->ui.xl.odr = LSM6DSO_XL_UI_208Hz_ULP;
      break;
    default:
      val->ui.xl.odr = LSM6DSO_XL_UI_OFF;
      break;
  }

  /* Updating the accelerometer data rate configuration */
  switch ( (ctrl7_g.g_hm_mode << 4) | ctrl2_g.odr_g)
  {
    case LSM6DSO_GY_UI_OFF:
      val->ui.gy.odr = LSM6DSO_GY_UI_OFF;
      break;
    case LSM6DSO_GY_UI_12Hz5_LP:
      val->ui.gy.odr = LSM6DSO_GY_UI_12Hz5_LP;
      break;
    case LSM6DSO_GY_UI_12Hz5_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_12Hz5_HP;
      break;
    case LSM6DSO_GY_UI_26Hz_LP:
      val->ui.gy.odr = LSM6DSO_GY_UI_26Hz_LP;
      break;
    case LSM6DSO_GY_UI_26Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_26Hz_HP;
      break;
    case LSM6DSO_GY_UI_52Hz_LP:
      val->ui.gy.odr = LSM6DSO_GY_UI_52Hz_LP;
      break;
    case LSM6DSO_GY_UI_52Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_52Hz_HP;
      break;
    case LSM6DSO_GY_UI_104Hz_NM:
      val->ui.gy.odr = LSM6DSO_GY_UI_104Hz_NM;
      break;
    case LSM6DSO_GY_UI_104Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_104Hz_HP;
      break;
    case LSM6DSO_GY_UI_208Hz_NM:
      val->ui.gy.odr = LSM6DSO_GY_UI_208Hz_NM;
      break;
    case LSM6DSO_GY_UI_208Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_208Hz_HP;
      break;
    case LSM6DSO_GY_UI_416Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_416Hz_HP;
      break;
    case LSM6DSO_GY_UI_833Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_833Hz_HP;
      break;
    case LSM6DSO_GY_UI_1667Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_1667Hz_HP;
      break;
    case LSM6DSO_GY_UI_3333Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_3333Hz_HP;
      break;
    case LSM6DSO_GY_UI_6667Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_6667Hz_HP;
      break;
    default:
      val->ui.gy.odr = LSM6DSO_GY_UI_OFF;
      break;
  }

  /* Check accelerometer full scale constraints */
  /* Full scale of 16g must be the same for UI and OIS */
  if ( (val->ui.xl.fs == LSM6DSO_XL_UI_16g) || (val->ois.xl.fs == LSM6DSO_XL_OIS_16g) )
  {
    val->ui.xl.fs = LSM6DSO_XL_UI_16g;
    val->ois.xl.fs = LSM6DSO_XL_OIS_16g;
  }

  /* prapare new configuration */

  /* Full scale of 16g must be the same for UI and OIS */
  if (val->ui.xl.fs == LSM6DSO_XL_UI_16g)
  {
    ctrl8_xl.xl_fs_mode = PROPERTY_DISABLE;
  }
  else {
    ctrl8_xl.xl_fs_mode = PROPERTY_ENABLE;
  }

  /* OIS new configuration */
  ctrl7_g.ois_on_en = val->ois.ctrl_md & 0x01U;

  switch (val->ois.ctrl_md)
  {
    case LSM6DSO_OIS_ONLY_AUX:
      ctrl1_ois.fs_g_ois = (uint8_t)val->ois.gy.fs;
      ctrl1_ois.ois_en_spi2 = (uint8_t)val->ois.gy.odr | (uint8_t)val->ois.xl.odr;
      ctrl1_ois.mode4_en = (uint8_t) val->ois.xl.odr;
      ctrl3_ois.fs_xl_ois = (uint8_t)val->ois.xl.fs;
      break;
    case LSM6DSO_OIS_MIXED:
      ctrl1_ois.fs_g_ois = (uint8_t)val->ois.gy.fs;
      ctrl7_g.ois_on = (uint8_t)val->ois.gy.odr | (uint8_t)val->ois.xl.odr;
      ctrl1_ois.mode4_en = (uint8_t) val->ois.xl.odr;
      ctrl3_ois.fs_xl_ois = (uint8_t)val->ois.xl.fs;
      break;
    default:
      ctrl1_ois.fs_g_ois = (uint8_t)val->ois.gy.fs;
      ctrl1_ois.ois_en_spi2 = (uint8_t)val->ois.gy.odr | (uint8_t)val->ois.xl.odr;
      ctrl1_ois.mode4_en = (uint8_t) val->ois.xl.odr;
      ctrl3_ois.fs_xl_ois = (uint8_t)val->ois.xl.fs;
      break;
  }

  /* UI new configuration */
  ctrl1_xl.odr_xl = odr_xl;
  ctrl1_xl.fs_xl = (uint8_t)val->ui.xl.fs;
  ctrl5_c.xl_ulp_en = xl_ulp_en;
  ctrl6_c.xl_hm_mode = xl_hm_mode;
  ctrl7_g.g_hm_mode = g_hm_mode;
  ctrl2_g.odr_g = odr_gy;
  ctrl2_g.fs_g = (uint8_t) val->ui.gy.fs;

  /* writing checked configuration */
  LSM6DSO_BYTE_COPY_INTERNAL(&reg[0], ( uint8_t*)&ctrl1_xl);
  LSM6DSO_BYTE_COPY_INTERNAL(&reg[1], ( uint8_t*)&ctrl2_g);
  LSM6DSO_BYTE_COPY_INTERNAL(&reg[2], ( uint8_t*)&ctrl3_c);
  LSM6DSO_BYTE_COPY_INTERNAL(&reg[3], ( uint8_t*)&ctrl4_c);
  LSM6DSO_BYTE_COPY_INTERNAL(&reg[4], ( uint8_t*)&ctrl5_c);
  LSM6DSO_BYTE_COPY_INTERNAL(&reg[5], ( uint8_t*)&ctrl6_c);
  LSM6DSO_BYTE_COPY_INTERNAL(&reg[6], ( uint8_t*)&ctrl7_g);
  LSM6DSO_BYTE_COPY_INTERNAL(&reg[7], ( uint8_t*)&ctrl8_xl);

  op_status = LSM6DSO_Write_Reg(LSM6DSO_CTRL1_XL, (uint8_t*)&reg, 8);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Write_Reg(LSM6DSO_FUNC_CFG_ACCESS,  (uint8_t*)&func_cfg_access, 1);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Mode_Get(LSM6DSO_Mode *val)
{
  LSM6DSO_EMB_Func_ODR_Cfg_B emb_func_odr_cfg_b;
  LSM6DSO_Func_Cfg_Access func_cfg_access;
  LSM6DSO_EMB_Func_En_B emb_func_en_b;
  LSM6DSO_FSM_Enable_A fsm_enable_a;
  LSM6DSO_FSM_Enable_B fsm_enable_b;
  LSM6DSO_Ctrl1_Ois ctrl1_ois;
  LSM6DSO_Ctrl2_Ois ctrl2_ois;
  LSM6DSO_Ctrl3_Ois ctrl3_ois;
  LSM6DSO_Ctrl1_XL ctrl1_xl;
  LSM6DSO_Ctrl2_G ctrl2_g;
  LSM6DSO_Ctrl3_C ctrl3_c;
  LSM6DSO_Ctrl4_C ctrl4_c;
  LSM6DSO_Ctrl5_C ctrl5_c;
  LSM6DSO_Ctrl6_C ctrl6_c;
  LSM6DSO_Ctrl7_G ctrl7_g;

  uint8_t reg[8];
  LSM6DSO_OpStatus op_status;

  /* reading the registers of the device */
  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_XL, reg, 7);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl1_xl, &reg[0]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl2_g,  &reg[1]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl3_c,  &reg[2]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl4_c,  &reg[3]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl5_c,  &reg[4]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl6_c,  &reg[5]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl7_g,  &reg[6]);

  op_status = LSM6DSO_Read_Reg(LSM6DSO_FUNC_CFG_ACCESS, (uint8_t*)&func_cfg_access, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_ODR_CFG_B, reg, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&emb_func_odr_cfg_b, &reg[0]);

  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_EN_B, (uint8_t*)&emb_func_en_b, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_FSM_ENABLE_A, reg, 2);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&fsm_enable_a, &reg[0]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&fsm_enable_b, &reg[1]);

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_CTRL1_OIS, reg, 3);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl1_ois, &reg[0]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl2_ois, &reg[1]);
  LSM6DSO_BYTE_COPY_INTERNAL(( uint8_t*)&ctrl3_ois, &reg[2]);
  
  /* fill the input structure */

  /* get accelerometer configuration */
  switch ( (ctrl5_c.xl_ulp_en << 5) | (ctrl6_c.xl_hm_mode << 4) | ctrl1_xl.odr_xl )
  {
    case LSM6DSO_XL_UI_OFF:
      val->ui.xl.odr = LSM6DSO_XL_UI_OFF;
      break;
    case LSM6DSO_XL_UI_12Hz5_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_12Hz5_HP;
      break;
    case LSM6DSO_XL_UI_26Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_26Hz_HP;
      break;
    case LSM6DSO_XL_UI_52Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_52Hz_HP;
      break;
    case LSM6DSO_XL_UI_104Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_104Hz_HP;
      break;
    case LSM6DSO_XL_UI_208Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_208Hz_HP;
      break;
    case LSM6DSO_XL_UI_416Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_416Hz_HP;
      break;
    case LSM6DSO_XL_UI_833Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_833Hz_HP;
      break;
    case LSM6DSO_XL_UI_1667Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_1667Hz_HP;
      break;
    case LSM6DSO_XL_UI_3333Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_3333Hz_HP;
      break;
    case LSM6DSO_XL_UI_6667Hz_HP:
      val->ui.xl.odr = LSM6DSO_XL_UI_6667Hz_HP;
      break;
    case LSM6DSO_XL_UI_1Hz6_LP:
      val->ui.xl.odr = LSM6DSO_XL_UI_1Hz6_LP;
      break;
    case LSM6DSO_XL_UI_12Hz5_LP:
      val->ui.xl.odr = LSM6DSO_XL_UI_12Hz5_LP;
      break;
    case LSM6DSO_XL_UI_26Hz_LP:
      val->ui.xl.odr = LSM6DSO_XL_UI_26Hz_LP;
      break;
    case LSM6DSO_XL_UI_52Hz_LP:
      val->ui.xl.odr = LSM6DSO_XL_UI_52Hz_LP;
      break;
    case LSM6DSO_XL_UI_104Hz_NM:
      val->ui.xl.odr = LSM6DSO_XL_UI_104Hz_NM;
      break;
    case LSM6DSO_XL_UI_208Hz_NM:
      val->ui.xl.odr = LSM6DSO_XL_UI_208Hz_NM;
      break;
    case LSM6DSO_XL_UI_1Hz6_ULP:
      val->ui.xl.odr = LSM6DSO_XL_UI_1Hz6_ULP;
      break;
    case LSM6DSO_XL_UI_12Hz5_ULP:
      val->ui.xl.odr = LSM6DSO_XL_UI_12Hz5_ULP;
      break;
    case LSM6DSO_XL_UI_26Hz_ULP:
      val->ui.xl.odr = LSM6DSO_XL_UI_26Hz_ULP;
      break;
    case LSM6DSO_XL_UI_52Hz_ULP:
      val->ui.xl.odr = LSM6DSO_XL_UI_52Hz_ULP;
      break;
    case LSM6DSO_XL_UI_104Hz_ULP:
      val->ui.xl.odr = LSM6DSO_XL_UI_104Hz_ULP;
      break;
    case LSM6DSO_XL_UI_208Hz_ULP:
      val->ui.xl.odr = LSM6DSO_XL_UI_208Hz_ULP;
      break;
    default:
      val->ui.xl.odr = LSM6DSO_XL_UI_OFF;
      break;
  }

  switch ( ctrl1_xl.fs_xl )
  {
    case LSM6DSO_XL_UI_2g:
      val->ui.xl.fs = LSM6DSO_XL_UI_2g;
      break;
    case LSM6DSO_XL_UI_4g:
      val->ui.xl.fs = LSM6DSO_XL_UI_4g;
      break;
    case LSM6DSO_XL_UI_8g:
      val->ui.xl.fs = LSM6DSO_XL_UI_8g;
      break;
    case LSM6DSO_XL_UI_16g:
      val->ui.xl.fs = LSM6DSO_XL_UI_16g;
      break;
    default:
      val->ui.xl.fs = LSM6DSO_XL_UI_2g;
      break;
  }

  /* get gyroscope configuration */
  switch ( (ctrl7_g.g_hm_mode << 4) | ctrl2_g.odr_g)
  {
    case LSM6DSO_GY_UI_OFF:
      val->ui.gy.odr = LSM6DSO_GY_UI_OFF;
      break;
    case LSM6DSO_GY_UI_12Hz5_LP:
      val->ui.gy.odr = LSM6DSO_GY_UI_12Hz5_LP;
      break;
    case LSM6DSO_GY_UI_12Hz5_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_12Hz5_HP;
      break;
    case LSM6DSO_GY_UI_26Hz_LP:
      val->ui.gy.odr = LSM6DSO_GY_UI_26Hz_LP;
      break;
    case LSM6DSO_GY_UI_26Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_26Hz_HP;
      break;
    case LSM6DSO_GY_UI_52Hz_LP:
      val->ui.gy.odr = LSM6DSO_GY_UI_52Hz_LP;
      break;
    case LSM6DSO_GY_UI_52Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_52Hz_HP;
      break;
    case LSM6DSO_GY_UI_104Hz_NM:
      val->ui.gy.odr = LSM6DSO_GY_UI_104Hz_NM;
      break;
    case LSM6DSO_GY_UI_104Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_104Hz_HP;
      break;
    case LSM6DSO_GY_UI_208Hz_NM:
      val->ui.gy.odr = LSM6DSO_GY_UI_208Hz_NM;
      break;
    case LSM6DSO_GY_UI_208Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_208Hz_HP;
      break;
    case LSM6DSO_GY_UI_416Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_416Hz_HP;
      break;
    case LSM6DSO_GY_UI_833Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_833Hz_HP;
      break;
    case LSM6DSO_GY_UI_1667Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_1667Hz_HP;
      break;
    case LSM6DSO_GY_UI_3333Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_3333Hz_HP;
      break;
    case LSM6DSO_GY_UI_6667Hz_HP:
      val->ui.gy.odr = LSM6DSO_GY_UI_6667Hz_HP;
      break;
    default:
      val->ui.gy.odr = LSM6DSO_GY_UI_OFF;
      break;
  }

  switch (ctrl2_g.fs_g)
  {
    case LSM6DSO_GY_UI_125dps:
      val->ui.gy.fs = LSM6DSO_GY_UI_125dps;
      break;
    case LSM6DSO_GY_UI_250dps:
      val->ui.gy.fs = LSM6DSO_GY_UI_250dps;
      break;
    case LSM6DSO_GY_UI_500dps:
      val->ui.gy.fs = LSM6DSO_GY_UI_500dps;
      break;
    case LSM6DSO_GY_UI_1000dps:
      val->ui.gy.fs = LSM6DSO_GY_UI_1000dps;
      break;
    case LSM6DSO_GY_UI_2000dps:
      val->ui.gy.fs = LSM6DSO_GY_UI_2000dps;
      break;
    default:
      val->ui.gy.fs = LSM6DSO_GY_UI_125dps;
      break;
  }

  /* get finite state machine configuration */
  if ( (fsm_enable_a.fsm1_en | fsm_enable_a.fsm2_en | fsm_enable_a.fsm3_en |
        fsm_enable_a.fsm4_en | fsm_enable_a.fsm5_en | fsm_enable_a.fsm6_en |
        fsm_enable_a.fsm7_en | fsm_enable_a.fsm8_en | fsm_enable_b.fsm9_en |
        fsm_enable_b.fsm10_en | fsm_enable_b.fsm11_en |
        fsm_enable_b.fsm12_en | fsm_enable_b.fsm13_en |
        fsm_enable_b.fsm14_en | fsm_enable_b.fsm15_en |
        fsm_enable_b.fsm16_en) == PROPERTY_ENABLE )
        {
          switch (emb_func_odr_cfg_b.fsm_odr) {
            case LSM6DSO_FSM_12Hz5:
              val->fsm.odr = LSM6DSO_FSM_12Hz5;
              break;
            case LSM6DSO_FSM_26Hz:
              val->fsm.odr = LSM6DSO_FSM_26Hz;
              break;
            case LSM6DSO_FSM_52Hz:
              val->fsm.odr = LSM6DSO_FSM_52Hz;
              break;
            case LSM6DSO_FSM_104Hz:
              val->fsm.odr = LSM6DSO_FSM_104Hz;
              break;
            default:
              val->fsm.odr = LSM6DSO_FSM_12Hz5;
              break;
          }

          val->fsm.sens = LSM6DSO_FSM_XL_GY;
          if (val->ui.gy.odr == LSM6DSO_GY_UI_OFF) {
            val->fsm.sens = LSM6DSO_FSM_XL;
          }
          if (val->ui.xl.odr == LSM6DSO_XL_UI_OFF) {
            val->fsm.sens = LSM6DSO_FSM_GY;
          }
  }
  else {
    val->fsm.sens = LSM6DSO_FSM_DISABLE;
  }

  /* get ois configuration */

  /* OIS configuration mode */
  switch ( ctrl7_g.ois_on_en )
  {
    case LSM6DSO_OIS_ONLY_AUX:
      switch ( ctrl3_ois.fs_xl_ois ) {
        case LSM6DSO_XL_OIS_2g:
          val->ois.xl.fs = LSM6DSO_XL_OIS_2g;
          break;
        case LSM6DSO_XL_OIS_4g:
          val->ois.xl.fs = LSM6DSO_XL_OIS_4g;
          break;
        case LSM6DSO_XL_OIS_8g:
          val->ois.xl.fs = LSM6DSO_XL_OIS_8g;
          break;
        case LSM6DSO_XL_OIS_16g:
          val->ois.xl.fs = LSM6DSO_XL_OIS_16g;
          break;
        default:
          val->ois.xl.fs = LSM6DSO_XL_OIS_2g;
          break;
      }
      switch ( ctrl1_ois.mode4_en ) {
        case LSM6DSO_XL_OIS_OFF:
          val->ois.xl.odr = LSM6DSO_XL_OIS_OFF;
          break;
        case LSM6DSO_XL_OIS_6667Hz_HP:
          val->ois.xl.odr = LSM6DSO_XL_OIS_6667Hz_HP;
          break;
        default:
          val->ois.xl.odr = LSM6DSO_XL_OIS_OFF;
          break;
      }
      switch ( ctrl1_ois.fs_g_ois ) {
        case LSM6DSO_GY_OIS_250dps:
          val->ois.gy.fs = LSM6DSO_GY_OIS_250dps;
          break;
        case LSM6DSO_GY_OIS_500dps:
          val->ois.gy.fs = LSM6DSO_GY_OIS_500dps;
          break;
        case LSM6DSO_GY_OIS_1000dps:
          val->ois.gy.fs = LSM6DSO_GY_OIS_1000dps;
          break;
        case LSM6DSO_GY_OIS_2000dps:
          val->ois.gy.fs = LSM6DSO_GY_OIS_2000dps;
          break;
        default:
          val->ois.gy.fs = LSM6DSO_GY_OIS_250dps;
          break;
      }
      switch ( ctrl1_ois.ois_en_spi2 ) {
        case LSM6DSO_GY_OIS_OFF:
          val->ois.gy.odr = LSM6DSO_GY_OIS_OFF;
          break;
        case LSM6DSO_GY_OIS_6667Hz_HP:
          val->ois.gy.odr = LSM6DSO_GY_OIS_6667Hz_HP;
          break;
        default:
          val->ois.gy.odr = LSM6DSO_GY_OIS_OFF;
          break;
      }
      val->ois.ctrl_md = LSM6DSO_OIS_ONLY_AUX;
      break;
    case LSM6DSO_OIS_MIXED:
      switch ( ctrl3_ois.fs_xl_ois ) {
        case LSM6DSO_XL_OIS_2g:
          val->ois.xl.fs = LSM6DSO_XL_OIS_2g;
          break;
        case LSM6DSO_XL_OIS_4g:
          val->ois.xl.fs = LSM6DSO_XL_OIS_4g;
          break;
        case LSM6DSO_XL_OIS_8g:
          val->ois.xl.fs = LSM6DSO_XL_OIS_8g;
          break;
        case LSM6DSO_XL_OIS_16g:
          val->ois.xl.fs = LSM6DSO_XL_OIS_16g;
          break;
        default:
          val->ois.xl.fs = LSM6DSO_XL_OIS_2g;
          break;
      }
      switch ( ctrl1_ois.mode4_en ) {
        case LSM6DSO_XL_OIS_OFF:
          val->ois.xl.odr = LSM6DSO_XL_OIS_OFF;
          break;
        case LSM6DSO_XL_OIS_6667Hz_HP:
          val->ois.xl.odr = LSM6DSO_XL_OIS_6667Hz_HP;
          break;
        default:
          val->ois.xl.odr = LSM6DSO_XL_OIS_OFF;
          break;
      }
      switch ( ctrl1_ois.fs_g_ois ) {
        case LSM6DSO_GY_OIS_250dps:
          val->ois.gy.fs = LSM6DSO_GY_OIS_250dps;
          break;
        case LSM6DSO_GY_OIS_500dps:
          val->ois.gy.fs = LSM6DSO_GY_OIS_500dps;
          break;
        case LSM6DSO_GY_OIS_1000dps:
          val->ois.gy.fs = LSM6DSO_GY_OIS_1000dps;
          break;
        case LSM6DSO_GY_OIS_2000dps:
          val->ois.gy.fs = LSM6DSO_GY_OIS_2000dps;
          break;
        default:
          val->ois.gy.fs = LSM6DSO_GY_OIS_250dps;
          break;
      }
      switch ( ctrl1_ois.ois_en_spi2 ) {
        case LSM6DSO_GY_OIS_OFF:
          val->ois.gy.odr = LSM6DSO_GY_OIS_OFF;
          break;
        case LSM6DSO_GY_OIS_6667Hz_HP:
          val->ois.gy.odr = LSM6DSO_GY_OIS_6667Hz_HP;
          break;
        default:
          val->ois.gy.odr = LSM6DSO_GY_OIS_OFF;
          break;
      }
      val->ois.ctrl_md = LSM6DSO_OIS_MIXED;
      break;
    default:
      ctrl1_ois.fs_g_ois = (uint8_t)val->ois.gy.fs;
      ctrl1_ois.ois_en_spi2 = (uint8_t)val->ois.gy.odr | (uint8_t)val->ois.xl.odr;
      ctrl1_ois.mode4_en = (uint8_t) val->ois.xl.odr;
      ctrl3_ois.fs_xl_ois = (uint8_t)val->ois.xl.fs;
      val->ois.ctrl_md = LSM6DSO_OIS_ONLY_AUX;
      break;
  }

  return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Data_Get(LSM6DSO_Mode *md, LSM6DSO_Data *data)
{
  uint8_t buff[14];
  uint8_t i;
  uint8_t j;
  LSM6DSO_OpStatus op_status;
  
  /* read data */
  op_status = LSM6DSO_Read_Reg(LSM6DSO_OUT_TEMP_L, buff, 14);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  j = 0;

  /* temperature conversion */
  data->ui.heat.raw = (int16_t)buff[j+1U];
  data->ui.heat.raw = ( ((int16_t)data->ui.heat.raw * (int16_t)256) + (int16_t)buff[j] );
  j+=2U;
  data->ui.heat.deg_c = LSM6DSO_From_LSB_To_Celsius((int16_t)data->ui.heat.raw);

  /* angular rate conversion */
  for (i = 0U; i < 3U; i++) {
    data->ui.gy.raw[i] = (int16_t)buff[j+1U];
    data->ui.gy.raw[i] = (data->ui.gy.raw[i] * 256) + (int16_t) buff[j];
    j+=2U;
    switch ( md->ui.gy.fs ) {
      case LSM6DSO_GY_UI_250dps:
        data->ui.gy.mdps[i] = LSM6DSO_From_FS250_To_Mdps(data->ui.gy.raw[i]);
        break;
      case LSM6DSO_GY_UI_125dps:
        data->ui.gy.mdps[i] = LSM6DSO_From_FS125_To_Mdps(data->ui.gy.raw[i]);
        break;
      case LSM6DSO_GY_UI_500dps:
        data->ui.gy.mdps[i] = LSM6DSO_From_FS500_To_Mdps(data->ui.gy.raw[i]);
        break;
      case LSM6DSO_GY_UI_1000dps:
        data->ui.gy.mdps[i] = LSM6DSO_From_FS1000_To_Mdps(data->ui.gy.raw[i]);
        break;
      case LSM6DSO_GY_UI_2000dps:
        data->ui.gy.mdps[i] = LSM6DSO_From_FS2000_To_Mdps(data->ui.gy.raw[i]);
        break;
      default:
        data->ui.gy.mdps[i] = 0.0f;
        break;
    }
  }

  /* acceleration conversion */
  for (i = 0U; i < 3U; i++) {
    data->ui.xl.raw[i] = (int16_t)buff[j+1U];
    data->ui.xl.raw[i] = (data->ui.xl.raw[i] * 256) + (int16_t) buff[j];
    j+=2U;
    switch ( md->ui.xl.fs ) {
      case LSM6DSO_XL_UI_2g:
        data->ui.xl.mg[i] = LSM6DSO_From_FS2_To_Mg(data->ui.xl.raw[i]);
        break;
      case LSM6DSO_XL_UI_4g:
        data->ui.xl.mg[i] = LSM6DSO_From_FS4_To_Mg(data->ui.xl.raw[i]);
        break;
      case LSM6DSO_XL_UI_8g:
        data->ui.xl.mg[i] = LSM6DSO_From_FS8_To_Mg(data->ui.xl.raw[i]);
        break;
      case LSM6DSO_XL_UI_16g:
        data->ui.xl.mg[i] = LSM6DSO_From_FS16_To_Mg(data->ui.xl.raw[i]);
        break;
      default:
        data->ui.xl.mg[i] = 0.0f;
        break;
    }
    
  }

  j = 0;

  /* ois angular rate conversion */
  for (i = 0U; i < 3U; i++) {
    data->ois.gy.raw[i] = (int16_t) buff[j+1U];
    data->ois.gy.raw[i] = (data->ois.gy.raw[i] * 256) + (int16_t) buff[j];
    j+=2U;
    switch ( md->ois.gy.fs ) {
      case LSM6DSO_GY_UI_250dps:
        data->ois.gy.mdps[i] = LSM6DSO_From_FS250_To_Mdps(data->ois.gy.raw[i]);
        break;
      case LSM6DSO_GY_UI_125dps:
        data->ois.gy.mdps[i] = LSM6DSO_From_FS125_To_Mdps(data->ois.gy.raw[i]);
        break;
      case LSM6DSO_GY_UI_500dps:
        data->ois.gy.mdps[i] = LSM6DSO_From_FS500_To_Mdps(data->ois.gy.raw[i]);
        break;
      case LSM6DSO_GY_UI_1000dps:
        data->ois.gy.mdps[i] = LSM6DSO_From_FS1000_To_Mdps(data->ois.gy.raw[i]);
        break;
      case LSM6DSO_GY_UI_2000dps:
        data->ois.gy.mdps[i] = LSM6DSO_From_FS2000_To_Mdps(data->ois.gy.raw[i]);
        break;
      default:
        data->ois.gy.mdps[i] = 0.0f;
        break;
    }
  }

  /* ois acceleration conversion */
  for (i = 0U; i < 3U; i++) {
    data->ois.xl.raw[i] = (int16_t) buff[j+1U];
    data->ois.xl.raw[i] = (data->ois.xl.raw[i] * 256) + (int16_t) buff[j];
    j+=2U;
    switch ( md->ois.xl.fs ) {
      case LSM6DSO_XL_UI_2g:
        data->ois.xl.mg[i] = LSM6DSO_From_FS2_To_Mg(data->ois.xl.raw[i]);
        break;
      case LSM6DSO_XL_UI_4g:
        data->ois.xl.mg[i] = LSM6DSO_From_FS4_To_Mg(data->ois.xl.raw[i]);
        break;
      case LSM6DSO_XL_UI_8g:
        data->ois.xl.mg[i] = LSM6DSO_From_FS8_To_Mg(data->ois.xl.raw[i]);
        break;
      case LSM6DSO_XL_UI_16g:
        data->ois.xl.mg[i] = LSM6DSO_From_FS16_To_Mg(data->ois.xl.raw[i]);
        break;
      default:
        data->ois.xl.mg[i] = 0.0f;
        break;
    }
  }

  return op_status;
}

LSM6DSO_OpStatus LSM6DSO_Reg_Embedded_Sens_Set(LSM6DSO_Emb_Sens *val)
{
  LSM6DSO_EMB_Func_En_A emb_func_en_a;
  LSM6DSO_EMB_Func_En_B emb_func_en_b;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_EN_A, (uint8_t*)&emb_func_en_a, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_EN_B, (uint8_t*)&emb_func_en_b, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  emb_func_en_b.fsm_en = val->fsm;
  emb_func_en_a.tilt_en = val->tilt;
  emb_func_en_a.pedo_en = val->step;
  emb_func_en_b.pedo_adv_en = val->step_adv;
  emb_func_en_a.sign_motion_en = val->sig_mot;
  emb_func_en_b.fifo_compr_en = val->fifo_compr;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_EMB_FUNC_EN_A, (uint8_t*)&emb_func_en_a, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_EMB_FUNC_EN_B, (uint8_t*)&emb_func_en_b, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Embedded_Sens_Get(LSM6DSO_Emb_Sens *emb_sens)
{
  LSM6DSO_EMB_Func_En_A emb_func_en_a;
  LSM6DSO_EMB_Func_En_B emb_func_en_b;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_EN_A, (uint8_t*)&emb_func_en_a, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_EN_B, (uint8_t*)&emb_func_en_b, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }

  emb_sens->fsm = emb_func_en_b.fsm_en;
  emb_sens->tilt = emb_func_en_a.tilt_en;
  emb_sens->step = emb_func_en_a.pedo_en;
  emb_sens->step_adv = emb_func_en_b.pedo_adv_en;
  emb_sens->sig_mot = emb_func_en_a.sign_motion_en;
  emb_sens->fifo_compr = emb_func_en_b.fifo_compr_en;

  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}

LSM6DSO_OpStatus LSM6DSO_Reg_Embedded_Sens_Off()
{
  LSM6DSO_EMB_Func_En_A emb_func_en_a;
  LSM6DSO_EMB_Func_En_B emb_func_en_b;
  LSM6DSO_OpStatus op_status;

  op_status = LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_EMBEDDED_FUNC_BANK);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_EN_A, (uint8_t*)&emb_func_en_a, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Read_Reg(LSM6DSO_EMB_FUNC_EN_B, (uint8_t*)&emb_func_en_b, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  emb_func_en_b.fsm_en = PROPERTY_DISABLE;
  emb_func_en_a.tilt_en = PROPERTY_DISABLE;
  emb_func_en_a.pedo_en = PROPERTY_DISABLE;
  emb_func_en_b.pedo_adv_en = PROPERTY_DISABLE;
  emb_func_en_a.sign_motion_en = PROPERTY_DISABLE;
  emb_func_en_b.fifo_compr_en = PROPERTY_DISABLE;

  op_status = LSM6DSO_Write_Reg(LSM6DSO_EMB_FUNC_EN_A, (uint8_t*)&emb_func_en_a, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  op_status = LSM6DSO_Write_Reg(LSM6DSO_EMB_FUNC_EN_B, (uint8_t*)&emb_func_en_b, 1);
  if (op_status < LSM6DSO_OK) {
    return op_status;
  }
  
  return LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_USER_BANK);
}