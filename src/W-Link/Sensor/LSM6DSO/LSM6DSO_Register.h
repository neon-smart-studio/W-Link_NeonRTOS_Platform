/*
This LSM6DSO driver is based on STMicroelectronics stm32-lsm6dso / LSM6DSO component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef LSM6DSO_REGISTER_H
#define LSM6DSO_REGISTER_H

#include "LSM6DSO_IO.h"
#include "LSM6DSO_Util.h"

#include "LSM6DSO_Def.h"

#include "LSM6DSO_Register_Def.h"

LSM6DSO_OpStatus LSM6DSO_Read_Reg(uint8_t reg, uint8_t* data, uint16_t len);
LSM6DSO_OpStatus LSM6DSO_Write_Reg(uint8_t reg, uint8_t* data, uint16_t len);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Full_Scale_Set(LSM6DSO_FS_XL val);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_Full_Scale_Get(LSM6DSO_FS_XL *val);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Data_Rate_Set(LSM6DSO_ODR_XL val);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_Data_Rate_Get(LSM6DSO_ODR_XL *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Full_Scale_Set(LSM6DSO_FS_G val);
LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Full_Scale_Get(LSM6DSO_FS_G *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Data_Rate_Set(LSM6DSO_ODR_G val);
LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Data_Rate_Get(LSM6DSO_ODR_G *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Block_Data_Update_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Block_Data_Update_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Offset_Weight_Set(LSM6DSO_Usr_Off_W val);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_Offset_Weight_Get(LSM6DSO_Usr_Off_W *val);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Power_Mode_Set(LSM6DSO_XL_HM_Mode val);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_Power_Mode_Get(LSM6DSO_XL_HM_Mode *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Power_Mode_Set(LSM6DSO_G_HM_Mode val);
LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Power_Mode_Get(LSM6DSO_G_HM_Mode *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Status_Reg_Get(LSM6DSO_Status_Reg *val);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Flag_Data_Ready_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Flag_Data_Ready_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Temp_Flag_Data_Ready_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_X_Set(uint8_t *buff);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_X_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_Y_Set(uint8_t *buff);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_Y_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_Z_Set(uint8_t *buff);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_Z_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_TimeStamp_Rst();

LSM6DSO_OpStatus LSM6DSO_Reg_TimeStamp_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_TimeStamp_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_TimeStamp_Raw_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_Rounding_Mode_Set(LSM6DSO_Rounding val);
LSM6DSO_OpStatus LSM6DSO_Reg_Rounding_Mode_Get(LSM6DSO_Rounding *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Temperature_Raw_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_Angular_Rate_Raw_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_Acceleration_Raw_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Out_Raw_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_Number_Of_Steps_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_Steps_Reset();

LSM6DSO_OpStatus LSM6DSO_Reg_ODR_Calibration_Reg_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_ODR_Calibration_Reg_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Mem_Bank_Set(LSM6DSO_Reg_Access val);
LSM6DSO_OpStatus LSM6DSO_Reg_Mem_Bank_Get(LSM6DSO_Reg_Access *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Ln_Page_Write_Byte(uint16_t address, uint8_t *val);
LSM6DSO_OpStatus LSM6DSO_Reg_Ln_Page_Write(uint16_t address, uint8_t *buf, uint8_t len);
LSM6DSO_OpStatus LSM6DSO_Reg_Ln_Page_Read_Byte(uint16_t address, uint8_t *val);
LSM6DSO_OpStatus LSM6DSO_Reg_Ln_Page_Read(uint16_t address, uint8_t *buf, uint8_t len);

LSM6DSO_OpStatus LSM6DSO_Reg_Data_Ready_Mode_Set(LSM6DSO_DataReady_Pulsed val);
LSM6DSO_OpStatus LSM6DSO_Reg_Data_Ready_Mode_Get(LSM6DSO_DataReady_Pulsed *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Device_ID_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_Reset_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Reset_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Auto_Increment_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Auto_Increment_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Boot_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Boot_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Self_Test_Set(LSM6DSO_ST_XL val);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_Self_Test_Get(LSM6DSO_ST_XL *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Self_Test_Set(LSM6DSO_ST_G val);
LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Self_Test_Get(LSM6DSO_ST_G *val);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Filter_LP2_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_Filter_LP2_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Filter_LP1_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Filter_LP1_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Filter_Settling_Mask_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Filter_Settling_Mask_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_LP1_BandWidth_Set(LSM6DSO_fType_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Gy_LP1_BandWidth_Get(LSM6DSO_fType_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_LP2_On_6D_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_LP2_On_6D_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_HP_Path_On_Out_Set(LSM6DSO_HP_Slope_XL_En val);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_HP_Path_On_Out_Get(LSM6DSO_HP_Slope_XL_En *val);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Fast_Settling_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_Fast_Settling_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_HP_Path_Internal_Set(LSM6DSO_Slope_Fds val);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_HP_Path_Internal_Get(LSM6DSO_Slope_Fds *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_HP_Path_Internal_Set(LSM6DSO_HPM_G val);
LSM6DSO_OpStatus LSM6DSO_Reg_Gy_HP_Path_Internal_Get(LSM6DSO_HPM_G *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_SDO_OCS_Mode_Set(LSM6DSO_Ois_PU_Dis val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_SDO_OCS_Mode_Get(LSM6DSO_Ois_PU_Dis *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Pwr_On_Ctrl_Set(LSM6DSO_Ois_On val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Pwr_On_Ctrl_Get(LSM6DSO_Ois_On *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_FS_Mode_Set(LSM6DSO_XL_FS_Mode val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_FS_Mode_Get(LSM6DSO_XL_FS_Mode *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Status_Get(LSM6DSO_Status_SpiAux *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_Flag_Data_Ready_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Flag_Data_Ready_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Flag_Settling_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_Self_Test_Set(LSM6DSO_St_XL_Ois val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_Self_Test_Get(LSM6DSO_St_XL_Ois *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Den_Polarity_Set(LSM6DSO_Den_LH_Ois val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Den_Polarity_Get(LSM6DSO_Den_LH_Ois *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Den_Mode_Set(LSM6DSO_Lvl2_Ois val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Den_Mode_Get(LSM6DSO_Lvl2_Ois *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_dRdy_On_Int2_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_dRdy_On_Int2_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Mode_Set(LSM6DSO_Ois_En_SPI2 val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Mode_Get(LSM6DSO_Ois_En_SPI2 *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Full_Scale_Set(LSM6DSO_FS_G_Ois val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Full_Scale_Get(LSM6DSO_FS_G_Ois *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_SPI_Mode_Set(LSM6DSO_Sim_Ois val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_SPI_Mode_Get(LSM6DSO_Sim_Ois *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_LP1_BandWidth_Set(LSM6DSO_fType_Ois val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_LP1_BandWidth_Get(LSM6DSO_fType_Ois *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_HP_BandWidth_Set(LSM6DSO_HPM_Ois val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_HP_BandWidth_Get(LSM6DSO_HPM_Ois *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Clamp_Set(LSM6DSO_ST_Ois_ClampDis val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Clamp_Get(LSM6DSO_ST_Ois_ClampDis *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Self_Test_Set(LSM6DSO_ST_Ois val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_Gy_Self_Test_Get(LSM6DSO_ST_Ois *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_BandWidth_Set(LSM6DSO_Filter_XL_Conf_Ois val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_BandWidth_Get(LSM6DSO_Filter_XL_Conf_Ois *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_Full_Scale_Set(LSM6DSO_FS_XL_Ois val);
LSM6DSO_OpStatus LSM6DSO_Reg_Aux_XL_Full_Scale_Get(LSM6DSO_FS_XL_Ois *val);

LSM6DSO_OpStatus LSM6DSO_Reg_SDO_SA0_Mode_Set(LSM6DSO_SDO_PU_En val);
LSM6DSO_OpStatus LSM6DSO_Reg_SDO_SA0_Mode_Get(LSM6DSO_SDO_PU_En *val);

LSM6DSO_OpStatus LSM6DSO_Reg_SPI_Mode_Set(LSM6DSO_Sim val);
LSM6DSO_OpStatus LSM6DSO_Reg_SPI_Mode_Get(LSM6DSO_Sim *val);

LSM6DSO_OpStatus LSM6DSO_Reg_I2C_Interface_Set(LSM6DSO_I2C_Disable val);
LSM6DSO_OpStatus LSM6DSO_Reg_I2C_Interface_Get(LSM6DSO_I2C_Disable *val);

LSM6DSO_OpStatus LSM6DSO_Reg_I3C_Disable_Set(LSM6DSO_I3C_Disable val);
LSM6DSO_OpStatus LSM6DSO_Reg_I3C_Disable_Get(LSM6DSO_I3C_Disable *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Int1_Mode_Set(LSM6DSO_Int1_PD_En val);
LSM6DSO_OpStatus LSM6DSO_Reg_Int1_Mode_Get(LSM6DSO_Int1_PD_En *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Mode_Set(LSM6DSO_PP_OD val);
LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Mode_Get(LSM6DSO_PP_OD *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Polarity_Set(LSM6DSO_HL_Active val);
LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Polarity_Get(LSM6DSO_HL_Active *val);

LSM6DSO_OpStatus LSM6DSO_Reg_All_On_Int1_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_All_On_Int1_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Int_Notification_Set(LSM6DSO_LIR val);
LSM6DSO_OpStatus LSM6DSO_Reg_Int_Notification_Get(LSM6DSO_LIR *val);

LSM6DSO_OpStatus LSM6DSO_Reg_WakeUp_Ths_Weight_Set(LSM6DSO_Wake_Ths_W val);
LSM6DSO_OpStatus LSM6DSO_Reg_WakeUp_Ths_Weight_Get(LSM6DSO_Wake_Ths_W *val);

LSM6DSO_OpStatus LSM6DSO_Reg_WakeUp_Threshold_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_WakeUp_Threshold_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_On_WakeUp_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_XL_Usr_Offset_On_WakeUp_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_WakeUp_Dur_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_WakeUp_Dur_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Sleep_Mode_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Gy_Sleep_Mode_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Act_Pin_Notification_Set(LSM6DSO_Sleep_Status_On_Int val);
LSM6DSO_OpStatus LSM6DSO_Reg_Act_Pin_Notification_Get(LSM6DSO_Sleep_Status_On_Int *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Act_Mode_Set(LSM6DSO_InAct_En val);
LSM6DSO_OpStatus LSM6DSO_Reg_Act_Mode_Get(LSM6DSO_InAct_En *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Act_Sleep_Dur_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Act_Sleep_Dur_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Detection_On_Z_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Detection_On_Z_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Detection_On_Y_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Detection_On_Y_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Detection_On_X_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Detection_On_X_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Threshold_X_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Threshold_X_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Axis_Priority_Set(LSM6DSO_Tap_Priority val);
LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Axis_Priority_Get(LSM6DSO_Tap_Priority *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Threshold_Y_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Threshold_Y_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Threshold_Z_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Threshold_Z_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Shock_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Shock_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Quiet_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Quiet_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Dur_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Dur_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Mode_Set(LSM6DSO_Single_Double_Tap val);
LSM6DSO_OpStatus LSM6DSO_Reg_Tap_Mode_Get(LSM6DSO_Single_Double_Tap *val);

LSM6DSO_OpStatus LSM6DSO_Reg_6D_Threshold_Set(LSM6DSO_6D_Ths val);
LSM6DSO_OpStatus LSM6DSO_Reg_6D_Threshold_Get(LSM6DSO_6D_Ths *val);

LSM6DSO_OpStatus LSM6DSO_Reg_4D_Mode_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_4D_Mode_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FF_Threshold_Set(LSM6DSO_FF_Ths val);
LSM6DSO_OpStatus LSM6DSO_Reg_FF_Threshold_Get(LSM6DSO_FF_Ths *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FF_Dur_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_FF_Dur_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_WaterMark_Set(uint16_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_WaterMark_Get(uint16_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Compression_Algo_Init_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Compression_Algo_Init_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Compression_Algo_Set(LSM6DSO_UnCoPtr_Rate val);
LSM6DSO_OpStatus LSM6DSO_Reg_Compression_Algo_Get(LSM6DSO_UnCoPtr_Rate *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Virtual_Sens_ODR_CHG_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Virtual_Sens_ODR_CHG_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Compression_Algo_Real_Time_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Compression_Algo_Real_Time_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Stop_On_WTM_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Stop_On_WTM_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_XL_Batch_Set(LSM6DSO_BDR_XL val);
LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_XL_Batch_Get(LSM6DSO_BDR_XL *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Gy_Batch_Set(LSM6DSO_BDR_Gy val);
LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Gy_Batch_Get(LSM6DSO_BDR_Gy *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Mode_Set(LSM6DSO_FIFO_Mode val);
LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Mode_Get(LSM6DSO_FIFO_Mode *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Temp_Batch_Set(LSM6DSO_ODR_Temp_Batch val);
LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Temp_Batch_Get(LSM6DSO_ODR_Temp_Batch *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_TimeStamp_Decimation_Set(LSM6DSO_ODR_TS_Batch val);
LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_TimeStamp_Decimation_Get(LSM6DSO_ODR_TS_Batch *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Cnt_Event_Batch_Set(LSM6DSO_Trig_Counter_BDR val);
LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Cnt_Event_Batch_Get(LSM6DSO_Trig_Counter_BDR *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Rst_Batch_Counter_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Rst_Batch_Counter_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Batch_Counter_Threshold_Set(uint16_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Batch_Counter_Threshold_Get(uint16_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Data_Level_Get(uint16_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Status_Get(LSM6DSO_FIFO_Status2 *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Full_Flag_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Ovr_Flag_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_WTM_Flag_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Sensor_Tag_Get(LSM6DSO_FIFO_Tag *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Pedo_Batch_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_FIFO_Pedo_Batch_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_0_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_0_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_1_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_1_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_2_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_2_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_3_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Batch_Slave_3_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mode_Set(LSM6DSO_Den_Mode val);
LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mode_Get(LSM6DSO_Den_Mode *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Polarity_Set(LSM6DSO_Den_LH val);
LSM6DSO_OpStatus LSM6DSO_Reg_Den_Polarity_Get(LSM6DSO_Den_LH *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Enable_Set(LSM6DSO_Den_XL_G val);
LSM6DSO_OpStatus LSM6DSO_Reg_Den_Enable_Get(LSM6DSO_Den_XL_G *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mark_Axis_X_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mark_Axis_X_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mark_Axis_Y_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mark_Axis_Y_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mark_Axis_Z_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Den_Mark_Axis_Z_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Sens_Set(LSM6DSO_Pedo_MD val);
LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Sens_Get(LSM6DSO_Pedo_MD *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Step_Detect_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Debounce_Steps_Set(uint8_t *buff);
LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Debounce_Steps_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Steps_Period_Set(uint8_t *buff);
LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Steps_Period_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Int_Mode_Set(LSM6DSO_Carry_Count_En val);
LSM6DSO_OpStatus LSM6DSO_Reg_Pedo_Int_Mode_Get(LSM6DSO_Carry_Count_En *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Motion_Flag_Data_Ready_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Tilt_Flag_Data_Ready_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Sensitivity_Set(uint8_t *buff);
LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Sensitivity_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Offset_Set(uint8_t *buff);
LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Offset_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Soft_Iron_Set(uint8_t *buff);
LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Soft_Iron_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Z_Orient_Set(LSM6DSO_Mag_Z_Axis val);
LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Z_Orient_Get(LSM6DSO_Mag_Z_Axis *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Y_Orient_Set(LSM6DSO_Mag_Y_Axis val);
LSM6DSO_OpStatus LSM6DSO_Reg_Mag_Y_Orient_Get(LSM6DSO_Mag_Y_Axis *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Mag_X_Orient_Set(LSM6DSO_Mag_X_Axis val);
LSM6DSO_OpStatus LSM6DSO_Reg_Mag_X_Orient_Get(LSM6DSO_Mag_X_Axis *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Long_Cnt_Flag_Data_Ready_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Enable_Set(LSM6DSO_Emb_FSM_Enable *val);
LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Enable_Get(LSM6DSO_Emb_FSM_Enable *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Long_Cnt_Set(uint8_t *buff);
LSM6DSO_OpStatus LSM6DSO_Reg_Long_Cnt_Get(uint8_t *buff);

LSM6DSO_OpStatus LSM6DSO_Reg_Long_Clr_Set(LSM6DSO_FSM_LC_Clr val);
LSM6DSO_OpStatus LSM6DSO_Reg_Long_Clr_Get(LSM6DSO_FSM_LC_Clr *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Out_Get(LSM6DSO_FSM_Out *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Data_Rate_Set(LSM6DSO_FSM_ODR val);
LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Data_Rate_Get(LSM6DSO_FSM_ODR *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Init_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Init_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Long_Cnt_Int_Value_Set(uint16_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Long_Cnt_Int_Value_Get(uint16_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Number_Of_Programs_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Number_Of_Programs_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Start_Address_Set(uint16_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_FSM_Start_Address_Get(uint16_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Read_Data_Raw_Get(uint8_t *val, uint8_t len);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Slave_Connected_Set(LSM6DSO_Aux_Sens_On val);
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Slave_Connected_Get(LSM6DSO_Aux_Sens_On *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Master_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Master_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Pin_Mode_Set(LSM6DSO_Shub_PU_En val);
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Pin_Mode_Get(LSM6DSO_Shub_PU_En *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Pass_Through_Set(uint8_t val);
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Pass_Through_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Syncro_Mode_Set(LSM6DSO_Start_Config val);
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Syncro_Mode_Get(LSM6DSO_Start_Config *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Write_Mode_Set(LSM6DSO_Write_Once val);
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Write_Mode_Get(LSM6DSO_Write_Once *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_reset_Set();
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_reset_Get(uint8_t *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Data_Rate_Set(LSM6DSO_Shub_ODR val);
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Data_Rate_Get(LSM6DSO_Shub_ODR *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Cfg_Write(LSM6DSO_Sh_Cfg_Write *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Slv0_Cfg_Read(LSM6DSO_Sh_Cfg_Read *val);
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Slv1_Cfg_Read(LSM6DSO_Sh_Cfg_Read *val);
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Slv2_Cfg_Read(LSM6DSO_Sh_Cfg_Read *val);
LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Slv3_Cfg_Read(LSM6DSO_Sh_Cfg_Read *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Sh_Status_Get(LSM6DSO_Status_Master *val);

LSM6DSO_OpStatus LSM6DSO_Reg_ID_Get(LSM6DSO_ID *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Init_Set(LSM6DSO_Init_Set val);

LSM6DSO_OpStatus LSM6DSO_Reg_Bus_Mode_Set(LSM6DSO_Bus_Mode val);
LSM6DSO_OpStatus LSM6DSO_Reg_Bus_Mode_Get(LSM6DSO_Bus_Mode *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Status_Get(LSM6DSO_Status *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Conf_Set(LSM6DSO_Pin_Conf val);
LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Conf_Get(LSM6DSO_Pin_Conf *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Interrupt_Mode_Set(LSM6DSO_Int_Mode val);
LSM6DSO_OpStatus LSM6DSO_Reg_Interrupt_Mode_Get(LSM6DSO_Int_Mode *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Int1_Route_Set(LSM6DSO_Pin_Int1_Route val);
LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Int1_Route_Get(LSM6DSO_Pin_Int1_Route *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Int2_Route_Set(LSM6DSO_Pin_Int2_Route val);
LSM6DSO_OpStatus LSM6DSO_Reg_Pin_Int2_Route_Get(LSM6DSO_Pin_Int2_Route *val);

LSM6DSO_OpStatus LSM6DSO_Reg_All_Sources_Get(LSM6DSO_All_Sources *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Mode_Set(LSM6DSO_Mode *val);
LSM6DSO_OpStatus LSM6DSO_Reg_Mode_Get(LSM6DSO_Mode *val);

LSM6DSO_OpStatus LSM6DSO_Reg_Data_Get(LSM6DSO_Mode *md, LSM6DSO_Data *data);

LSM6DSO_OpStatus LSM6DSO_Reg_Embedded_Sens_Set(LSM6DSO_Emb_Sens *emb_sens);
LSM6DSO_OpStatus LSM6DSO_Reg_Embedded_Sens_Get(LSM6DSO_Emb_Sens *emb_sens);
LSM6DSO_OpStatus LSM6DSO_Reg_Embedded_Sens_Off();

#endif //LSM6DSO_REGISTER_H