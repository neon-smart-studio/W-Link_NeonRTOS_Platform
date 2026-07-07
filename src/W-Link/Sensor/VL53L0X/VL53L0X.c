/**
 ******************************************************************************
 * @file    vl53l0x_class.cpp
 * @author  IMG
 * @version V0.0.1
 * @date    28-June-2016
 * @brief   Implementation file for the VL53L0X driver class
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
*/
/*
 * Based on STMicroelectronics VL53L0X driver
 * Modified by Neon Smart Studio for W-Link
 */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "VL53L0X_Def.h"
#include "VL53L0X_Interrupt_Threshold_Settings.h"
#include "VL53L0X_Tuning.h"
#include "VL53L0X_IO.h"
#include "VL53L0X.h"

#define REF_ARRAY_SPAD_0  0
#define REF_ARRAY_SPAD_5  5
#define REF_ARRAY_SPAD_10 10

static VL53L0X_DevData_t *p_devData = NULL;

uint32_t refArrayQuadrants[4] = {REF_ARRAY_SPAD_10, REF_ARRAY_SPAD_5, REF_ARRAY_SPAD_0, REF_ARRAY_SPAD_5 };

static VL53L0X_OpResult VL53L0X_Get_Sequence_Step_Timeout(uint8_t sensor_index, VL53L0X_SequenceStepId SequenceStepId, uint32_t *pTimeOutMicroSecs);
static VL53L0X_OpResult VL53L0X_Set_Measurement_Timing_Budget_Micro_Seconds(uint8_t sensor_index, uint32_t MeasurementTimingBudgetMicroSeconds);
static VL53L0X_OpResult VL53L0X_Set_Limit_Check_Enable(uint8_t sensor_index, uint16_t LimitCheckId, uint8_t LimitCheckEnable);
static VL53L0X_OpResult VL53L0X_Perform_Phase_Calibration(uint8_t sensor_index, uint8_t *pPhaseCal, const uint8_t get_data_enable, const uint8_t restore_config);
static VL53L0X_OpResult VL53L0X_Set_Sequence_Step_Timeout(uint8_t sensor_index, VL53L0X_SequenceStepId SequenceStepId, uint32_t TimeOutMicroSecs);

VL53L0X_OpResult VL53L0X_Init(uint8_t num_of_sensor, hwGPIO_Pin* p_pwr_pin_list, hwGPIO_Int_Pin* p_int_pin_list, VL53L0X_Interrupt_Handler callback)
{
	p_devData = mem_Malloc(sizeof(VL53L0X_DevData_t)*num_of_sensor);
	if(p_devData==NULL)
	{
		return VL53L0X_MemoryError;
	}

   	return VL53L0X_IO_Init(num_of_sensor, p_pwr_pin_list, p_int_pin_list, callback);
}

VL53L0X_OpResult VL53L0X_DeInit()
{
	if(p_devData==NULL)
	{
		return VL53L0X_NotInit;
	}
	
	mem_Free(p_devData);

    return VL53L0X_IO_DeInit();
}

VL53L0X_OpResult VL53L0X_Power_On(uint8_t sensor_index)
{
   return VL53L0X_IO_Power_On(sensor_index);
}

VL53L0X_OpResult VL53L0X_Power_Off(uint8_t sensor_index)
{
   return VL53L0X_IO_Power_Off(sensor_index);
}

VL53L0X_OpResult VL53L0X_Set_I2C_Address(uint8_t sensor_index, uint8_t new_address)
{
   return VL53L0X_IO_Set_I2C_Address(sensor_index, new_address);
}

static VL53L0X_OpResult VL53L0X_Device_Read_Strobe(uint8_t sensor_index)
{
	VL53L0X_OpResult status;
	uint8_t strobe;
	uint32_t LoopNb;

	status = VL53L0X_IO_Write_Byte(sensor_index, 0x83, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* polling
	 * use timeout to avoid deadlock*/
	LoopNb = 0;
	do {
		status = VL53L0X_IO_Read_Byte(sensor_index, 0x83, &strobe);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		if ((strobe != 0x00))
		{
			break;
		}

		LoopNb++;

		if (LoopNb >= VL53L0X_DEFAULT_MAX_LOOP)
		{
			return VL53L0X_SlaveTimeout;
		}
	} while (1);

	status = VL53L0X_IO_Write_Byte(sensor_index, 0x83, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_Info_From_Device(uint8_t sensor_index, uint8_t option)
{
	VL53L0X_OpResult status;
	uint8_t byte;
	uint32_t TmpDWord;
	uint8_t ModuleId;
	uint8_t Revision;
	uint8_t ReferenceSpadCount = 0;
	uint8_t ReferenceSpadType = 0;
	uint32_t PartUIDUpper = 0;
	uint32_t PartUIDLower = 0;
	uint32_t OffsetFixed1104_mm = 0;
	int16_t OffsetMicroMeters = 0;
	uint32_t DistMeasTgtFixed1104_mm = 400 << 4;
	uint32_t DistMeasFixed1104_400_mm = 0;
	uint32_t SignalRateMeasFixed1104_400_mm = 0;
	char ProductId[19];
	char *pProductId;
	uint8_t ReadDataFromDeviceDone;
	FixPoint1616_t SignalRateMeasFixed400mmFix = 0;
	uint8_t NvmRefGoodSpadMap[VL53L0X_REF_SPAD_BUFFER_SIZE];
	int i;

	ReadDataFromDeviceDone = p_devData[sensor_index].DeviceSpecificParameters.ReadDataFromDeviceDone;

	/* This access is done only once after that a GetDeviceInfo or
	 * datainit is done*/
	if (ReadDataFromDeviceDone != 7)
	{
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x80, 0x01);
		if(status < VL53L0X_OK) { return status; }
		status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x01);
		if(status < VL53L0X_OK) { return status; }
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x00, 0x00);
		if(status < VL53L0X_OK) { return status; }

		status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x06);
		if(status < VL53L0X_OK) { return status; }
		status = VL53L0X_IO_Read_Byte(sensor_index, 0x83, &byte);
		if(status < VL53L0X_OK) { return status; }
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x83, byte|4);
		if(status < VL53L0X_OK) { return status; }
		status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x07);
		if(status < VL53L0X_OK) { return status; }
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x81, 0x01);
		if(status < VL53L0X_OK) { return status; }

		NeonRTOS_Sleep(2);

		status = VL53L0X_IO_Write_Byte(sensor_index, 0x80, 0x01);
		if(status < VL53L0X_OK) { return status; }

		if (((option & 1) == 1) && ((ReadDataFromDeviceDone & 1) == 0))
		{
			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x6b);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Read_DWord(sensor_index, 0x90, &TmpDWord);
			if(status < VL53L0X_OK) { return status; }

			ReferenceSpadCount = (uint8_t)((TmpDWord >> 8) & 0x07f);
			ReferenceSpadType  = (uint8_t)((TmpDWord >> 15) & 0x01);

			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x24);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Read_DWord(sensor_index, 0x90, &TmpDWord);
			if(status < VL53L0X_OK) { return status; }


			NvmRefGoodSpadMap[0] = (uint8_t)((TmpDWord >> 24) & 0xff);
			NvmRefGoodSpadMap[1] = (uint8_t)((TmpDWord >> 16) & 0xff);
			NvmRefGoodSpadMap[2] = (uint8_t)((TmpDWord >> 8) & 0xff);
			NvmRefGoodSpadMap[3] = (uint8_t)(TmpDWord & 0xff);

			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x25);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Read_DWord(sensor_index, 0x90, &TmpDWord);
			if(status < VL53L0X_OK) { return status; }

			NvmRefGoodSpadMap[4] = (uint8_t)((TmpDWord >> 24) & 0xff);
			NvmRefGoodSpadMap[5] = (uint8_t)((TmpDWord >> 16) & 0xff);
		}

		if (((option & 2) == 2) && ((ReadDataFromDeviceDone & 2) == 0))
		{
			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x02);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Read_Byte(sensor_index, 0x90, &ModuleId);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x7B);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Read_Byte(sensor_index, 0x90, &Revision);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x77);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Read_DWord(sensor_index, 0x90, &TmpDWord);
			if(status < VL53L0X_OK) { return status; }

			ProductId[0] = (char)((TmpDWord >> 25) & 0x07f);
			ProductId[1] = (char)((TmpDWord >> 18) & 0x07f);
			ProductId[2] = (char)((TmpDWord >> 11) & 0x07f);
			ProductId[3] = (char)((TmpDWord >> 4) & 0x07f);

			byte = (uint8_t)((TmpDWord & 0x00f) << 3);

			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x78);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Read_DWord(sensor_index, 0x90, &TmpDWord);
			if(status < VL53L0X_OK) { return status; }

			ProductId[4] = (char)(byte + ((TmpDWord >> 29) & 0x07f));
			ProductId[5] = (char)((TmpDWord >> 22) & 0x07f);
			ProductId[6] = (char)((TmpDWord >> 15) & 0x07f);
			ProductId[7] = (char)((TmpDWord >> 8) & 0x07f);
			ProductId[8] = (char)((TmpDWord >> 1) & 0x07f);

			byte = (uint8_t)((TmpDWord & 0x001) << 6);

			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x79);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Read_DWord(sensor_index, 0x90, &TmpDWord);
			if(status < VL53L0X_OK) { return status; }

			ProductId[9] = (char)(byte + ((TmpDWord >> 26) & 0x07f));
			ProductId[10] = (char)((TmpDWord >> 19) & 0x07f);
			ProductId[11] = (char)((TmpDWord >> 12) & 0x07f);
			ProductId[12] = (char)((TmpDWord >> 5) & 0x07f);

			byte = (uint8_t)((TmpDWord & 0x01f) << 2);

			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x7A);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Read_DWord(sensor_index, 0x90, &TmpDWord);
			if(status < VL53L0X_OK) { return status; }

			ProductId[13] = (char)(byte + ((TmpDWord >> 30) & 0x07f));
			ProductId[14] = (char)((TmpDWord >> 23) & 0x07f);
			ProductId[15] = (char)((TmpDWord >> 16) & 0x07f);
			ProductId[16] = (char)((TmpDWord >> 9) & 0x07f);
			ProductId[17] = (char)((TmpDWord >> 2) & 0x07f);
			ProductId[18] = '\0';
		}

		if (((option & 4) == 4) && ((ReadDataFromDeviceDone & 4) == 0))
		{
			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x7B);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Read_DWord(sensor_index, 0x90, &PartUIDUpper);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x7C);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Read_DWord(sensor_index, 0x90, &PartUIDLower);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x73);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Read_DWord(sensor_index, 0x90, &TmpDWord);
			if(status < VL53L0X_OK) { return status; }

			SignalRateMeasFixed1104_400_mm = (TmpDWord & 0x0000000ff) << 8;

			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x74);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Read_DWord(sensor_index, 0x90, &TmpDWord);
			if(status < VL53L0X_OK) { return status; }

			SignalRateMeasFixed1104_400_mm |= ((TmpDWord & 0xff000000) >> 24);

			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x75);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Read_DWord(sensor_index, 0x90, &TmpDWord);
			if(status < VL53L0X_OK) { return status; }

			DistMeasFixed1104_400_mm = (TmpDWord & 0x0000000ff) << 8;

			status = VL53L0X_IO_Write_Byte(sensor_index, 0x94, 0x76);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_Device_Read_Strobe(sensor_index);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Read_DWord(sensor_index, 0x90, &TmpDWord);
			if(status < VL53L0X_OK) { return status; }

			DistMeasFixed1104_400_mm |= ((TmpDWord & 0xff000000) >> 24);
		}

		status = VL53L0X_IO_Write_Byte(sensor_index, 0x81, 0x00);
		if(status < VL53L0X_OK) { return status; }
		status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x06);
		if(status < VL53L0X_OK) { return status; }
		status = VL53L0X_IO_Read_Byte(sensor_index, 0x83, &byte);
		if(status < VL53L0X_OK) { return status; }
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x83, byte&0xfb);
		if(status < VL53L0X_OK) { return status; }
		status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x01);
		if(status < VL53L0X_OK) { return status; }
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x00, 0x01);
		if(status < VL53L0X_OK) { return status; }

		status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x00);
		if(status < VL53L0X_OK) { return status; }
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x80, 0x00);
		if(status < VL53L0X_OK) { return status; }
		
		/* Assign to variable if status is ok */
		if (((option & 1) == 1) && ((ReadDataFromDeviceDone & 1) == 0))
		{
			p_devData[sensor_index].DeviceSpecificParameters.ReferenceSpadCount = ReferenceSpadCount;

			p_devData[sensor_index].DeviceSpecificParameters.ReferenceSpadType = ReferenceSpadType;

			for (i = 0; i < VL53L0X_REF_SPAD_BUFFER_SIZE; i++)
			{
				p_devData[sensor_index].SpadData.RefGoodSpadMap[i] = NvmRefGoodSpadMap[i];
			}
		}

		if (((option & 2) == 2) &&
			((ReadDataFromDeviceDone & 2) == 0)) {
			p_devData[sensor_index].DeviceSpecificParameters.ModuleId = ModuleId;

			p_devData[sensor_index].DeviceSpecificParameters.Revision = Revision;

			pProductId = p_devData[sensor_index].DeviceSpecificParameters.ProductId;
			memcpy(pProductId, ProductId, VL53L0X_MAX_STRING_LENGTH);
		}

		if (((option & 4) == 4) &&
			((ReadDataFromDeviceDone & 4) == 0)) {
			p_devData[sensor_index].DeviceSpecificParameters.PartUIDUpper = PartUIDUpper;

			p_devData[sensor_index].DeviceSpecificParameters.PartUIDLower = PartUIDLower;

			SignalRateMeasFixed400mmFix = VL53L0X_FIXPOINT97TOFIXPOINT1616(SignalRateMeasFixed1104_400_mm);

			p_devData[sensor_index].DeviceSpecificParameters.SignalRateMeasFixed400mm = SignalRateMeasFixed400mmFix;

			OffsetMicroMeters = 0;
			if (DistMeasFixed1104_400_mm != 0)
			{
				OffsetFixed1104_mm = DistMeasFixed1104_400_mm - DistMeasTgtFixed1104_mm;
				OffsetMicroMeters = (OffsetFixed1104_mm * 1000) >> 4;
				OffsetMicroMeters *= -1;
			}

			p_devData[sensor_index].Part2PartOffsetAdjustmentNVMMicroMeter = OffsetMicroMeters;
		}
		byte = (uint8_t)(ReadDataFromDeviceDone|option);
		p_devData[sensor_index].DeviceSpecificParameters.ReadDataFromDeviceDone = byte;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_Offset_Calibration_Data_Micro_Meter(uint8_t sensor_index, int32_t *pOffsetCalibrationDataMicroMeter)
{
	VL53L0X_OpResult status;
	uint16_t RangeOffsetRegister;
	uint16_t cMaxOffset = 2047;
	int16_t cOffsetRange = 4096;

	/* Note that offset has 10.2 format */

	status = VL53L0X_IO_Read_Word(sensor_index, VL53L0X_REG_ALGO_PART_TO_PART_RANGE_OFFSET_MM, &RangeOffsetRegister);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	RangeOffsetRegister = (RangeOffsetRegister & 0x0fff);

	/* Apply 12 bit 2's compliment conversion */
	if (RangeOffsetRegister > cMaxOffset)
	{
		*pOffsetCalibrationDataMicroMeter = (int16_t)(RangeOffsetRegister - cOffsetRange) * 250;
	}
	else
	{
		*pOffsetCalibrationDataMicroMeter = (int16_t)RangeOffsetRegister * 250;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Set_Offset_Calibration_Data_Micro_Meter(uint8_t sensor_index, int32_t OffsetCalibrationDataMicroMeter)
{
	VL53L0X_OpResult status;
	int32_t cMaxOffsetMicroMeter = 511000;
	int32_t cMinOffsetMicroMeter = -512000;
	int16_t cOffsetRange = 4096;
	uint32_t encodedOffsetVal;

	if (OffsetCalibrationDataMicroMeter > cMaxOffsetMicroMeter)
	{
		OffsetCalibrationDataMicroMeter = cMaxOffsetMicroMeter;
	}
	else if (OffsetCalibrationDataMicroMeter < cMinOffsetMicroMeter)
	{
		OffsetCalibrationDataMicroMeter = cMinOffsetMicroMeter;
	}

	/* The offset register is 10.2 format and units are mm
	 * therefore conversion is applied by a division of
	 * 250.
	 */
	if (OffsetCalibrationDataMicroMeter >= 0)
	{
		encodedOffsetVal = OffsetCalibrationDataMicroMeter/250;
	}
	else
	{
		encodedOffsetVal = cOffsetRange + OffsetCalibrationDataMicroMeter/250;
	}

	status = VL53L0X_IO_Write_Word(sensor_index, VL53L0X_REG_ALGO_PART_TO_PART_RANGE_OFFSET_MM, encodedOffsetVal);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Apply_Offset_Adjustment(uint8_t sensor_index)
{
	VL53L0X_OpResult status;
	int32_t CorrectedOffsetMicroMeters;
	int32_t CurrentOffsetMicroMeters;

	/* if we run on this function we can read all the NVM info
	 * used by the API */
	status = VL53L0X_Get_Info_From_Device(sensor_index, 7);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Read back current device offset */
	status = VL53L0X_Get_Offset_Calibration_Data_Micro_Meter(sensor_index, &CurrentOffsetMicroMeters);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Apply Offset Adjustment derived from 400mm measurements */
	/* Store initial device offset */
	p_devData[sensor_index].Part2PartOffsetNVMMicroMeter = CurrentOffsetMicroMeters;

	CorrectedOffsetMicroMeters = CurrentOffsetMicroMeters + (int32_t)p_devData[sensor_index].Part2PartOffsetAdjustmentNVMMicroMeter;

	status = VL53L0X_Set_Offset_Calibration_Data_Micro_Meter(sensor_index, CorrectedOffsetMicroMeters);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* store current, adjusted offset */
	p_devData[sensor_index].CurrentParameters.RangeOffsetMicroMeters = CorrectedOffsetMicroMeters;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_GetDeviceMode(uint8_t sensor_index, VL53L0X_DeviceModes *pDeviceMode)
{
	*pDeviceMode = p_devData[sensor_index].CurrentParameters.DeviceMode;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_Inter_Measurement_Period_MilliSeconds(uint8_t sensor_index, uint32_t *pInterMeasurementPeriodMilliSeconds)
{
	VL53L0X_OpResult status;
	uint16_t osc_calibrate_val;
	uint32_t IMPeriodMilliSeconds;

	status = VL53L0X_IO_Read_Word(sensor_index, VL53L0X_REG_OSC_CALIBRATE_VAL, &osc_calibrate_val);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Read_DWord(sensor_index, VL53L0X_REG_SYSTEM_INTERMEASUREMENT_PERIOD, &IMPeriodMilliSeconds);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	if (osc_calibrate_val != 0)
	{
		*pInterMeasurementPeriodMilliSeconds = IMPeriodMilliSeconds / osc_calibrate_val;
	}

	p_devData[sensor_index].CurrentParameters.InterMeasurementPeriodMilliSeconds = *pInterMeasurementPeriodMilliSeconds;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_XTalk_Compensation_Rate_Mega_Cps(uint8_t sensor_index, FixPoint1616_t *pXTalkCompensationRateMegaCps)
{
	VL53L0X_OpResult status;
	uint16_t Value;
	FixPoint1616_t TempFix1616;

	status = VL53L0X_IO_Read_Word(sensor_index, VL53L0X_REG_CROSSTALK_COMPENSATION_PEAK_RATE_MCPS, (uint16_t *)&Value);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	
	if (Value == 0)
	{
		/* the Xtalk is disabled return value from memory */
		TempFix1616 = p_devData[sensor_index].CurrentParameters.XTalkCompensationRateMegaCps;
		*pXTalkCompensationRateMegaCps = TempFix1616;
		p_devData[sensor_index].CurrentParameters.XTalkCompensationEnable = 0;
	}
	else
	{
		TempFix1616 = VL53L0X_FIXPOINT313TOFIXPOINT1616(Value);
		*pXTalkCompensationRateMegaCps = TempFix1616;
		p_devData[sensor_index].CurrentParameters.XTalkCompensationRateMegaCps = TempFix1616;
		p_devData[sensor_index].CurrentParameters.XTalkCompensationEnable = 1;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_GetLimitCheckValue(uint8_t sensor_index, uint16_t LimitCheckId, FixPoint1616_t *pLimitCheckValue)
{
	VL53L0X_OpResult status;
	uint8_t EnableZeroValue = 0;
	uint16_t Temp16;
	FixPoint1616_t TempFix1616;

	switch (LimitCheckId) {

		case VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE:
			/* internal computation: */
			TempFix1616 = p_devData[sensor_index].CurrentParameters.LimitChecksValue[VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE];
			EnableZeroValue = 0;
			break;

		case VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE:
			status = VL53L0X_IO_Read_Word(sensor_index, VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, &Temp16);
			if(status < VL53L0X_OK)
			{
				return status;
			}

			TempFix1616 = VL53L0X_FIXPOINT97TOFIXPOINT1616(Temp16);
			EnableZeroValue = 1;
			break;

		case VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP:
			/* internal computation: */
			TempFix1616 = p_devData[sensor_index].CurrentParameters.LimitChecksValue[VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP];
			EnableZeroValue = 0;
			break;

		case VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD:
			/* internal computation: */
			TempFix1616 = p_devData[sensor_index].CurrentParameters.LimitChecksValue[VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD];
			EnableZeroValue = 0;
			break;

		case VL53L0X_CHECKENABLE_SIGNAL_RATE_MSRC:
		case VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE:
			status = VL53L0X_IO_Read_Word(sensor_index, VL53L0X_REG_PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT, &Temp16);
			if(status < VL53L0X_OK)
			{
				return status;
			}
			
			TempFix1616 = VL53L0X_FIXPOINT97TOFIXPOINT1616(Temp16);
			EnableZeroValue = 0;
			break;

		default:
			return VL53L0X_InvalidParameter;
	}

	if (EnableZeroValue == 1)
	{
		if (TempFix1616 == 0)
		{
			/* disabled: return value from memory */
			TempFix1616 = p_devData[sensor_index].CurrentParameters.LimitChecksValue[LimitCheckId];
			*pLimitCheckValue = TempFix1616;
			p_devData[sensor_index].CurrentParameters.LimitChecksEnable[LimitCheckId] = 0;
		}
		else
		{
			*pLimitCheckValue = TempFix1616;
			p_devData[sensor_index].CurrentParameters.LimitChecksValue[LimitCheckId] = TempFix1616;
			p_devData[sensor_index].CurrentParameters.LimitChecksEnable[LimitCheckId] = 1;
		}
	}
	else
	{
		*pLimitCheckValue = TempFix1616;
	}

	return status;

}

static VL53L0X_OpResult VL53L0X_GetLimitCheckEnable(uint8_t sensor_index, uint16_t LimitCheckId, uint8_t *pLimitCheckEnable)
{
	uint8_t Temp8;

	if (LimitCheckId >= VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS)
	{
		*pLimitCheckEnable = 0;
		return VL53L0X_InvalidParameter;
	}

	Temp8 = p_devData[sensor_index].CurrentParameters.LimitChecksEnable[LimitCheckId];
	*pLimitCheckEnable = Temp8;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_GetWrapAroundCheckEnable(uint8_t sensor_index, uint8_t *pWrapAroundCheckEnable)
{
	VL53L0X_OpResult status;
	uint8_t data;

	status = VL53L0X_IO_Read_Byte(sensor_index, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, &data);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	p_devData[sensor_index].SequenceConfig = data;

	if (data & (0x01 << 7))
	{
		*pWrapAroundCheckEnable = 0x01;
	}
	else
	{
		*pWrapAroundCheckEnable = 0x00;
	}

	p_devData[sensor_index].CurrentParameters.WrapAroundCheckEnable = *pWrapAroundCheckEnable;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Sequence_Step_Enabled(uint8_t sensor_index, VL53L0X_SequenceStepId SequenceStepId, uint8_t SequenceConfig, uint8_t *pSequenceStepEnabled)
{
	*pSequenceStepEnabled = 0;

	switch (SequenceStepId) {
	case VL53L0X_SEQUENCESTEP_TCC:
		*pSequenceStepEnabled = (SequenceConfig & 0x10) >> 4;
		break;
	case VL53L0X_SEQUENCESTEP_DSS:
		*pSequenceStepEnabled = (SequenceConfig & 0x08) >> 3;
		break;
	case VL53L0X_SEQUENCESTEP_MSRC:
		*pSequenceStepEnabled = (SequenceConfig & 0x04) >> 2;
		break;
	case VL53L0X_SEQUENCESTEP_PRE_RANGE:
		*pSequenceStepEnabled = (SequenceConfig & 0x40) >> 6;
		break;
	case VL53L0X_SEQUENCESTEP_FINAL_RANGE:
		*pSequenceStepEnabled = (SequenceConfig & 0x80) >> 7;
		break;
	default:
		return VL53L0X_InvalidParameter;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_GetSequenceStepEnables(uint8_t sensor_index, VL53L0X_SchedulerSequenceSteps_t *pSchedulerSequenceSteps)
{
	VL53L0X_OpResult status;
	uint8_t SequenceConfig = 0;

	status = VL53L0X_IO_Read_Byte(sensor_index, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, &SequenceConfig);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Sequence_Step_Enabled(sensor_index, VL53L0X_SEQUENCESTEP_TCC, SequenceConfig, &pSchedulerSequenceSteps->TccOn);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Sequence_Step_Enabled(sensor_index, VL53L0X_SEQUENCESTEP_DSS, SequenceConfig, &pSchedulerSequenceSteps->DssOn);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	
	status = VL53L0X_Sequence_Step_Enabled(sensor_index, VL53L0X_SEQUENCESTEP_MSRC, SequenceConfig, &pSchedulerSequenceSteps->MsrcOn);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	
	status = VL53L0X_Sequence_Step_Enabled(sensor_index, VL53L0X_SEQUENCESTEP_PRE_RANGE, SequenceConfig, &pSchedulerSequenceSteps->PreRangeOn);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	
	status = VL53L0X_Sequence_Step_Enabled(sensor_index, VL53L0X_SEQUENCESTEP_FINAL_RANGE, SequenceConfig, &pSchedulerSequenceSteps->FinalRangeOn);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	return VL53L0X_OK;
}

static uint8_t VL53L0X_Decode_Vcsel_Period(uint8_t vcsel_period_reg)
{
	/*!
	 * Converts the encoded VCSEL period register value into the real
	 * period in PLL clocks
	 */

	uint8_t vcsel_period_pclks = 0;

	vcsel_period_pclks = (vcsel_period_reg + 1) << 1;

	return vcsel_period_pclks;
}

static uint8_t VL53L0X_Encode_Vcsel_Period(uint8_t vcsel_period_pclks)
{
	/*!
	 * Converts the encoded VCSEL period register value into the real period
	 * in PLL clocks
	 */

	uint8_t vcsel_period_reg = 0;

	vcsel_period_reg = (vcsel_period_pclks >> 1) - 1;

	return vcsel_period_reg;
}

static VL53L0X_OpResult VL53L0X_Set_Vcsel_Pulse_Period(uint8_t sensor_index, VL53L0X_VcselPeriod VcselPeriodType, uint8_t VCSELPulsePeriodPCLK)
{
	VL53L0X_OpResult status;
	uint8_t vcsel_period_reg;
	uint8_t MinPreVcselPeriodPCLK = 12;
	uint8_t MaxPreVcselPeriodPCLK = 18;
	uint8_t MinFinalVcselPeriodPCLK = 8;
	uint8_t MaxFinalVcselPeriodPCLK = 14;
	uint32_t MeasurementTimingBudgetMicroSeconds;
	uint32_t FinalRangeTimeoutMicroSeconds;
	uint32_t PreRangeTimeoutMicroSeconds;
	uint32_t MsrcTimeoutMicroSeconds;
	uint8_t PhaseCalInt = 0;

	/* Check if valid clock period requested */

	if ((VCSELPulsePeriodPCLK % 2) != 0)
	{
		/* Value must be an even number */
		return VL53L0X_InvalidParameter;
	}
	else if (VcselPeriodType == VL53L0X_VCSEL_PERIOD_PRE_RANGE && (VCSELPulsePeriodPCLK < MinPreVcselPeriodPCLK || VCSELPulsePeriodPCLK > MaxPreVcselPeriodPCLK))
	{
		return VL53L0X_InvalidParameter;
	}
	else if (VcselPeriodType == VL53L0X_VCSEL_PERIOD_FINAL_RANGE && (VCSELPulsePeriodPCLK < MinFinalVcselPeriodPCLK || VCSELPulsePeriodPCLK > MaxFinalVcselPeriodPCLK))
	{
		return VL53L0X_InvalidParameter;
	}

	/* Apply specific settings for the requested clock period */

	if (VcselPeriodType == VL53L0X_VCSEL_PERIOD_PRE_RANGE)
	{
		/* Set phase check limits */
		if (VCSELPulsePeriodPCLK == 12)
		{
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x18);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
			if(status < VL53L0X_OK) { return status; }
		}
		else if (VCSELPulsePeriodPCLK == 14)
		{
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x30);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
			if(status < VL53L0X_OK) { return status; }
		}
		else if (VCSELPulsePeriodPCLK == 16)
		{
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x40);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
			if(status < VL53L0X_OK) { return status; }
		}
		else if (VCSELPulsePeriodPCLK == 18)
		{
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x50);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
			if(status < VL53L0X_OK) { return status; }
		}
	}
	else if (VcselPeriodType == VL53L0X_VCSEL_PERIOD_FINAL_RANGE)
	{
		if (VCSELPulsePeriodPCLK == 8)
		{
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x10);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_GLOBAL_CONFIG_VCSEL_WIDTH, 0x02);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_ALGO_PHASECAL_CONFIG_TIMEOUT, 0x0C);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x01);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_ALGO_PHASECAL_LIM, 0x30);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x00);
			if(status < VL53L0X_OK) { return status; }
		}
		else if (VCSELPulsePeriodPCLK == 10)
		{
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x28);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_ALGO_PHASECAL_CONFIG_TIMEOUT, 0x09);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x01);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_ALGO_PHASECAL_LIM, 0x20);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x00);
			if(status < VL53L0X_OK) { return status; }
		}
		else if (VCSELPulsePeriodPCLK == 12)
		{
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x38);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_ALGO_PHASECAL_CONFIG_TIMEOUT, 0x08);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x01);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_ALGO_PHASECAL_LIM, 0x20);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x00);
			if(status < VL53L0X_OK) { return status; }
		}
		else if (VCSELPulsePeriodPCLK == 14)
		{
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x048);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_ALGO_PHASECAL_CONFIG_TIMEOUT, 0x07);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x01);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_ALGO_PHASECAL_LIM, 0x20);
			if(status < VL53L0X_OK) { return status; }
			status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x00);
			if(status < VL53L0X_OK) { return status; }
		}
	}

	/* Re-calculate and apply timeouts, in macro periods */

	vcsel_period_reg = VL53L0X_Encode_Vcsel_Period((uint8_t)VCSELPulsePeriodPCLK);

	/* When the VCSEL period for the pre or final range is changed	* the corresponding timeout must be read from the device using
	* the current VCSEL period, then the new VCSEL period can be
	* applied. The timeout then must be written back to the device
	* using the new VCSEL period.
	*
	* For the MSRC timeout, the same applies - this timeout being
	* dependant on the pre-range vcsel period.
	*/
	switch (VcselPeriodType) {
		case VL53L0X_VCSEL_PERIOD_PRE_RANGE:
			status = VL53L0X_Get_Sequence_Step_Timeout(sensor_index, VL53L0X_SEQUENCESTEP_PRE_RANGE, &PreRangeTimeoutMicroSeconds);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_Get_Sequence_Step_Timeout(sensor_index, VL53L0X_SEQUENCESTEP_MSRC, &MsrcTimeoutMicroSeconds);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_Set_Sequence_Step_Timeout(sensor_index, VL53L0X_SEQUENCESTEP_PRE_RANGE, PreRangeTimeoutMicroSeconds);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_Set_Sequence_Step_Timeout(sensor_index, VL53L0X_SEQUENCESTEP_MSRC, MsrcTimeoutMicroSeconds);
			if(status < VL53L0X_OK) { return status; }

			p_devData[sensor_index].DeviceSpecificParameters.PreRangeVcselPulsePeriod = VCSELPulsePeriodPCLK;
			break;
		case VL53L0X_VCSEL_PERIOD_FINAL_RANGE:
			status = VL53L0X_Get_Sequence_Step_Timeout(sensor_index, VL53L0X_SEQUENCESTEP_FINAL_RANGE, &FinalRangeTimeoutMicroSeconds);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg);
			if(status < VL53L0X_OK) { return status; }

			status = VL53L0X_Set_Sequence_Step_Timeout(sensor_index, VL53L0X_SEQUENCESTEP_FINAL_RANGE, FinalRangeTimeoutMicroSeconds);
			if(status < VL53L0X_OK) { return status; }

			p_devData[sensor_index].DeviceSpecificParameters.FinalRangeVcselPulsePeriod = VCSELPulsePeriodPCLK;
			break;
		default:
			return VL53L0X_InvalidParameter;
	}

	/* Finally, the timing budget must be re-applied */
	MeasurementTimingBudgetMicroSeconds = p_devData[sensor_index].CurrentParameters.MeasurementTimingBudgetMicroSeconds;

	status = VL53L0X_Set_Measurement_Timing_Budget_Micro_Seconds(sensor_index, MeasurementTimingBudgetMicroSeconds);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Perform the phase calibration. This is needed after changing on
	 * vcsel period.
	 * get_data_enable = 0, restore_config = 1 */
	status = VL53L0X_Perform_Phase_Calibration(sensor_index, &PhaseCalInt, 0, 1);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	return status;
}

static VL53L0X_OpResult VL53L0X_Get_Vcsel_Pulse_Period(uint8_t sensor_index, VL53L0X_VcselPeriod VcselPeriodType, uint8_t *pVCSELPulsePeriodPCLK)
{
	VL53L0X_OpResult status;
	uint8_t vcsel_period_reg;

	switch (VcselPeriodType)
	{
		case VL53L0X_VCSEL_PERIOD_PRE_RANGE:
			status = VL53L0X_IO_Read_Byte(sensor_index, VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD, &vcsel_period_reg);
			if(status < VL53L0X_OK) { return status; }
			break;
		case VL53L0X_VCSEL_PERIOD_FINAL_RANGE:
			status = VL53L0X_IO_Read_Byte(sensor_index, VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD, &vcsel_period_reg);
			if(status < VL53L0X_OK) { return status; }
			break;
		default:
			return VL53L0X_InvalidParameter;
	}

	*pVCSELPulsePeriodPCLK = VL53L0X_Decode_Vcsel_Period(vcsel_period_reg);

	return VL53L0X_OK;
}

uint32_t VL53L0X_Decode_Timeout(uint16_t encoded_timeout)
{
	/*!
	 * Decode 16-bit timeout register value - format (LSByte * 2^MSByte) + 1
	 */

	uint32_t timeout_macro_clks = 0;

	timeout_macro_clks = ((uint32_t) (encoded_timeout & 0x00FF) << (uint32_t) ((encoded_timeout & 0xFF00) >> 8)) + 1;

	return timeout_macro_clks;
}

uint32_t VL53L0X_Calc_Macro_Period_pS(uint8_t vcsel_period_pclks)
{
	uint64_t PLL_period_ps;
	uint32_t macro_period_vclks;
	uint32_t macro_period_ps;

	/* The above calculation will produce rounding errors   therefore set fixed value
	*/
	PLL_period_ps = 1655;

	macro_period_vclks = 2304;
	macro_period_ps = (uint32_t)(macro_period_vclks * vcsel_period_pclks * PLL_period_ps);

	return macro_period_ps;
}

/* To convert register value into us */
uint32_t VL53L0X_Calc_Timeout_uS(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks)
{
	uint32_t macro_period_ps;
	uint32_t macro_period_ns;
	uint32_t actual_timeout_period_us = 0;

	macro_period_ps = VL53L0X_Calc_Macro_Period_pS(vcsel_period_pclks);
	macro_period_ns = (macro_period_ps + 500) / 1000;

	actual_timeout_period_us = ((timeout_period_mclks * macro_period_ns) + 500) / 1000;

	return actual_timeout_period_us;
}

static VL53L0X_OpResult VL53L0X_Get_Sequence_Step_Timeout(uint8_t sensor_index, VL53L0X_SequenceStepId SequenceStepId, uint32_t *pTimeOutMicroSecs)
{
	VL53L0X_OpResult status;
	uint8_t CurrentVCSELPulsePeriodPClk;
	uint8_t EncodedTimeOutByte = 0;
	uint32_t TimeoutMicroSeconds = 0;
	uint16_t PreRangeEncodedTimeOut = 0;
	uint16_t MsrcTimeOutMClks;
	uint16_t PreRangeTimeOutMClks;
	uint16_t FinalRangeTimeOutMClks = 0;
	uint16_t FinalRangeEncodedTimeOut;
	VL53L0X_SchedulerSequenceSteps_t SchedulerSequenceSteps;

	if ((SequenceStepId == VL53L0X_SEQUENCESTEP_TCC) || (SequenceStepId == VL53L0X_SEQUENCESTEP_DSS) || (SequenceStepId == VL53L0X_SEQUENCESTEP_MSRC))
	{
		status = VL53L0X_Get_Vcsel_Pulse_Period(sensor_index, VL53L0X_VCSEL_PERIOD_PRE_RANGE, &CurrentVCSELPulsePeriodPClk);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		status = VL53L0X_IO_Read_Byte(sensor_index, VL53L0X_REG_MSRC_CONFIG_TIMEOUT_MACROP, &EncodedTimeOutByte);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		MsrcTimeOutMClks = VL53L0X_Decode_Timeout(EncodedTimeOutByte);

		TimeoutMicroSeconds = VL53L0X_Calc_Timeout_uS(MsrcTimeOutMClks, CurrentVCSELPulsePeriodPClk);
	}
	else if (SequenceStepId == VL53L0X_SEQUENCESTEP_PRE_RANGE)
	{
		/* Retrieve PRE-RANGE VCSEL Period */
		status = VL53L0X_Get_Vcsel_Pulse_Period(sensor_index, VL53L0X_VCSEL_PERIOD_PRE_RANGE, &CurrentVCSELPulsePeriodPClk);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		/* Retrieve PRE-RANGE VCSEL Period */
		status = VL53L0X_Get_Vcsel_Pulse_Period(sensor_index, VL53L0X_VCSEL_PERIOD_PRE_RANGE, &CurrentVCSELPulsePeriodPClk);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		status = VL53L0X_IO_Read_Word(sensor_index, VL53L0X_REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, &PreRangeEncodedTimeOut);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		PreRangeTimeOutMClks = VL53L0X_Decode_Timeout(PreRangeEncodedTimeOut);

		TimeoutMicroSeconds = VL53L0X_Calc_Timeout_uS(PreRangeTimeOutMClks, CurrentVCSELPulsePeriodPClk);
	}
	else if (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)
	{
		status = VL53L0X_GetSequenceStepEnables(sensor_index, &SchedulerSequenceSteps);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		PreRangeTimeOutMClks = 0;

		if (SchedulerSequenceSteps.PreRangeOn)
		{
			/* Retrieve PRE-RANGE VCSEL Period */
			status = VL53L0X_Get_Vcsel_Pulse_Period(sensor_index, VL53L0X_VCSEL_PERIOD_PRE_RANGE, &CurrentVCSELPulsePeriodPClk);
			if(status < VL53L0X_OK)
			{
				return status;
			}

			/* Retrieve PRE-RANGE Timeout in Macro periods
			 * (MCLKS) */
			status = VL53L0X_IO_Read_Word(sensor_index, VL53L0X_REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, &PreRangeEncodedTimeOut);
			if(status < VL53L0X_OK)
			{
				return status;
			}

			PreRangeTimeOutMClks = VL53L0X_Decode_Timeout( PreRangeEncodedTimeOut);
		}

		/* Retrieve FINAL-RANGE VCSEL Period */
		status = VL53L0X_Get_Vcsel_Pulse_Period(sensor_index, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, &CurrentVCSELPulsePeriodPClk);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		/* Retrieve FINAL-RANGE Timeout in Macro periods (MCLKS) */
		status = VL53L0X_IO_Read_Word(sensor_index, VL53L0X_REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, &FinalRangeEncodedTimeOut);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		FinalRangeTimeOutMClks = VL53L0X_Decode_Timeout(FinalRangeEncodedTimeOut);

		FinalRangeTimeOutMClks -= PreRangeTimeOutMClks;
		TimeoutMicroSeconds = VL53L0X_Calc_Timeout_uS(FinalRangeTimeOutMClks, CurrentVCSELPulsePeriodPClk);
	}

	*pTimeOutMicroSecs = TimeoutMicroSeconds;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_Measurement_Timing_Budget_Micro_Seconds(uint8_t sensor_index, uint32_t *pMeasurementTimingBudgetMicroSeconds)
{
	VL53L0X_OpResult status;
	VL53L0X_SchedulerSequenceSteps_t SchedulerSequenceSteps;
	uint32_t FinalRangeTimeoutMicroSeconds;
	uint32_t MsrcDccTccTimeoutMicroSeconds	= 2000;
	uint32_t StartOverheadMicroSeconds		= 1910;
	uint32_t EndOverheadMicroSeconds		= 960;
	uint32_t MsrcOverheadMicroSeconds		= 660;
	uint32_t TccOverheadMicroSeconds		= 590;
	uint32_t DssOverheadMicroSeconds		= 690;
	uint32_t PreRangeOverheadMicroSeconds	= 660;
	uint32_t FinalRangeOverheadMicroSeconds = 550;
	uint32_t PreRangeTimeoutMicroSeconds	= 0;


	/* Start and end overhead times always present */
	*pMeasurementTimingBudgetMicroSeconds = StartOverheadMicroSeconds + EndOverheadMicroSeconds;

	status = VL53L0X_GetSequenceStepEnables(sensor_index, &SchedulerSequenceSteps);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	if (SchedulerSequenceSteps.TccOn  || SchedulerSequenceSteps.MsrcOn || SchedulerSequenceSteps.DssOn)
	{
		status = VL53L0X_Get_Sequence_Step_Timeout(sensor_index, VL53L0X_SEQUENCESTEP_MSRC, &MsrcDccTccTimeoutMicroSeconds);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		if (SchedulerSequenceSteps.TccOn)
		{
			*pMeasurementTimingBudgetMicroSeconds += MsrcDccTccTimeoutMicroSeconds + TccOverheadMicroSeconds;
		}

		if (SchedulerSequenceSteps.DssOn)
		{
			*pMeasurementTimingBudgetMicroSeconds += 2 * (MsrcDccTccTimeoutMicroSeconds + DssOverheadMicroSeconds);
		}
		else if (SchedulerSequenceSteps.MsrcOn)
		{
			*pMeasurementTimingBudgetMicroSeconds += MsrcDccTccTimeoutMicroSeconds + MsrcOverheadMicroSeconds;
		}
	}

	if (SchedulerSequenceSteps.PreRangeOn) {
		status = VL53L0X_Get_Sequence_Step_Timeout(sensor_index, VL53L0X_SEQUENCESTEP_PRE_RANGE, &PreRangeTimeoutMicroSeconds);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		*pMeasurementTimingBudgetMicroSeconds += PreRangeTimeoutMicroSeconds + PreRangeOverheadMicroSeconds;
	}

	if (SchedulerSequenceSteps.FinalRangeOn) {
		status = VL53L0X_Get_Sequence_Step_Timeout(sensor_index, VL53L0X_SEQUENCESTEP_FINAL_RANGE, &FinalRangeTimeoutMicroSeconds);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		*pMeasurementTimingBudgetMicroSeconds += (FinalRangeTimeoutMicroSeconds + FinalRangeOverheadMicroSeconds);
	}

	p_devData[sensor_index].CurrentParameters.MeasurementTimingBudgetMicroSeconds = *pMeasurementTimingBudgetMicroSeconds;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_GetDeviceParameters(uint8_t sensor_index, VL53L0X_DeviceParameters_t *pDeviceParameters)
{
	VL53L0X_OpResult status;
	int i;

	status = VL53L0X_GetDeviceMode(sensor_index, &(pDeviceParameters->DeviceMode));
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Get_Inter_Measurement_Period_MilliSeconds(sensor_index, &(pDeviceParameters->InterMeasurementPeriodMilliSeconds));
	if(status < VL53L0X_OK)
	{
		return status;
	}

	pDeviceParameters->XTalkCompensationEnable = 0;

	status = VL53L0X_Get_XTalk_Compensation_Rate_Mega_Cps(sensor_index, &(pDeviceParameters->XTalkCompensationRateMegaCps));
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Get_Offset_Calibration_Data_Micro_Meter(sensor_index, &(pDeviceParameters->RangeOffsetMicroMeters));
	if(status < VL53L0X_OK)
	{
		return status;
	}

	for (i = 0; i < VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS; i++)
	{
		/* get first the values, then the enables.
			* VL53L0X_GetLimitCheckValue will modify the enable
			* flags
			*/
		status = VL53L0X_GetLimitCheckValue(sensor_index, i, &(pDeviceParameters->LimitChecksValue[i]));
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_GetLimitCheckEnable(sensor_index, i, &(pDeviceParameters->LimitChecksEnable[i]));
		if(status < VL53L0X_OK)
		{
			return status;
		}
	}

	status = VL53L0X_GetWrapAroundCheckEnable(sensor_index, &(pDeviceParameters->WrapAroundCheckEnable));
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Need to be done at the end as it uses VCSELPulsePeriod */
	status = VL53L0X_Get_Measurement_Timing_Budget_Micro_Seconds(sensor_index, &(pDeviceParameters->MeasurementTimingBudgetMicroSeconds));
	if(status < VL53L0X_OK)
	{
		return status;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_SetLimitCheckValue(uint8_t sensor_index, uint16_t LimitCheckId, FixPoint1616_t LimitCheckValue)
{
	VL53L0X_OpResult status;
	uint8_t Temp8;

	Temp8 = p_devData[sensor_index].CurrentParameters.LimitChecksEnable[LimitCheckId];

	if (Temp8 == 0)
	{ /* disabled write only internal value */
		 p_devData[sensor_index].CurrentParameters.LimitChecksValue[LimitCheckId] = LimitCheckValue;
	}
	else
	{
		switch (LimitCheckId)
		{
			case VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE:
				/* internal computation: */
				p_devData[sensor_index].CurrentParameters.LimitChecksValue[VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE] = LimitCheckValue;

				break;

			case VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE:
				status = VL53L0X_IO_Write_Word(sensor_index, VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, VL53L0X_FIXPOINT1616TOFIXPOINT97(LimitCheckValue));
				if(status < VL53L0X_OK)
				{
					return status;
				}

				break;

			case VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP:
				/* internal computation: */
				p_devData[sensor_index].CurrentParameters.LimitChecksValue[VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP] = LimitCheckValue;

				break;

			case VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD:
				/* internal computation: */
				 p_devData[sensor_index].CurrentParameters.LimitChecksValue[VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD] = LimitCheckValue;

				break;

			case VL53L0X_CHECKENABLE_SIGNAL_RATE_MSRC:
			case VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE:
				status = VL53L0X_IO_Write_Word(sensor_index, VL53L0X_REG_PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT, VL53L0X_FIXPOINT1616TOFIXPOINT97(LimitCheckValue));
				if(status < VL53L0X_OK)
				{
					return status;
				}

				break;

			default:
				return VL53L0X_InvalidParameter;
			}

			p_devData[sensor_index].CurrentParameters.LimitChecksValue[LimitCheckId] = LimitCheckValue;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_DataInit(uint8_t sensor_index)
{
	VL53L0X_OpResult status;
	VL53L0X_DeviceParameters_t CurrentParameters;
	int i;
	uint8_t StopVariable;

	/* by default the I2C is running at 1V8 if you want to change it you
	 * need to include this define at compilation level. */
#ifdef USE_I2C_2V8
	status = VL53L0X_IO_Update_Byte(sensor_index, VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, 0xFE, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}
#endif

	/* Set I2C standard mode */
	status = VL53L0X_IO_Write_Byte(sensor_index, 0x88, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	p_devData[sensor_index].DeviceSpecificParameters.ReadDataFromDeviceDone = 0;

#ifdef USE_IQC_STATION
	status = VL53L0X_Apply_Offset_Adjustment();
	if(status < VL53L0X_OK)
	{
		return status;
	}
#endif

	/* Default value is 1000 for Linearity Corrective Gain */
	p_devData[sensor_index].LinearityCorrectiveGain = 1000;

	/* Dmax default Parameter */
	p_devData[sensor_index].DmaxCalRangeMilliMeter = 400;
	p_devData[sensor_index].DmaxCalSignalRateRtnMegaCps = (FixPoint1616_t)((0x00016B85)); /* 1.42 No Cover Glass*/

	/* Set Default static parameters
	 *set first temporary values 9.44MHz * 65536 = 618660 */
	p_devData[sensor_index].DeviceSpecificParameters.OscFrequencyMHz = 618660;

	/* Set Default XTalkCompensationRateMegaCps to 0  */
	p_devData[sensor_index].CurrentParameters.XTalkCompensationRateMegaCps = 0;

	/* Get default parameters */
	status = VL53L0X_GetDeviceParameters(sensor_index, &CurrentParameters);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	
	/* initialize PAL values */
	CurrentParameters.DeviceMode = VL53L0X_DEVICEMODE_SINGLE_RANGING;
	CurrentParameters.HistogramMode = VL53L0X_HISTOGRAMMODE_DISABLED;
	p_devData[sensor_index].CurrentParameters = CurrentParameters;

	/* Sigma estimator variable */
	p_devData[sensor_index].SigmaEstRefArray = 100;
	p_devData[sensor_index].SigmaEstEffPulseWidth = 900;
	p_devData[sensor_index].SigmaEstEffAmbWidth = 500;
	p_devData[sensor_index].targetRefRate = 0x0A00; /* 20 MCPS in 9:7 format */

	/* Use internal default settings */
	p_devData[sensor_index].UseInternalTuningSettings = 1;

	status = VL53L0X_IO_Write_Byte(sensor_index, 0x80, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0x00, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Read_Byte(sensor_index, 0x91, &StopVariable);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	p_devData[sensor_index].StopVariable = StopVariable;

	status = VL53L0X_IO_Write_Byte(sensor_index, 0x00, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0x80, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Enable all check */
	for (i = 0; i < VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS; i++) {
		status = VL53L0X_Set_Limit_Check_Enable(sensor_index, i, 1);
		if(status < VL53L0X_OK)
		{
			return status;
		}
	}

	/* Disable the following checks */
	status = VL53L0X_Set_Limit_Check_Enable(sensor_index, VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP, 0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Set_Limit_Check_Enable(sensor_index, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Set_Limit_Check_Enable(sensor_index, VL53L0X_CHECKENABLE_SIGNAL_RATE_MSRC, 0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Set_Limit_Check_Enable(sensor_index, VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE, 0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Limit default values */
	status = VL53L0X_SetLimitCheckValue(sensor_index, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(18 * 65536));
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* 0.25 * 65536 */
	status = VL53L0X_SetLimitCheckValue(sensor_index, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(25 * 65536 / 100));
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_SetLimitCheckValue(sensor_index, VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP, (FixPoint1616_t)(35 * 65536));
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_SetLimitCheckValue(sensor_index, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, (FixPoint1616_t)(0 * 65536));
	if(status < VL53L0X_OK)
	{
		return status;
	}

	p_devData[sensor_index].SequenceConfig = 0xFF;
	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0xFF);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Set PAL state to tell that we are waiting for call to
		* VL53L0X_StaticInit */
	p_devData[sensor_index].PalState = VL53L0X_STATE_WAIT_STATICINIT;

	p_devData[sensor_index].DeviceSpecificParameters.RefSpadsInitialised = 0;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Check_Part_Used(uint8_t sensor_index, uint8_t *Revision, VL53L0X_DeviceInfo_t *pVL53L0X_DeviceInfo)
{
	VL53L0X_OpResult status;
	uint8_t ModuleIdInt;
	char *pProductId;

	status = VL53L0X_Get_Info_From_Device(sensor_index, 2);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	ModuleIdInt = p_devData[sensor_index].DeviceSpecificParameters.ModuleId;

	if (ModuleIdInt == 0)
	{
		*Revision = 0;
		memcpy(pVL53L0X_DeviceInfo->ProductId, "", strlen(""));
	}
	else
	{
		*Revision = p_devData[sensor_index].DeviceSpecificParameters.Revision;
		pProductId = p_devData[sensor_index].DeviceSpecificParameters.ProductId;
		memcpy(pVL53L0X_DeviceInfo->ProductId, pProductId, strlen(pProductId));
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_Device_Info(uint8_t sensor_index, VL53L0X_DeviceInfo_t *pVL53L0X_DeviceInfo)
{
	VL53L0X_OpResult status;
	uint8_t revision_id;
	uint8_t Revision;

	status = VL53L0X_Check_Part_Used(sensor_index, &Revision, pVL53L0X_DeviceInfo);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	if (Revision == 0)
	{
		memcpy(pVL53L0X_DeviceInfo->Name, VL53L0X_STRING_DEVICE_INFO_NAME_TS0, strlen(VL53L0X_STRING_DEVICE_INFO_NAME_TS0));
	}
	else if ((Revision <= 34) && (Revision != 32))
	{
		memcpy(pVL53L0X_DeviceInfo->Name, VL53L0X_STRING_DEVICE_INFO_NAME_TS1, strlen(VL53L0X_STRING_DEVICE_INFO_NAME_TS1));
	}
	else if (Revision < 39)
	{
		memcpy(pVL53L0X_DeviceInfo->Name, VL53L0X_STRING_DEVICE_INFO_NAME_TS2, strlen(VL53L0X_STRING_DEVICE_INFO_NAME_TS2));
	}
	else
	{
		memcpy(pVL53L0X_DeviceInfo->Name, VL53L0X_STRING_DEVICE_INFO_NAME_ES1, strlen(VL53L0X_STRING_DEVICE_INFO_NAME_ES1));
	}

	memcpy(pVL53L0X_DeviceInfo->Type, VL53L0X_STRING_DEVICE_INFO_TYPE, strlen(VL53L0X_STRING_DEVICE_INFO_TYPE));

	status = VL53L0X_IO_Read_Byte(sensor_index, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &pVL53L0X_DeviceInfo->ProductType);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Read_Byte(sensor_index, VL53L0X_REG_IDENTIFICATION_REVISION_ID, &revision_id);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	pVL53L0X_DeviceInfo->ProductRevisionMajor = 1;
	pVL53L0X_DeviceInfo->ProductRevisionMinor = (revision_id & 0xF0) >> 4;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_Interrupt_Mask_Status(uint8_t sensor_index, uint32_t *pInterruptMaskstatus)
{
	VL53L0X_OpResult status;
	uint8_t Byte;

	status = VL53L0X_IO_Read_Byte(sensor_index, VL53L0X_REG_RESULT_INTERRUPT_STATUS, &Byte);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	*pInterruptMaskstatus = Byte & 0x07;

	if (Byte & 0x18)
	{
		return VL53L0X_Range_Error;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_Measurement_Data_Ready(uint8_t sensor_index, uint8_t *pMeasurementDataReady)
{
	VL53L0X_OpResult status;
	uint8_t SysRangestatusRegister;
	uint8_t InterruptConfig;
	uint32_t InterruptMask;

	InterruptConfig = p_devData[sensor_index].DeviceSpecificParameters.Pin0GpioFunctionality;

	if (InterruptConfig == VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY)
	{
		status = VL53L0X_Get_Interrupt_Mask_Status(sensor_index, &InterruptMask);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		if (InterruptMask == VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY)
		{
			*pMeasurementDataReady = 1;
		}
		else
		{
			*pMeasurementDataReady = 0;
		}
	}
	else
	{
		status = VL53L0X_IO_Read_Byte(sensor_index, VL53L0X_REG_RESULT_RANGE_STATUS, &SysRangestatusRegister);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		if (SysRangestatusRegister & 0x01)
		{
			*pMeasurementDataReady = 1;
		}
		else
		{
			*pMeasurementDataReady = 0;
		}
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Measurement_Poll_For_Completion(uint8_t sensor_index)
{
	VL53L0X_OpResult status;
	uint8_t NewDataReady = 0;
	uint32_t LoopNb;

	LoopNb = 0;
	do {
		status = VL53L0X_Get_Measurement_Data_Ready(sensor_index, &NewDataReady);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		if (NewDataReady == 1)
			break; /* done note that status == 0 */

		LoopNb++;
		if (LoopNb >= VL53L0X_DEFAULT_MAX_LOOP) {
			return VL53L0X_SlaveTimeout;
		}

		NeonRTOS_Sleep(2);
	} while (1);

	return VL53L0X_OK;
}

/* Group PAL Interrupt Functions */
static VL53L0X_OpResult VL53L0X_Clear_Interrupt_Mask(uint8_t sensor_index, uint32_t InterruptMask)
{
	VL53L0X_OpResult status;
	uint8_t LoopCount;
	uint8_t Byte;

	/* clear bit 0 range interrupt, bit 1 error interrupt */
	LoopCount = 0;
	do {
		status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x00);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Read_Byte(sensor_index, VL53L0X_REG_RESULT_INTERRUPT_STATUS, &Byte);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		LoopCount++;

		if((Byte & 0x07) == 0x00)
		{
			break;
		}

		if (LoopCount >= 3)
		{
			return VL53L0X_Interrupt_Not_Cleard;
		}
	} while (1);

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Perform_Single_Ref_Calibration(uint8_t sensor_index, uint8_t vhv_init_byte)
{
	VL53L0X_OpResult status;

	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_START_STOP | vhv_init_byte);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Measurement_Poll_For_Completion(sensor_index);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Clear_Interrupt_Mask(sensor_index, 0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSRANGE_START, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Ref_Calibration_IO(uint8_t sensor_index, uint8_t read_not_write, uint8_t VhvSettings, uint8_t PhaseCal, uint8_t *pVhvSettings, uint8_t *pPhaseCal, const uint8_t vhv_enable, const uint8_t phase_enable)
{
	VL53L0X_OpResult status;
	uint8_t PhaseCalint = 0;

	/* Read VHV from device */
	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0x00, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	if (read_not_write)
	{
		if (vhv_enable)
		{
			status = VL53L0X_IO_Read_Byte(sensor_index, 0xCB, pVhvSettings);
			if(status < VL53L0X_OK)
			{
				return status;
			}
		}
		if (phase_enable)
		{
			status = VL53L0X_IO_Read_Byte(sensor_index, 0xEE, &PhaseCalint);
			if(status < VL53L0X_OK)
			{
				return status;
			}
		}
	}
	else
	{
		if (vhv_enable)
		{
			status = VL53L0X_IO_Write_Byte(sensor_index, 0xCB, VhvSettings);
			if(status < VL53L0X_OK)
			{
				return status;
			}
		}
		if (phase_enable)
		{
			status = VL53L0X_IO_Update_Byte(sensor_index, 0xEE, 0x80, PhaseCal);
			if(status < VL53L0X_OK)
			{
				return status;
			}
		}
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0x00, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	*pPhaseCal = (uint8_t)(PhaseCalint&0xEF);

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Perform_VHV_Calibration(uint8_t sensor_index, uint8_t *pVhvSettings, const uint8_t get_data_enable, const uint8_t restore_config)
{
	VL53L0X_OpResult status;
	uint8_t SequenceConfig = 0;
	uint8_t VhvSettings = 0;
	uint8_t PhaseCal = 0;
	uint8_t PhaseCalInt = 0;

	/* store the value of the sequence config * this will be reset before the end of the function
	 */

	if (restore_config)
	{
		SequenceConfig = p_devData[sensor_index].SequenceConfig;
	}

	/* Run VHV */
	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Perform_Single_Ref_Calibration(sensor_index, 0x40);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Read VHV from device */
	if (get_data_enable == 1) {
		status = VL53L0X_Ref_Calibration_IO(sensor_index, 1, VhvSettings, PhaseCal, pVhvSettings, &PhaseCalInt, 1, 0);
		if(status < VL53L0X_OK)
		{
			return status;
		}
	}
	else
	{
		*pVhvSettings = 0;
	}

	if (restore_config)
	{
		/* restore the previous Sequence Config */
		status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, SequenceConfig);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		p_devData[sensor_index].SequenceConfig = SequenceConfig;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Perform_Phase_Calibration(uint8_t sensor_index, uint8_t *pPhaseCal, const uint8_t get_data_enable, const uint8_t restore_config)
{
	VL53L0X_OpResult status;
	uint8_t SequenceConfig = 0;
	uint8_t VhvSettings = 0;
	uint8_t PhaseCal = 0;
	uint8_t VhvSettingsint;

	/* store the value of the sequence config * this will be reset before the end of the function
	 */

	if (restore_config)
	{
		SequenceConfig = p_devData[sensor_index].SequenceConfig;
	}

	/* Run PhaseCal */
	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0x02);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Perform_Single_Ref_Calibration(sensor_index, 0x0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Read PhaseCal from device */
	if (get_data_enable == 1)
	{
		status = VL53L0X_Ref_Calibration_IO(sensor_index, 1, VhvSettings, PhaseCal, &VhvSettingsint, pPhaseCal, 0, 1);
		if(status < VL53L0X_OK)
		{
			return status;
		}
	}
	else
	{
		*pPhaseCal = 0;
	}

	if (restore_config)
	{
		/* restore the previous Sequence Config */
		status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, SequenceConfig);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		
		p_devData[sensor_index].SequenceConfig = SequenceConfig;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Perform_Ref_Calibration(uint8_t sensor_index, uint8_t *pVhvSettings, uint8_t *pPhaseCal, uint8_t get_data_enable)
{
	VL53L0X_OpResult status;
	uint8_t SequenceConfig = 0;

	/* store the value of the sequence config * this will be reset before the end of the function
	 */

	SequenceConfig = p_devData[sensor_index].SequenceConfig;

	/* In the following function we don't save the config to optimize
	 * writes on device. Config is saved and restored only once. */
	status = VL53L0X_Perform_VHV_Calibration(sensor_index, pVhvSettings, get_data_enable, 0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Perform_Phase_Calibration(sensor_index, pPhaseCal, get_data_enable, 0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* restore the previous Sequence Config */
	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, SequenceConfig);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	p_devData[sensor_index].SequenceConfig = SequenceConfig;

	return VL53L0X_OK;
}

void VL53L0X_Get_Next_Good_Spad(uint8_t goodSpadArray[], uint32_t size, uint32_t curr, int32_t *next)
{
	uint32_t startIndex;
	uint32_t fineOffset;
	uint32_t cSpadsPerByte = 8;
	uint32_t coarseIndex;
	uint32_t fineIndex;
	uint8_t dataByte;
	uint8_t success = 0;

	/*
	 * Starting with the current good spad, loop through the array to find
	 * the next. i.e. the next bit set in the sequence.
	 *
	 * The coarse index is the byte index of the array and the fine index is
	 * the index of the bit within each byte.
	 */

	*next = -1;

	startIndex = curr / cSpadsPerByte;
	fineOffset = curr % cSpadsPerByte;

	for (coarseIndex = startIndex; ((coarseIndex < size) && !success); coarseIndex++)
	{
		fineIndex = 0;
		dataByte = goodSpadArray[coarseIndex];

		if (coarseIndex == startIndex)
		{
			/* locate the bit position of the provided current
			 * spad bit before iterating */
			dataByte >>= fineOffset;
			fineIndex = fineOffset;
		}

		while (fineIndex < cSpadsPerByte)
		{
			if ((dataByte & 0x1) == 1) {
				success = 1;
				*next = coarseIndex * cSpadsPerByte + fineIndex;
				break;
			}
			dataByte >>= 1;
			fineIndex++;
		}
	}
}

uint8_t VL53L0X_Is_Aperture(uint32_t spadIndex)
{
	/*
	 * This function reports if a given spad index is an aperture SPAD by
	 * deriving the quadrant.
	 */
	uint32_t quadrant;
	uint8_t isAperture = 1;

	quadrant = spadIndex >> 6;
	if (refArrayQuadrants[quadrant] == REF_ARRAY_SPAD_0)
	{
		isAperture = 0;
	}

	return isAperture;
}

VL53L0X_OpResult VL53L0X_Enable_Spad_Bit(uint8_t spadArray[], uint32_t size, uint32_t spadIndex)
{
	uint32_t cSpadsPerByte = 8;
	uint32_t coarseIndex;
	uint32_t fineIndex;

	coarseIndex = spadIndex / cSpadsPerByte;
	fineIndex = spadIndex % cSpadsPerByte;

	if (coarseIndex >= size)
	{
		return VL53L0X_Ref_Spad_Init;
	}

	spadArray[coarseIndex] |= (1 << fineIndex);

	return VL53L0X_OK;
}

VL53L0X_OpResult VL53L0X_Set_Ref_Spad_Map(uint8_t sensor_index, uint8_t *refSpadArray)
{
	return VL53L0X_IO_Write_Multi(sensor_index, VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0, refSpadArray, 6);
}

VL53L0X_OpResult VL53L0X_Get_Ref_Spad_Map(uint8_t sensor_index, uint8_t *refSpadArray)
{
//	VL53L0X_OpResult status;
//	uint8_t count=0;

//	for (count = 0; count < 6; count++)
//        status = VL53L0X_IO_Read_Byte(sensor_index, (VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0 + count), &refSpadArray[count]);
	return VL53L0X_IO_Read_Multi(sensor_index, VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0, refSpadArray, 6);
}

VL53L0X_OpResult VL53L0X_Enable_Ref_Spads(uint8_t sensor_index, uint8_t apertureSpads, uint8_t goodSpadArray[], uint8_t spadArray[], uint32_t size, uint32_t start, uint32_t offset, uint32_t spadCount, uint32_t *lastSpad)
{
	VL53L0X_OpResult status;
	uint32_t index;
	uint32_t i;
	int32_t nextGoodSpad = offset;
	uint32_t currentSpad;
	uint8_t checkSpadArray[6];

	/*
	 * This function takes in a spad array which may or may not have SPADS
	 * already enabled and appends from a given offset a requested number
	 * of new SPAD enables. The 'good spad map' is applied to
	 * determine the next SPADs to enable.
	 *
	 * This function applies to only aperture or only non-aperture spads.
	 * Checks are performed to ensure this.
	 */

	currentSpad = offset;
	for (index = 0; index < spadCount; index++)
	{
		VL53L0X_Get_Next_Good_Spad(goodSpadArray, size, currentSpad, &nextGoodSpad);

		if (nextGoodSpad == -1)
		{
			return VL53L0X_Ref_Spad_Init;
		}

		/* Confirm that the next good SPAD is non-aperture */
		if (VL53L0X_Is_Aperture(start + nextGoodSpad) != apertureSpads)
		{
			/* if we can't get the required number of good aperture
			 * spads from the current quadrant then this is an error
			 */
			return VL53L0X_Ref_Spad_Init;
		}

		currentSpad = (uint32_t)nextGoodSpad;
		VL53L0X_Enable_Spad_Bit(spadArray, size, currentSpad);
		currentSpad++;
	}

	*lastSpad = currentSpad;

	status = VL53L0X_Set_Ref_Spad_Map(sensor_index, spadArray);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Get_Ref_Spad_Map(sensor_index, checkSpadArray);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	i = 0;

	/* Compare spad maps. If not equal report error. */
	while (i < size) {
		if (spadArray[i] != checkSpadArray[i]) {
			return VL53L0X_Ref_Spad_Init;
		}
		i++;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Set_Device_Mode(uint8_t sensor_index, VL53L0X_DeviceModes DeviceMode)
{
	switch (DeviceMode) {
	case VL53L0X_DEVICEMODE_SINGLE_RANGING:
	case VL53L0X_DEVICEMODE_CONTINUOUS_RANGING:
	case VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING:
	case VL53L0X_DEVICEMODE_GPIO_DRIVE:
	case VL53L0X_DEVICEMODE_GPIO_OSC:
		/* Supported modes */
		p_devData[sensor_index].CurrentParameters.DeviceMode = DeviceMode;
		break;
	default:
		/* Unsupported mode */
		return VL53L0X_Unsupport;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_Interrupt_Thresholds(uint8_t sensor_index, VL53L0X_DeviceModes DeviceMode, FixPoint1616_t *pThresholdLow, FixPoint1616_t *pThresholdHigh)
{
	VL53L0X_OpResult status;
	uint16_t Threshold16;

	/* no dependency on DeviceMode for Ewok */

	status = VL53L0X_IO_Read_Word(sensor_index, VL53L0X_REG_SYSTEM_THRESH_LOW, &Threshold16);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Need to multiply by 2 because the FW will apply a x2 */
	*pThresholdLow = ((FixPoint1616_t)(0x00fff & Threshold16) << 17);

	status = VL53L0X_IO_Read_Word(sensor_index, VL53L0X_REG_SYSTEM_THRESH_HIGH, &Threshold16);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Need to multiply by 2 because the FW will apply a x2 */
	*pThresholdHigh = ((FixPoint1616_t)(0x00fff & Threshold16) << 17);

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Load_Tuning_Settings(uint8_t sensor_index, uint8_t *pTuningSettingBuffer)
{
	VL53L0X_OpResult status;
	int i;
	int Index;
	uint8_t msb;
	uint8_t lsb;
	uint8_t SelectParam;
	uint8_t NumberOfWrites;
	uint8_t Address;
	uint8_t localBuffer[4]; /* max */
	uint16_t Temp16;

	Index = 0;

	while (*(pTuningSettingBuffer + Index) != 0)
	{
		NumberOfWrites = *(pTuningSettingBuffer + Index);
		Index++;
		if (NumberOfWrites == 0xFF)
		{
			/* internal parameters */
			SelectParam = *(pTuningSettingBuffer + Index);
			Index++;
			switch (SelectParam)
			{
				case 0: /* uint16_t SigmaEstRefArray -> 2 bytes */
					msb = *(pTuningSettingBuffer + Index);
					Index++;
					lsb = *(pTuningSettingBuffer + Index);
					Index++;
					Temp16 = VL53L0X_MAKEUINT16(lsb, msb);
					p_devData[sensor_index].SigmaEstRefArray = Temp16;
					break;
				case 1: /* uint16_t SigmaEstEffPulseWidth -> 2 bytes */
					msb = *(pTuningSettingBuffer + Index);
					Index++;
					lsb = *(pTuningSettingBuffer + Index);
					Index++;
					Temp16 = VL53L0X_MAKEUINT16(lsb, msb);
					p_devData[sensor_index].SigmaEstEffPulseWidth = Temp16;
					break;
				case 2: /* uint16_t SigmaEstEffAmbWidth -> 2 bytes */
					msb = *(pTuningSettingBuffer + Index);
					Index++;
					lsb = *(pTuningSettingBuffer + Index);
					Index++;
					Temp16 = VL53L0X_MAKEUINT16(lsb, msb);
					p_devData[sensor_index].SigmaEstEffAmbWidth = Temp16;
					break;
				case 3: /* uint16_t targetRefRate -> 2 bytes */
					msb = *(pTuningSettingBuffer + Index);
					Index++;
					lsb = *(pTuningSettingBuffer + Index);
					Index++;
					Temp16 = VL53L0X_MAKEUINT16(lsb, msb);
					p_devData[sensor_index].targetRefRate = Temp16;
					break;
				default: /* invalid parameter */
					return VL53L0X_InvalidParameter;
			}
		}
		else if (NumberOfWrites <= 4)
		{
			Address = *(pTuningSettingBuffer + Index);
			Index++;

			for (i = 0; i < NumberOfWrites; i++)
			{
				localBuffer[i] = *(pTuningSettingBuffer + Index);
				Index++;
			}

			status = VL53L0X_IO_Write_Multi(sensor_index, Address, localBuffer, NumberOfWrites);
			if(status < VL53L0X_OK)
			{
				return status;
			}
		}
		else
		{
			return VL53L0X_InvalidParameter;
		}
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Check_And_Load_Interrupt_Settings(uint8_t sensor_index, uint8_t StartNotStopFlag)
{
	uint8_t InterruptConfig;
	FixPoint1616_t ThresholdLow = 0;
	FixPoint1616_t ThresholdHigh = 0;
	VL53L0X_OpResult status;

	InterruptConfig = p_devData[sensor_index].DeviceSpecificParameters.Pin0GpioFunctionality;

	if ((InterruptConfig == VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_LOW) || (InterruptConfig == VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_HIGH) || (InterruptConfig == VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_OUT))
	{
		status = VL53L0X_Get_Interrupt_Thresholds(sensor_index, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, &ThresholdLow, &ThresholdHigh);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		if ((ThresholdLow > 255*65536) || (ThresholdHigh > 255*65536))
		{
			if (StartNotStopFlag != 0)
			{
				status = VL53L0X_Load_Tuning_Settings(sensor_index, InterruptThresholdSettings);
				if(status < VL53L0X_OK)
				{
					return status;
				}
			}
			else
			{
				status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x04);
				if(status < VL53L0X_OK)
				{
					return status;
				}
				status = VL53L0X_IO_Write_Byte(sensor_index, 0x70, 0x00);
				if(status < VL53L0X_OK)
				{
					return status;
				}
				status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x00);
				if(status < VL53L0X_OK)
				{
					return status;
				}
				status = VL53L0X_IO_Write_Byte(sensor_index, 0x80, 0x00);
				if(status < VL53L0X_OK)
				{
					return status;
				}
			}
		}
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Start_Measurement(uint8_t sensor_index)
{
	VL53L0X_OpResult status;
	VL53L0X_DeviceModes DeviceMode;
	uint8_t Byte;
	uint8_t StartStopByte = VL53L0X_REG_SYSRANGE_MODE_START_STOP;
	uint32_t LoopNb;

	/* Get Current DeviceMode */
	status = VL53L0X_GetDeviceMode(sensor_index, &DeviceMode);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, 0x80, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0x00, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0x91, p_devData[sensor_index].StopVariable);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0x00, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0x80, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	switch (DeviceMode) {
	case VL53L0X_DEVICEMODE_SINGLE_RANGING:
		status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSRANGE_START, 0x01);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		Byte = StartStopByte;
		/* Wait until start bit has been cleared */
		LoopNb = 0;
		do {
			if (LoopNb > 0)
			{
				status = VL53L0X_IO_Read_Byte(sensor_index, VL53L0X_REG_SYSRANGE_START, &Byte);
				if(status < VL53L0X_OK)
				{
					return status;
				}
			}
			LoopNb = LoopNb + 1;

			if((Byte & StartStopByte) != StartStopByte)
			{
				break;
			}

			if (LoopNb >= VL53L0X_DEFAULT_MAX_LOOP)
			{
				return VL53L0X_SlaveTimeout;
			}
		} while (1);

		break;
	case VL53L0X_DEVICEMODE_CONTINUOUS_RANGING:
		/* Back-to-back mode */

		/* Check if need to apply interrupt settings */
		status = VL53L0X_Check_And_Load_Interrupt_Settings(sensor_index, 1);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		/* Set PAL State to Running */
		p_devData[sensor_index].PalState = VL53L0X_STATE_RUNNING;
		break;
	case VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING:
		/* Continuous mode */
		/* Check if need to apply interrupt settings */
		status = VL53L0X_Check_And_Load_Interrupt_Settings(sensor_index, 1);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_TIMED);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		/* Set PAL State to Running */
		p_devData[sensor_index].PalState = VL53L0X_STATE_RUNNING;
		break;
	default:
		/* Selected mode not supported */
		return VL53L0X_Unsupport;
	}

	return status;
}

/* Group PAL Measurement Functions */
static VL53L0X_OpResult VL53L0X_Perform_Single_Measurement(uint8_t sensor_index)
{
	VL53L0X_OpResult status;
	VL53L0X_DeviceModes DeviceMode;

	/* Get Current DeviceMode */
	status = VL53L0X_GetDeviceMode(sensor_index, &DeviceMode);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Start immediately to run a single ranging measurement in case of
	 * single ranging or single histogram */
	if (DeviceMode == VL53L0X_DEVICEMODE_SINGLE_RANGING)
	{
		status = VL53L0X_Start_Measurement(sensor_index);
		if(status < VL53L0X_OK)
		{
			return status;
		}
	}

	status = VL53L0X_Measurement_Poll_For_Completion(sensor_index);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Change PAL State in case of single ranging or single histogram */
	if (DeviceMode == VL53L0X_DEVICEMODE_SINGLE_RANGING)
	{
		p_devData[sensor_index].PalState = VL53L0X_STATE_IDLE;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_XTalk_Compensation_Enable(uint8_t sensor_index, uint8_t *pXTalkCompensationEnable)
{
	uint8_t Temp8;

	Temp8 = p_devData[sensor_index].CurrentParameters.XTalkCompensationEnable;
	*pXTalkCompensationEnable = Temp8;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_Total_Xtalk_Rate(uint8_t sensor_index, VL53L0X_RangingMeasurementData_t *pRangingMeasurementData, FixPoint1616_t *ptotal_xtalk_rate_mcps)
{
	VL53L0X_OpResult status;

	uint8_t xtalkCompEnable;
	FixPoint1616_t totalXtalkMegaCps;
	FixPoint1616_t xtalkPerSpadMegaCps;

	*ptotal_xtalk_rate_mcps = 0;

	status = VL53L0X_Get_XTalk_Compensation_Enable(sensor_index, &xtalkCompEnable);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	if (xtalkCompEnable) {

		xtalkPerSpadMegaCps = p_devData[sensor_index].CurrentParameters.XTalkCompensationRateMegaCps;

		/* FixPoint1616 * FixPoint 8:8 = FixPoint0824 */
		totalXtalkMegaCps = pRangingMeasurementData->EffectiveSpadRtnCount * xtalkPerSpadMegaCps;

		/* FixPoint0824 >> 8 = FixPoint1616 */
		*ptotal_xtalk_rate_mcps = (totalXtalkMegaCps + 0x80) >> 8;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_Total_Signal_Rate(uint8_t sensor_index, VL53L0X_RangingMeasurementData_t *pRangingMeasurementData, FixPoint1616_t *ptotal_signal_rate_mcps)
{
	VL53L0X_OpResult status;
	FixPoint1616_t totalXtalkMegaCps;

	*ptotal_signal_rate_mcps = pRangingMeasurementData->SignalRateRtnMegaCps;

	status = VL53L0X_Get_Total_Xtalk_Rate(sensor_index, pRangingMeasurementData, &totalXtalkMegaCps);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	*ptotal_signal_rate_mcps += totalXtalkMegaCps;

	return VL53L0X_OK;
}

/* To convert ms into register value */
uint32_t VL53L0X_Calc_Timeout_Mclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks)
{
	uint32_t macro_period_ps;
	uint32_t macro_period_ns;
	uint32_t timeout_period_mclks = 0;

	macro_period_ps = VL53L0X_Calc_Macro_Period_pS(vcsel_period_pclks);
	macro_period_ns = (macro_period_ps + 500) / 1000;

	timeout_period_mclks = (uint32_t) (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);

    return timeout_period_mclks;
}

uint32_t VL53L0X_Isqrt(uint32_t num)
{
	/*
	 * Implements an integer square root
	 *
	 * From: http://en.wikipedia.org/wiki/Methods_of_computing_square_roots
	 */

	uint32_t  res = 0;
	uint32_t  bit = (uint32_t)1 << 30;
	/* The second-to-top bit is set:
	 *	1 << 14 for 16-bits, 1 << 30 for 32 bits */

	 /* "bit" starts at the highest power of four <= the argument. */
	while (bit > num)
	{
		bit >>= 2;
	}

	while (bit != 0)
	{
		if (num >= res + bit) {
			num -= res + bit;
			res = (res >> 1) + bit;
		} else
			res >>= 1;

		bit >>= 2;
	}

	return res;
}

static VL53L0X_OpResult VL53L0X_Calc_Dmax(uint8_t sensor_index, FixPoint1616_t totalSignalRate_mcps, FixPoint1616_t totalCorrSignalRate_mcps, FixPoint1616_t pwMult, uint32_t sigmaEstimateP1, FixPoint1616_t sigmaEstimateP2, uint32_t peakVcselDuration_us, uint32_t *pdmax_mm)
{
	const uint32_t cSigmaLimit		= 18;
	const FixPoint1616_t cSignalLimit	= 0x4000; /* 0.25 */
	const FixPoint1616_t cSigmaEstRef	= 0x00000042; /* 0.001 */
	const uint32_t cAmbEffWidthSigmaEst_ns = 6;
	const uint32_t cAmbEffWidthDMax_ns	   = 7;
	uint32_t dmaxCalRange_mm;
	FixPoint1616_t dmaxCalSignalRateRtn_mcps;
	FixPoint1616_t minSignalNeeded;
	FixPoint1616_t minSignalNeeded_p1;
	FixPoint1616_t minSignalNeeded_p2;
	FixPoint1616_t minSignalNeeded_p3;
	FixPoint1616_t minSignalNeeded_p4;
	FixPoint1616_t sigmaLimitTmp;
	FixPoint1616_t sigmaEstSqTmp;
	FixPoint1616_t signalLimitTmp;
	FixPoint1616_t SignalAt0mm;
	FixPoint1616_t dmaxDark;
	FixPoint1616_t dmaxAmbient;
	FixPoint1616_t dmaxDarkTmp;
	FixPoint1616_t sigmaEstP2Tmp;
	uint32_t signalRateTemp_mcps;

	dmaxCalRange_mm = p_devData[sensor_index].DmaxCalRangeMilliMeter;

	dmaxCalSignalRateRtn_mcps = p_devData[sensor_index].DmaxCalSignalRateRtnMegaCps;

	/* uint32 * FixPoint1616 = FixPoint1616 */
	SignalAt0mm = dmaxCalRange_mm * dmaxCalSignalRateRtn_mcps;

	/* FixPoint1616 >> 8 = FixPoint2408 */
	SignalAt0mm = (SignalAt0mm + 0x80) >> 8;
	SignalAt0mm *= dmaxCalRange_mm;

	minSignalNeeded_p1 = 0;

	if (totalCorrSignalRate_mcps > 0)
	{
		/* Shift by 10 bits to increase resolution prior to the
		 * division */
		signalRateTemp_mcps = totalSignalRate_mcps << 10;

		/* Add rounding value prior to division */
		minSignalNeeded_p1 = signalRateTemp_mcps + (totalCorrSignalRate_mcps/2);

		/* FixPoint0626/FixPoint1616 = FixPoint2210 */
		minSignalNeeded_p1 /= totalCorrSignalRate_mcps;

		/* Apply a factored version of the speed of light.
		 Correction to be applied at the end */
		minSignalNeeded_p1 *= 3;

		/* FixPoint2210 * FixPoint2210 = FixPoint1220 */
		minSignalNeeded_p1 *= minSignalNeeded_p1;

		/* FixPoint1220 >> 16 = FixPoint2804 */
		minSignalNeeded_p1 = (minSignalNeeded_p1 + 0x8000) >> 16;
	}

	minSignalNeeded_p2 = pwMult * sigmaEstimateP1;

	/* FixPoint1616 >> 16 =	 uint32 */
	minSignalNeeded_p2 = (minSignalNeeded_p2 + 0x8000) >> 16;

	/* uint32 * uint32	=  uint32 */
	minSignalNeeded_p2 *= minSignalNeeded_p2;

	/* Check sigmaEstimateP2
	 * If this value is too high there is not enough signal rate
	 * to calculate dmax value so set a suitable value to ensure
	 * a very small dmax.
	 */
	sigmaEstP2Tmp = (sigmaEstimateP2 + 0x8000) >> 16;
	sigmaEstP2Tmp = (sigmaEstP2Tmp + cAmbEffWidthSigmaEst_ns/2)/cAmbEffWidthSigmaEst_ns;
	sigmaEstP2Tmp *= cAmbEffWidthDMax_ns;

	if (sigmaEstP2Tmp > 0xffff)
	{
		minSignalNeeded_p3 = 0xfff00000;
	}
	else
	{
		/* DMAX uses a different ambient width from sigma, so apply
		 * correction.
		 * Perform division before multiplication to prevent overflow.
		 */
		sigmaEstimateP2 = (sigmaEstimateP2 + cAmbEffWidthSigmaEst_ns/2)/cAmbEffWidthSigmaEst_ns;
		sigmaEstimateP2 *= cAmbEffWidthDMax_ns;

		/* FixPoint1616 >> 16 = uint32 */
		minSignalNeeded_p3 = (sigmaEstimateP2 + 0x8000) >> 16;

		minSignalNeeded_p3 *= minSignalNeeded_p3;
	}

	/* FixPoint1814 / uint32 = FixPoint1814 */
	sigmaLimitTmp = ((cSigmaLimit << 14) + 500) / 1000;

	/* FixPoint1814 * FixPoint1814 = FixPoint3628 := FixPoint0428 */
	sigmaLimitTmp *= sigmaLimitTmp;

	/* FixPoint1616 * FixPoint1616 = FixPoint3232 */
	sigmaEstSqTmp = cSigmaEstRef * cSigmaEstRef;

	/* FixPoint3232 >> 4 = FixPoint0428 */
	sigmaEstSqTmp = (sigmaEstSqTmp + 0x08) >> 4;

	/* FixPoint0428 - FixPoint0428	= FixPoint0428 */
	sigmaLimitTmp -=  sigmaEstSqTmp;

	/* uint32_t * FixPoint0428 = FixPoint0428 */
	minSignalNeeded_p4 = 4 * 12 * sigmaLimitTmp;

	/* FixPoint0428 >> 14 = FixPoint1814 */
	minSignalNeeded_p4 = (minSignalNeeded_p4 + 0x2000) >> 14;

	/* uint32 + uint32 = uint32 */
	minSignalNeeded = (minSignalNeeded_p2 + minSignalNeeded_p3);

	/* uint32 / uint32 = uint32 */
	minSignalNeeded += (peakVcselDuration_us/2);
	minSignalNeeded /= peakVcselDuration_us;

	/* uint32 << 14 = FixPoint1814 */
	minSignalNeeded <<= 14;

	/* FixPoint1814 / FixPoint1814 = uint32 */
	minSignalNeeded += (minSignalNeeded_p4/2);
	minSignalNeeded /= minSignalNeeded_p4;

	/* FixPoint3200 * FixPoint2804 := FixPoint2804*/
	minSignalNeeded *= minSignalNeeded_p1;

	/* Apply correction by dividing by 1000000.
	 * This assumes 10E16 on the numerator of the equation
	 * and 10E-22 on the denominator.
	 * We do this because 32bit fix point calculation can't
	 * handle the larger and smaller elements of this equation * i.e. speed of light and pulse widths.
	 */
	minSignalNeeded = (minSignalNeeded + 500) / 1000;
	minSignalNeeded <<= 4;

	minSignalNeeded = (minSignalNeeded + 500) / 1000;

	/* FixPoint1616 >> 8 = FixPoint2408 */
	signalLimitTmp = (cSignalLimit + 0x80) >> 8;

	/* FixPoint2408/FixPoint2408 = uint32 */
	if (signalLimitTmp != 0)
	{
		dmaxDarkTmp = (SignalAt0mm + (signalLimitTmp / 2))/ signalLimitTmp;
	}
	else
	{
		dmaxDarkTmp = 0;
	}

	dmaxDark = VL53L0X_Isqrt(dmaxDarkTmp);

	/* FixPoint2408/FixPoint2408 = uint32 */
	if (minSignalNeeded != 0)
	{
		dmaxAmbient = (SignalAt0mm + minSignalNeeded/2)/ minSignalNeeded;
	}
	else
	{
		dmaxAmbient = 0;
	}

	dmaxAmbient = VL53L0X_Isqrt(dmaxAmbient);

	*pdmax_mm = dmaxDark;

	if (dmaxDark > dmaxAmbient)
	{
		*pdmax_mm = dmaxAmbient;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Calc_Sigma_Estimate(uint8_t sensor_index, VL53L0X_RangingMeasurementData_t *pRangingMeasurementData, FixPoint1616_t *pSigmaEstimate, uint32_t *pDmax_mm)
{
	/* Expressed in 100ths of a ns, i.e. centi-ns */
	const uint32_t cPulseEffectiveWidth_centi_ns   = 800;
	/* Expressed in 100ths of a ns, i.e. centi-ns */
	const uint32_t cAmbientEffectiveWidth_centi_ns = 600;
	const FixPoint1616_t cDfltFinalRangeIntegrationTimeMilliSecs	= 0x00190000; /* 25ms */
	const uint32_t cVcselPulseWidth_ps	= 4700; /* pico secs */
	const FixPoint1616_t cSigmaEstMax	= 0x028F87AE;
	const FixPoint1616_t cSigmaEstRtnMax	= 0xF000;
	const FixPoint1616_t cAmbToSignalRatioMax = 0xF0000000/
		cAmbientEffectiveWidth_centi_ns;
	/* Time Of Flight per mm (6.6 pico secs) */
	const FixPoint1616_t cTOF_per_mm_ps		= 0x0006999A;
	const uint32_t c16BitRoundingParam		= 0x00008000;
	const FixPoint1616_t cMaxXTalk_kcps		= 0x00320000;
	const uint32_t cPllPeriod_ps			= 1655;

	uint32_t vcselTotalEventsRtn;
	uint32_t finalRangeTimeoutMicroSecs;
	uint32_t preRangeTimeoutMicroSecs;
	uint32_t finalRangeIntegrationTimeMilliSecs;
	FixPoint1616_t sigmaEstimateP1;
	FixPoint1616_t sigmaEstimateP2;
	FixPoint1616_t sigmaEstimateP3;
	FixPoint1616_t deltaT_ps;
	FixPoint1616_t pwMult;
	FixPoint1616_t sigmaEstRtn;
	FixPoint1616_t sigmaEstimate;
	FixPoint1616_t xTalkCorrection;
	FixPoint1616_t ambientRate_kcps;
	FixPoint1616_t peakSignalRate_kcps;
	FixPoint1616_t xTalkCompRate_mcps;
	uint32_t xTalkCompRate_kcps;
	VL53L0X_OpResult status;
	FixPoint1616_t diff1_mcps;
	FixPoint1616_t diff2_mcps;
	FixPoint1616_t sqr1;
	FixPoint1616_t sqr2;
	FixPoint1616_t sqrSum;
	FixPoint1616_t sqrtResult_centi_ns;
	FixPoint1616_t sqrtResult;
	FixPoint1616_t totalSignalRate_mcps;
	FixPoint1616_t correctedSignalRate_mcps;
	FixPoint1616_t sigmaEstRef;
	uint32_t vcselWidth;
	uint32_t finalRangeMacroPCLKS;
	uint32_t preRangeMacroPCLKS;
	uint32_t peakVcselDuration_us;
	uint8_t finalRangeVcselPCLKS;
	uint8_t preRangeVcselPCLKS;

	/*! \addtogroup calc_sigma_estimate
	 * @{
	 *
	 * Estimates the range sigma
	 */

	xTalkCompRate_mcps = p_devData[sensor_index].CurrentParameters.XTalkCompensationRateMegaCps;

	/*
	 * We work in kcps rather than mcps as this helps keep within the
	 * confines of the 32 Fix1616 type.
	 */

	ambientRate_kcps = (pRangingMeasurementData->AmbientRateRtnMegaCps * 1000) >> 16;

	correctedSignalRate_mcps = pRangingMeasurementData->SignalRateRtnMegaCps;

	status = VL53L0X_Get_Total_Signal_Rate(sensor_index, pRangingMeasurementData, &totalSignalRate_mcps);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_Get_Total_Xtalk_Rate(sensor_index, pRangingMeasurementData, &xTalkCompRate_mcps);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Signal rate measurement provided by device is the
	 * peak signal rate, not average.
	 */
	peakSignalRate_kcps = (totalSignalRate_mcps * 1000);
	peakSignalRate_kcps = (peakSignalRate_kcps + 0x8000) >> 16;

	xTalkCompRate_kcps = xTalkCompRate_mcps * 1000;

	if (xTalkCompRate_kcps > cMaxXTalk_kcps)
	{
		xTalkCompRate_kcps = cMaxXTalk_kcps;
	}

	/* Calculate final range macro periods */
	finalRangeTimeoutMicroSecs = p_devData[sensor_index].DeviceSpecificParameters.FinalRangeTimeoutMicroSecs;

	finalRangeVcselPCLKS = p_devData[sensor_index].DeviceSpecificParameters.FinalRangeVcselPulsePeriod;

	finalRangeMacroPCLKS = VL53L0X_Calc_Timeout_Mclks(finalRangeTimeoutMicroSecs, finalRangeVcselPCLKS);

	/* Calculate pre-range macro periods */
	preRangeTimeoutMicroSecs = p_devData[sensor_index].DeviceSpecificParameters.PreRangeTimeoutMicroSecs;

	preRangeVcselPCLKS = p_devData[sensor_index].DeviceSpecificParameters.PreRangeVcselPulsePeriod;

	preRangeMacroPCLKS = VL53L0X_Calc_Timeout_Mclks(
		preRangeTimeoutMicroSecs, preRangeVcselPCLKS);

	vcselWidth = 3;
	if (finalRangeVcselPCLKS == 8)
	{
		vcselWidth = 2;
	}

	peakVcselDuration_us = vcselWidth * 2048 * (preRangeMacroPCLKS + finalRangeMacroPCLKS);
	peakVcselDuration_us = (peakVcselDuration_us + 500)/1000;
	peakVcselDuration_us *= cPllPeriod_ps;
	peakVcselDuration_us = (peakVcselDuration_us + 500)/1000;

	/* Fix1616 >> 8 = Fix2408 */
	totalSignalRate_mcps = (totalSignalRate_mcps + 0x80) >> 8;

	/* Fix2408 * uint32 = Fix2408 */
	vcselTotalEventsRtn = totalSignalRate_mcps * peakVcselDuration_us;

	/* Fix2408 >> 8 = uint32 */
	vcselTotalEventsRtn = (vcselTotalEventsRtn + 0x80) >> 8;

	/* Fix2408 << 8 = Fix1616 = */
	totalSignalRate_mcps <<= 8;

	if (peakSignalRate_kcps == 0)
	{
		*pSigmaEstimate = cSigmaEstMax;
		p_devData[sensor_index].SigmaEstimate = cSigmaEstMax;
		*pDmax_mm = 0;
	}
	else
	{
		if (vcselTotalEventsRtn < 1)
		{
			vcselTotalEventsRtn = 1;
		}

		sigmaEstimateP1 = cPulseEffectiveWidth_centi_ns;

		/* ((FixPoint1616 << 16)* uint32)/uint32 = FixPoint1616 */
		sigmaEstimateP2 = (ambientRate_kcps << 16)/peakSignalRate_kcps;
		if (sigmaEstimateP2 > cAmbToSignalRatioMax)
		{
			/* Clip to prevent overflow. Will ensure safe
			 * max result. */
			sigmaEstimateP2 = cAmbToSignalRatioMax;
		}
		sigmaEstimateP2 *= cAmbientEffectiveWidth_centi_ns;

		sigmaEstimateP3 = 2 * VL53L0X_Isqrt(vcselTotalEventsRtn * 12);

		/* uint32 * FixPoint1616 = FixPoint1616 */
		deltaT_ps = pRangingMeasurementData->RangeMilliMeter * cTOF_per_mm_ps;

		/*
		 * vcselRate - xtalkCompRate
		 * (uint32 << 16) - FixPoint1616 = FixPoint1616.
		 * Divide result by 1000 to convert to mcps.
		 * 500 is added to ensure rounding when integer division
		 * truncates.
		 */
		diff1_mcps = (((peakSignalRate_kcps << 16) - 2 * xTalkCompRate_kcps) + 500)/1000;

		/* vcselRate + xtalkCompRate */
		diff2_mcps = ((peakSignalRate_kcps << 16) + 500)/1000;

		/* Shift by 8 bits to increase resolution prior to the
		 * division */
		diff1_mcps <<= 8;

		/* FixPoint0824/FixPoint1616 = FixPoint2408 */
//		xTalkCorrection	 = abs(diff1_mcps/diff2_mcps);
// abs is causing compiler overloading isue in C++, but unsigned types. So, redundant call anyway!
		xTalkCorrection	 = diff1_mcps/diff2_mcps;

		/* FixPoint2408 << 8 = FixPoint1616 */
		xTalkCorrection <<= 8;

		if(pRangingMeasurementData->RangeStatus != 0)
		{
			pwMult = (FixPoint1616_t)1 << 16;
		}
		else
		{
			/* FixPoint1616/uint32 = FixPoint1616 */
			pwMult = deltaT_ps/cVcselPulseWidth_ps; /* smaller than 1.0f */

			/*
			 * FixPoint1616 * FixPoint1616 = FixPoint3232, however both
			 * values are small enough such that32 bits will not be
			 * exceeded.
			 */
			pwMult *= (((FixPoint1616_t)1 << 16) - xTalkCorrection);

			/* (FixPoint3232 >> 16) = FixPoint1616 */
			pwMult =  (pwMult + c16BitRoundingParam) >> 16;

			/* FixPoint1616 + FixPoint1616 = FixPoint1616 */
			pwMult += ((FixPoint1616_t)1 << 16);

			/*
			 * At this point the value will be 1.xx, therefore if we square
			 * the value this will exceed 32 bits. To address this perform
			 * a single shift to the right before the multiplication.
			 */
			pwMult >>= 1;
			/* FixPoint1715 * FixPoint1715 = FixPoint3430 */
			pwMult = pwMult * pwMult;

			/* (FixPoint3430 >> 14) = Fix1616 */
			pwMult >>= 14;
		}

		/* FixPoint1616 * uint32 = FixPoint1616 */
		sqr1 = pwMult * sigmaEstimateP1;

		/* (FixPoint1616 >> 16) = FixPoint3200 */
		sqr1 = (sqr1 + 0x8000) >> 16;

		/* FixPoint3200 * FixPoint3200 = FixPoint6400 */
		sqr1 *= sqr1;

		sqr2 = sigmaEstimateP2;

		/* (FixPoint1616 >> 16) = FixPoint3200 */
		sqr2 = (sqr2 + 0x8000) >> 16;

		/* FixPoint3200 * FixPoint3200 = FixPoint6400 */
		sqr2 *= sqr2;

		/* FixPoint64000 + FixPoint6400 = FixPoint6400 */
		sqrSum = sqr1 + sqr2;

		/* SQRT(FixPoin6400) = FixPoint3200 */
		sqrtResult_centi_ns = VL53L0X_Isqrt(sqrSum);

		/* (FixPoint3200 << 16) = FixPoint1616 */
		sqrtResult_centi_ns <<= 16;

		/*
		 * Note that the Speed Of Light is expressed in um per 1E-10
		 * seconds (2997) Therefore to get mm/ns we have to divide by
		 * 10000
		 */
		sigmaEstRtn = (((sqrtResult_centi_ns+50)/100) / sigmaEstimateP3);
		sigmaEstRtn *= VL53L0X_SPEED_OF_LIGHT_IN_AIR;

		/* Add 5000 before dividing by 10000 to ensure rounding. */
		sigmaEstRtn += 5000;
		sigmaEstRtn /= 10000;

		if (sigmaEstRtn > cSigmaEstRtnMax) {
			/* Clip to prevent overflow. Will ensure safe
			 * max result. */
			sigmaEstRtn = cSigmaEstRtnMax;
		}

		finalRangeIntegrationTimeMilliSecs = (finalRangeTimeoutMicroSecs + preRangeTimeoutMicroSecs + 500)/1000;

		/* sigmaEstRef = 1mm * 25ms/final range integration time (inc pre-range)
		 * sqrt(FixPoint1616/int) = FixPoint2408)
		 */
		sigmaEstRef = VL53L0X_Isqrt((cDfltFinalRangeIntegrationTimeMilliSecs + finalRangeIntegrationTimeMilliSecs/2)/finalRangeIntegrationTimeMilliSecs);

		/* FixPoint2408 << 8 = FixPoint1616 */
		sigmaEstRef <<= 8;
		sigmaEstRef = (sigmaEstRef + 500)/1000;

		/* FixPoint1616 * FixPoint1616 = FixPoint3232 */
		sqr1 = sigmaEstRtn * sigmaEstRtn;
		/* FixPoint1616 * FixPoint1616 = FixPoint3232 */
		sqr2 = sigmaEstRef * sigmaEstRef;

		/* sqrt(FixPoint3232) = FixPoint1616 */
		sqrtResult = VL53L0X_Isqrt((sqr1 + sqr2));
		/*
		 * Note that the Shift by 4 bits increases resolution prior to
		 * the sqrt, therefore the result must be shifted by 2 bits to
		 * the right to revert back to the FixPoint1616 format.
		 */

		sigmaEstimate = 1000 * sqrtResult;

		if ((peakSignalRate_kcps < 1) || (vcselTotalEventsRtn < 1) || (sigmaEstimate > cSigmaEstMax))
		{
			sigmaEstimate = cSigmaEstMax;
		}

		*pSigmaEstimate = (uint32_t)(sigmaEstimate);
		p_devData[sensor_index].SigmaEstimate = *pSigmaEstimate;

		status = VL53L0X_Calc_Dmax(sensor_index, totalSignalRate_mcps, correctedSignalRate_mcps, pwMult, sigmaEstimateP1, sigmaEstimateP2, peakVcselDuration_us, pDmax_mm);
		if(status < VL53L0X_OK)
		{
			return status;
		}
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_Pal_Range_Status(uint8_t sensor_index, uint8_t DeviceRangestatus, FixPoint1616_t SignalRate, uint16_t EffectiveSpadRtnCount, VL53L0X_RangingMeasurementData_t *pRangingMeasurementData, uint8_t *pPalRangestatus)
{
	VL53L0X_OpResult status;
	uint8_t NoneFlag;
	uint8_t SigmaLimitflag = 0;
	uint8_t SignalRefClipflag = 0;
	uint8_t RangeIgnoreThresholdflag = 0;
	uint8_t SigmaLimitCheckEnable = 0;
	uint8_t SignalRateFinalRangeLimitCheckEnable = 0;
	uint8_t SignalRefClipLimitCheckEnable = 0;
	uint8_t RangeIgnoreThresholdLimitCheckEnable = 0;
	FixPoint1616_t SigmaEstimate;
	FixPoint1616_t SigmaLimitValue;
	FixPoint1616_t SignalRefClipValue;
	FixPoint1616_t RangeIgnoreThresholdValue;
	FixPoint1616_t SignalRatePerSpad;
	uint8_t DeviceRangestatusInternal = 0;
	uint16_t tmpWord = 0;
	uint8_t Temp8;
	uint32_t Dmax_mm = 0;
	FixPoint1616_t LastSignalRefMcps;

	/*
	 * VL53L0X has a good ranging when the value of the
	 * DeviceRangestatus = 11. This function will replace the value 0 with
	 * the value 11 in the DeviceRangestatus.
	 * In addition, the SigmaEstimator is not included in the VL53L0X
	 * DeviceRangestatus, this will be added in the PalRangestatus.
	 */

	DeviceRangestatusInternal = ((DeviceRangestatus & 0x78) >> 3);

	if (DeviceRangestatusInternal == 0 || DeviceRangestatusInternal == 5 || DeviceRangestatusInternal == 7 || DeviceRangestatusInternal == 12 || DeviceRangestatusInternal == 13 || DeviceRangestatusInternal == 14 || DeviceRangestatusInternal == 15)
	{
		NoneFlag = 1;
	}
	else
	{
		NoneFlag = 0;
	}

	/*
	 * Check if Sigma limit is enabled, if yes then do comparison with limit
	 * value and put the result back into pPalRangestatus.
	 */
	status = VL53L0X_GetLimitCheckEnable(sensor_index, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, &SigmaLimitCheckEnable);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	if (SigmaLimitCheckEnable != 0)
	{
		/*
		* compute the Sigma and check with limit
		*/
		status = VL53L0X_Calc_Sigma_Estimate(sensor_index, pRangingMeasurementData, &SigmaEstimate, &Dmax_mm);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		status = VL53L0X_GetLimitCheckValue(sensor_index, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, &SigmaLimitValue);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		if ((SigmaLimitValue > 0) && (SigmaEstimate > SigmaLimitValue))
		{
			/* Limit Fail */
			SigmaLimitflag = 1;
		}
	}

	/*
	 * Check if Signal ref clip limit is enabled, if yes then do comparison
	 * with limit value and put the result back into pPalRangestatus.
	 */
	status = VL53L0X_GetLimitCheckEnable(sensor_index, VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP, &SignalRefClipLimitCheckEnable);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	if ((SignalRefClipLimitCheckEnable != 0))
	{
		status = VL53L0X_GetLimitCheckValue(sensor_index, VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP, &SignalRefClipValue);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		/* Read LastSignalRefMcps from device */
		status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x01);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		status = VL53L0X_IO_Read_Word(sensor_index, VL53L0X_REG_RESULT_PEAK_SIGNAL_RATE_REF, &tmpWord);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x00);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		LastSignalRefMcps = VL53L0X_FIXPOINT97TOFIXPOINT1616(tmpWord);
		p_devData[sensor_index].LastSignalRefMcps = LastSignalRefMcps;

		if ((SignalRefClipValue > 0) && (LastSignalRefMcps > SignalRefClipValue))
		{
			/* Limit Fail */
			SignalRefClipflag = 1;
		}
	}

	/*
	 * Check if Signal ref clip limit is enabled, if yes then do comparison
	 * with limit value and put the result back into pPalRangestatus.
	 * EffectiveSpadRtnCount has a format 8.8
	 * If (Return signal rate < (1.5 x Xtalk x number of Spads)) : FAIL
	 */
	status = VL53L0X_GetLimitCheckEnable(sensor_index, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, &RangeIgnoreThresholdLimitCheckEnable);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	if (RangeIgnoreThresholdLimitCheckEnable != 0)
	{
		/* Compute the signal rate per spad */
		if (EffectiveSpadRtnCount == 0)
		{
			SignalRatePerSpad = 0;
		}
		else
		{
			SignalRatePerSpad = (FixPoint1616_t)((256 * SignalRate) / EffectiveSpadRtnCount);
		}

		status = VL53L0X_GetLimitCheckValue(sensor_index, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, &RangeIgnoreThresholdValue);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		if ((RangeIgnoreThresholdValue > 0) && (SignalRatePerSpad < RangeIgnoreThresholdValue))
		{
			/* Limit Fail add 2^6 to range status */
			RangeIgnoreThresholdflag = 1;
		}
	}

	if (NoneFlag == 1)
	{
		*pPalRangestatus = 255;	 /* NONE */
	}
	else if (DeviceRangestatusInternal == 1 || DeviceRangestatusInternal == 2 || DeviceRangestatusInternal == 3)
	{
		*pPalRangestatus = 5; /* HW fail */
	}
	else if (DeviceRangestatusInternal == 6 || DeviceRangestatusInternal == 9)
	{
		*pPalRangestatus = 4;  /* Phase fail */
	}
	else if (DeviceRangestatusInternal == 8 || DeviceRangestatusInternal == 10 || SignalRefClipflag == 1)
	{
		*pPalRangestatus = 3;  /* Min range */
	}
	else if (DeviceRangestatusInternal == 4 || RangeIgnoreThresholdflag == 1)
	{
		*pPalRangestatus = 2;  /* Signal Fail */
	}
	else if (SigmaLimitflag == 1)
	{
		*pPalRangestatus = 1;  /* Sigma	 Fail */
	}
	else
	{
		*pPalRangestatus = 0; /* Range Valid */
	}

	/* DMAX only relevant during range error */
	if (*pPalRangestatus == 0)
	{
		pRangingMeasurementData->RangeDMaxMilliMeter = 0;
	}

	/* fill the Limit Check status */

	status = VL53L0X_GetLimitCheckEnable(sensor_index, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, &SignalRateFinalRangeLimitCheckEnable);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	if ((SigmaLimitCheckEnable == 0) || (SigmaLimitflag == 1))
	{
		Temp8 = 1;
	}
	else
	{
		Temp8 = 0;
	}

	p_devData[sensor_index].CurrentParameters.LimitChecksStatus[VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE] = Temp8;

	if ((DeviceRangestatusInternal == 4) || (SignalRateFinalRangeLimitCheckEnable == 0))
	{
		Temp8 = 1;
	}
	else
	{
		Temp8 = 0;
	}

	p_devData[sensor_index].CurrentParameters.LimitChecksStatus[VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE] = Temp8;

	if ((SignalRefClipLimitCheckEnable == 0) || (SignalRefClipflag == 1))
	{
		Temp8 = 1;
	}
	else
	{
		Temp8 = 0;
	}

	p_devData[sensor_index].CurrentParameters.LimitChecksStatus[VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP] = Temp8;

	if ((RangeIgnoreThresholdLimitCheckEnable == 0) || (RangeIgnoreThresholdflag == 1))
	{
		Temp8 = 1;
	}
	else
	{
		Temp8 = 0;
	}

	p_devData[sensor_index].CurrentParameters.LimitChecksStatus[VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD] = Temp8;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_Ranging_Measurement_Data(uint8_t sensor_index, VL53L0X_RangingMeasurementData_t *pRangingMeasurementData)
{
	VL53L0X_OpResult status;
	uint8_t DeviceRangestatus;
	uint8_t RangeFractionalEnable;
	uint8_t PalRangestatus = 255;
	uint8_t XTalkCompensationEnable;
	uint16_t AmbientRate;
	FixPoint1616_t SignalRate;
	uint16_t XTalkCompensationRateMegaCps;
	uint16_t EffectiveSpadRtnCount;
	uint16_t tmpuint16;
	uint16_t XtalkRangeMilliMeter;
	uint16_t LinearityCorrectiveGain;
	uint8_t localBuffer[12];
	VL53L0X_RangingMeasurementData_t LastRangeDataBuffer;

	/*
	 * use multi read even if some registers are not useful, result will
	 * be more efficient
	 * start reading at 0x14 dec20
	 * end reading at 0x21 dec33 total 14 bytes to read
	 */
	status = VL53L0X_IO_Read_Multi(sensor_index, 0x14, localBuffer, 12);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	pRangingMeasurementData->ZoneId = 0; /* Only one zone */
	pRangingMeasurementData->TimeStamp = 0; /* Not Implemented */

	tmpuint16 = VL53L0X_MAKEUINT16(localBuffer[11], localBuffer[10]);
	/* cut1.1 if SYSTEM__RANGE_CONFIG if 1 range is 2bits fractional
		*(format 11.2) else no fractional
		*/

	pRangingMeasurementData->MeasurementTimeUsec = 0;

	SignalRate = VL53L0X_FIXPOINT97TOFIXPOINT1616(VL53L0X_MAKEUINT16(localBuffer[7], localBuffer[6]));
	/* peak_signal_count_rate_rtn_mcps */
	pRangingMeasurementData->SignalRateRtnMegaCps = SignalRate;

	AmbientRate = VL53L0X_MAKEUINT16(localBuffer[9], localBuffer[8]);
	pRangingMeasurementData->AmbientRateRtnMegaCps = VL53L0X_FIXPOINT97TOFIXPOINT1616(AmbientRate);

	EffectiveSpadRtnCount = VL53L0X_MAKEUINT16(localBuffer[3], localBuffer[2]);
	/* EffectiveSpadRtnCount is 8.8 format */
	pRangingMeasurementData->EffectiveSpadRtnCount = EffectiveSpadRtnCount;

	DeviceRangestatus = localBuffer[0];

	/* Get Linearity Corrective Gain */
	LinearityCorrectiveGain = p_devData[sensor_index].LinearityCorrectiveGain;

	/* Get ranging configuration */
	RangeFractionalEnable = p_devData[sensor_index].RangeFractionalEnable;

	if (LinearityCorrectiveGain != 1000)
	{
		tmpuint16 = (uint16_t)((LinearityCorrectiveGain * tmpuint16 + 500) / 1000);

		/* Implement Xtalk */
		XTalkCompensationRateMegaCps = p_devData[sensor_index].CurrentParameters.XTalkCompensationRateMegaCps;
		XTalkCompensationEnable = p_devData[sensor_index].CurrentParameters.XTalkCompensationEnable;

		if (XTalkCompensationEnable)
		{
			if ((SignalRate - ((XTalkCompensationRateMegaCps * EffectiveSpadRtnCount) >> 8)) <= 0)
			{
				if (RangeFractionalEnable)
				{
					XtalkRangeMilliMeter = 8888;
				}
				else
				{
					XtalkRangeMilliMeter = 8888 << 2;
				}
			}
			else
			{
				XtalkRangeMilliMeter = (tmpuint16 * SignalRate) / (SignalRate - ((XTalkCompensationRateMegaCps * EffectiveSpadRtnCount) >> 8));
			}

			tmpuint16 = XtalkRangeMilliMeter;
		}
	}

	if (RangeFractionalEnable)
	{
		pRangingMeasurementData->RangeMilliMeter = (uint16_t)((tmpuint16) >> 2);
		pRangingMeasurementData->RangeFractionalPart = (uint8_t)((tmpuint16 & 0x03) << 6);
	}
	else
	{
		pRangingMeasurementData->RangeMilliMeter = tmpuint16;
		pRangingMeasurementData->RangeFractionalPart = 0;
	}

	/*
		* For a standard definition of Rangestatus, this should
		* return 0 in case of good result after a ranging
		* The range status depends on the device so call a device
		* specific function to obtain the right status.
		*/
	status = VL53L0X_Get_Pal_Range_Status(sensor_index, DeviceRangestatus, SignalRate, EffectiveSpadRtnCount, pRangingMeasurementData, &PalRangestatus);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	pRangingMeasurementData->RangeStatus = PalRangestatus;

	/* Copy last read data into Dev buffer */
	LastRangeDataBuffer = p_devData[sensor_index].LastRangeMeasure;

	LastRangeDataBuffer.RangeMilliMeter = pRangingMeasurementData->RangeMilliMeter;
	LastRangeDataBuffer.RangeFractionalPart = pRangingMeasurementData->RangeFractionalPart;
	LastRangeDataBuffer.RangeDMaxMilliMeter = pRangingMeasurementData->RangeDMaxMilliMeter;
	LastRangeDataBuffer.MeasurementTimeUsec = pRangingMeasurementData->MeasurementTimeUsec;
	LastRangeDataBuffer.SignalRateRtnMegaCps = pRangingMeasurementData->SignalRateRtnMegaCps;
	LastRangeDataBuffer.AmbientRateRtnMegaCps = pRangingMeasurementData->AmbientRateRtnMegaCps;
	LastRangeDataBuffer.EffectiveSpadRtnCount = pRangingMeasurementData->EffectiveSpadRtnCount;
	LastRangeDataBuffer.RangeStatus = pRangingMeasurementData->RangeStatus;

	p_devData[sensor_index].LastRangeMeasure, LastRangeDataBuffer;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Perform_Single_Ranging_Measurement(uint8_t sensor_index, VL53L0X_RangingMeasurementData_t *pRangingMeasurementData)
{
	VL53L0X_OpResult status;

	/* This function will do a complete single ranging
	 * Here we fix the mode! */
	status = VL53L0X_Set_Device_Mode(sensor_index, VL53L0X_DEVICEMODE_SINGLE_RANGING);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Perform_Single_Measurement(sensor_index);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Get_Ranging_Measurement_Data(sensor_index, pRangingMeasurementData);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Clear_Interrupt_Mask(sensor_index, 0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	return VL53L0X_OK;
}

VL53L0X_OpResult VL53L0X_Perform_Ref_Signal_Measurement(uint8_t sensor_index, uint16_t *refSignalRate)
{
	VL53L0X_OpResult status;
	VL53L0X_RangingMeasurementData_t rangingMeasurementData;

	uint8_t SequenceConfig = 0;

	/* store the value of the sequence config * this will be reset before the end of the function
	 */

	SequenceConfig = p_devData[sensor_index].SequenceConfig;

	/*
	 * This function performs a reference signal rate measurement.
	 */
	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0xC0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Perform_Single_Ranging_Measurement(sensor_index, &rangingMeasurementData);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Read_Word(sensor_index, VL53L0X_REG_RESULT_PEAK_SIGNAL_RATE_REF, refSignalRate);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}

		/* restore the previous Sequence Config */
	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, SequenceConfig);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	p_devData[sensor_index].SequenceConfig, SequenceConfig;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Perform_Ref_Spad_Management(uint8_t sensor_index, uint32_t *refSpadCount, uint8_t *isApertureSpads)
{
	VL53L0X_OpResult status;
	uint8_t lastSpadArray[6];
	uint8_t startSelect = 0xB4;
	uint32_t minimumSpadCount = 3;
	uint32_t maxSpadCount = 44;
	uint32_t currentSpadIndex = 0;
	uint32_t lastSpadIndex = 0;
	int32_t nextGoodSpad = 0;
	uint16_t targetRefRate = 0x0A00; /* 20 MCPS in 9:7 format */
	uint16_t peakSignalRateRef;
	uint32_t needAptSpads = 0;
	uint32_t index = 0;
	uint32_t spadArraySize = 6;
	uint32_t signalRateDiff = 0;
	uint32_t lastSignalRateDiff = 0;
	uint8_t complete = 0;
	uint8_t VhvSettings = 0;
	uint8_t PhaseCal = 0;
	uint32_t refSpadCount_int = 0;
	uint8_t	 isApertureSpads_int = 0;

	/*
	 * The reference SPAD initialization procedure determines the minimum
	 * amount of reference spads to be enables to achieve a target reference
	 * signal rate and should be performed once during initialization.
	 *
	 * Either aperture or non-aperture spads are applied but never both.
	 * Firstly non-aperture spads are set, begining with 5 spads, and
	 * increased one spad at a time until the closest measurement to the
	 * target rate is achieved.
	 *
	 * If the target rate is exceeded when 5 non-aperture spads are enabled * initialization is performed instead with aperture spads.
	 *
	 * When setting spads, a 'Good Spad Map' is applied.
	 *
	 * This procedure operates within a SPAD window of interest of a maximum
	 * 44 spads.
	 * The start point is currently fixed to 180, which lies towards the end
	 * of the non-aperture quadrant and runs in to the adjacent aperture
	 * quadrant.
	 */


	targetRefRate = p_devData[sensor_index].targetRefRate;

	/*
	 * Initialize Spad arrays.
	 * Currently the good spad map is initialised to 'All good'.
	 * This is a short term implementation. The good spad map will be
	 * provided as an input.
	 * Note that there are 6 bytes. Only the first 44 bits will be used to
	 * represent spads.
	 */
	for (index = 0; index < spadArraySize; index++)
	{
		p_devData[sensor_index].SpadData.RefSpadEnables[index] = 0;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_GLOBAL_CONFIG_REF_EN_START_SELECT, startSelect);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_POWER_MANAGEMENT_GO1_POWER_FORCE, 0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Perform ref calibration */
	status = VL53L0X_Perform_Ref_Calibration(sensor_index, &VhvSettings, &PhaseCal, 0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Enable Minimum NON-APERTURE Spads */
	currentSpadIndex = 0;
	lastSpadIndex = currentSpadIndex;
	needAptSpads = 0;
	status = VL53L0X_Enable_Ref_Spads(sensor_index, needAptSpads, p_devData[sensor_index].SpadData.RefGoodSpadMap, p_devData[sensor_index].SpadData.RefSpadEnables, spadArraySize, startSelect, currentSpadIndex, minimumSpadCount, &lastSpadIndex);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	currentSpadIndex = lastSpadIndex;

	status = VL53L0X_Perform_Ref_Signal_Measurement(sensor_index, &peakSignalRateRef);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	if (peakSignalRateRef > targetRefRate)
	{
		/* Signal rate measurement too high		 * switch to APERTURE SPADs */

		for (index = 0; index < spadArraySize; index++)
			p_devData[sensor_index].SpadData.RefSpadEnables[index] = 0;


		/* Increment to the first APERTURE spad */
		while ((VL53L0X_Is_Aperture(startSelect + currentSpadIndex)
			== 0) && (currentSpadIndex < maxSpadCount)) {
			currentSpadIndex++;
		}

		needAptSpads = 1;

		status = VL53L0X_Enable_Ref_Spads(sensor_index, needAptSpads, p_devData[sensor_index].SpadData.RefGoodSpadMap, p_devData[sensor_index].SpadData.RefSpadEnables, spadArraySize, startSelect, currentSpadIndex, minimumSpadCount, &lastSpadIndex);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		currentSpadIndex = lastSpadIndex;
		status = VL53L0X_Perform_Ref_Signal_Measurement(sensor_index, &peakSignalRateRef);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		if (peakSignalRateRef > targetRefRate)
		{
			/* Signal rate still too high after
				* setting the minimum number of
				* APERTURE spads. Can do no more
				* therefore set the min number of
				* aperture spads as the result.
				*/
			isApertureSpads_int = 1;
			refSpadCount_int = minimumSpadCount;
		}
	}
	else
	{
		needAptSpads = 0;
	}

	if ((peakSignalRateRef < targetRefRate))
	{
		/* At this point, the minimum number of either aperture
		 * or non-aperture spads have been set. Proceed to add
		 * spads and perform measurements until the target
		 * reference is reached.
		 */
		isApertureSpads_int = needAptSpads;
		refSpadCount_int = minimumSpadCount;

		memcpy(lastSpadArray, p_devData[sensor_index].SpadData.RefSpadEnables, spadArraySize);
		lastSignalRateDiff = abs(peakSignalRateRef - targetRefRate);
		complete = 0;

		while (!complete)
		{
			VL53L0X_Get_Next_Good_Spad(p_devData[sensor_index].SpadData.RefGoodSpadMap, spadArraySize, currentSpadIndex, &nextGoodSpad);

			if (nextGoodSpad == -1)
			{
				return VL53L0X_Ref_Spad_Init;
			}

			/* Cannot combine Aperture and Non-Aperture spads, so
			 * ensure the current spad is of the correct type.
			 */
			if (VL53L0X_Is_Aperture((uint32_t)startSelect + nextGoodSpad) != needAptSpads)
			{
				/* At this point we have enabled the maximum
				 * number of Aperture spads.
				 */
				complete = 1;
				break;
			}

			(refSpadCount_int)++;

			currentSpadIndex = nextGoodSpad;
			status = VL53L0X_Enable_Spad_Bit(p_devData[sensor_index].SpadData.RefSpadEnables, spadArraySize, currentSpadIndex);
			if(status < VL53L0X_OK)
			{
				return status;
			}

			currentSpadIndex++;
			/* Proceed to apply the additional spad and
				* perform measurement. */
			status = VL53L0X_Set_Ref_Spad_Map(sensor_index, p_devData[sensor_index].SpadData.RefSpadEnables);
			if(status < VL53L0X_OK)
			{
				return status;
			}

			status = VL53L0X_Perform_Ref_Signal_Measurement(sensor_index, &peakSignalRateRef);
			if(status < VL53L0X_OK)
			{
				return status;
			}

			signalRateDiff = abs(peakSignalRateRef - targetRefRate);

			if (peakSignalRateRef > targetRefRate)
			{
				/* Select the spad map that provides the
				 * measurement closest to the target rate			 * either above or below it.
				 */
				if (signalRateDiff > lastSignalRateDiff)
				{
					/* Previous spad map produced a closer
					 * measurement, so choose this. */
					status = VL53L0X_Set_Ref_Spad_Map(sensor_index, lastSpadArray);
					memcpy(p_devData[sensor_index].SpadData.RefSpadEnables, lastSpadArray, spadArraySize);

					(refSpadCount_int)--;
				}
				complete = 1;
			}
			else
			{
				/* Continue to add spads */
				lastSignalRateDiff = signalRateDiff;
				memcpy(lastSpadArray, p_devData[sensor_index].SpadData.RefSpadEnables, spadArraySize);
			}
		} /* while */
	}

	*refSpadCount = refSpadCount_int;
	*isApertureSpads = isApertureSpads_int;

	p_devData[sensor_index].DeviceSpecificParameters.RefSpadsInitialised = 1;
	p_devData[sensor_index].DeviceSpecificParameters.ReferenceSpadCount = (uint8_t)(*refSpadCount);
	p_devData[sensor_index].DeviceSpecificParameters.ReferenceSpadType = *isApertureSpads;

	return status;
}

static VL53L0X_OpResult VL53L0X_Set_Reference_Spads(uint8_t sensor_index, uint32_t count, uint8_t isApertureSpads)
{
	VL53L0X_OpResult status;
	uint32_t currentSpadIndex = 0;
	uint8_t startSelect = 0xB4;
	uint32_t spadArraySize = 6;
	uint32_t maxSpadCount = 44;
	uint32_t lastSpadIndex;
	uint32_t index;

	/*
	 * This function applies a requested number of reference spads, either
	 * aperture or
	 * non-aperture, as requested.
	 * The good spad map will be applied.
	 */

	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x01);

	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_GLOBAL_CONFIG_REF_EN_START_SELECT, startSelect);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	for (index = 0; index < spadArraySize; index++)
	{
		p_devData[sensor_index].SpadData.RefSpadEnables[index] = 0;
	}

	if (isApertureSpads)
	{
		/* Increment to the first APERTURE spad */
		while ((VL53L0X_Is_Aperture(startSelect + currentSpadIndex) == 0) &&
			  (currentSpadIndex < maxSpadCount)) {
			currentSpadIndex++;
		}
	}

	status = VL53L0X_Enable_Ref_Spads(sensor_index, isApertureSpads, p_devData[sensor_index].SpadData.RefGoodSpadMap, p_devData[sensor_index].SpadData.RefSpadEnables, spadArraySize, startSelect, currentSpadIndex, count, &lastSpadIndex);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	p_devData[sensor_index].DeviceSpecificParameters.RefSpadsInitialised = 1;
	p_devData[sensor_index].DeviceSpecificParameters.ReferenceSpadCount = (uint8_t)(count);
	p_devData[sensor_index].DeviceSpecificParameters.ReferenceSpadType = isApertureSpads;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Set_Gpio_Config(uint8_t sensor_index, uint8_t Pin, VL53L0X_DeviceModes DeviceMode, VL53L0X_GpioFunctionality Functionality, VL53L0X_InterruptPolarity Polarity)
{
	VL53L0X_OpResult status;
	uint8_t data;

	if (Pin != 0)
	{
		return VL53L0X_GPIO_Not_Exist;
	}
	else if (DeviceMode == VL53L0X_DEVICEMODE_GPIO_DRIVE)
	{
		if (Polarity == VL53L0X_INTERRUPTPOLARITY_LOW)
		{
			data = 0x10;
		}
		else
		{
			data = 1;
		}

		status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_GPIO_HV_MUX_ACTIVE_HIGH, data);
		if(status < VL53L0X_OK)
		{
			return status;
		}
	}
	else if (DeviceMode == VL53L0X_DEVICEMODE_GPIO_OSC)
	{
		status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x01);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x00, 0x00);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x00);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x80, 0x01);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x85, 0x02);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x04);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0xcd, 0x00);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0xcc, 0x11);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x07);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0xbe, 0x00);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x06);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0xcc, 0x09);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x00);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0xff, 0x01);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x00, 0x00);
		if(status < VL53L0X_OK)
		{
			return status;
		}
	}
	else
	{
		switch (Functionality)
		{
			case VL53L0X_GPIOFUNCTIONALITY_OFF:
				data = 0x00;
				break;
			case VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_LOW:
				data = 0x01;
				break;
			case VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_HIGH:
				data = 0x02;
				break;
			case VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_OUT:
				data = 0x03;
				break;
			case VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY:
				data = 0x04;
				break;
			default:
				return VL53L0X_GPIO_Function_Not_Support;
		}

		status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO, data);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		if (Polarity == VL53L0X_INTERRUPTPOLARITY_LOW)
		{
			data = 0;
		}
		else
		{
			data = (uint8_t)(1 << 4);
		}

		status = VL53L0X_IO_Update_Byte(sensor_index, VL53L0X_REG_GPIO_HV_MUX_ACTIVE_HIGH, 0xEF, data);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		p_devData[sensor_index].DeviceSpecificParameters.Pin0GpioFunctionality, Functionality;

		status = VL53L0X_Clear_Interrupt_Mask(sensor_index, 0);
		if(status < VL53L0X_OK)
		{
			return status;
		}
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Get_Fraction_Enable(uint8_t sensor_index, uint8_t *pEnabled)
{
	VL53L0X_OpResult status;

	status = VL53L0X_IO_Read_Byte(sensor_index, VL53L0X_REG_SYSTEM_RANGE_CONFIG, pEnabled);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	*pEnabled = (*pEnabled & 1);

	return VL53L0X_OK;
}

static uint16_t VL53L0X_Encode_Timeout(uint32_t timeout_macro_clks)
{
	/*!
	 * Encode timeout in macro periods in (LSByte * 2^MSByte) + 1 format
	 */

	uint16_t encoded_timeout = 0;
	uint32_t ls_byte = 0;
	uint16_t ms_byte = 0;

	if (timeout_macro_clks > 0)
	{
		ls_byte = timeout_macro_clks - 1;

		while ((ls_byte & 0xFFFFFF00) > 0) {
			ls_byte = ls_byte >> 1;
			ms_byte++;
		}

		encoded_timeout = (ms_byte << 8) + (uint16_t) (ls_byte & 0x000000FF);
	}

	return encoded_timeout;
}

static VL53L0X_OpResult VL53L0X_Set_Sequence_Step_Timeout(uint8_t sensor_index, VL53L0X_SequenceStepId SequenceStepId, uint32_t TimeOutMicroSecs)
{
	VL53L0X_OpResult status;
	uint8_t CurrentVCSELPulsePeriodPClk;
	uint8_t MsrcEncodedTimeOut;
	uint16_t PreRangeEncodedTimeOut;
	uint16_t PreRangeTimeOutMClks;
	uint16_t MsrcRangeTimeOutMClks;
	uint32_t FinalRangeTimeOutMClks;
	uint16_t FinalRangeEncodedTimeOut;
	VL53L0X_SchedulerSequenceSteps_t SchedulerSequenceSteps;

	if ((SequenceStepId == VL53L0X_SEQUENCESTEP_TCC) || (SequenceStepId == VL53L0X_SEQUENCESTEP_DSS) || (SequenceStepId == VL53L0X_SEQUENCESTEP_MSRC))
	{
		status = VL53L0X_Get_Vcsel_Pulse_Period(sensor_index, VL53L0X_VCSEL_PERIOD_PRE_RANGE, &CurrentVCSELPulsePeriodPClk);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		MsrcRangeTimeOutMClks = VL53L0X_Calc_Timeout_Mclks(TimeOutMicroSecs, (uint8_t)CurrentVCSELPulsePeriodPClk);

		if (MsrcRangeTimeOutMClks > 256)
		{
			MsrcEncodedTimeOut = 255;
		}
		else
		{
			MsrcEncodedTimeOut = (uint8_t)MsrcRangeTimeOutMClks - 1;
		}

		p_devData[sensor_index].DeviceSpecificParameters.LastEncodedTimeout = MsrcEncodedTimeOut;

		status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_MSRC_CONFIG_TIMEOUT_MACROP, MsrcEncodedTimeOut);
		if(status < VL53L0X_OK)
		{
			return status;
		}
	}
	else
	{
		if (SequenceStepId == VL53L0X_SEQUENCESTEP_PRE_RANGE)
		{
			status = VL53L0X_Get_Vcsel_Pulse_Period(sensor_index, VL53L0X_VCSEL_PERIOD_PRE_RANGE, &CurrentVCSELPulsePeriodPClk);
			if(status < VL53L0X_OK)
			{
				return status;
			}

			PreRangeTimeOutMClks = VL53L0X_Calc_Timeout_Mclks(TimeOutMicroSecs, (uint8_t)CurrentVCSELPulsePeriodPClk);
			PreRangeEncodedTimeOut = VL53L0X_Encode_Timeout(PreRangeTimeOutMClks);

			p_devData[sensor_index].DeviceSpecificParameters.LastEncodedTimeout = PreRangeEncodedTimeOut;

			status = VL53L0X_IO_Write_Word(sensor_index, VL53L0X_REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, PreRangeEncodedTimeOut);
			if(status < VL53L0X_OK)
			{
				return status;
			}

			p_devData[sensor_index].DeviceSpecificParameters.PreRangeTimeoutMicroSecs = TimeOutMicroSecs;
		}
		else if (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)
		{
			/* For the final range timeout, the pre-range timeout
			 * must be added. To do this both final and pre-range
			 * timeouts must be expressed in macro periods MClks
			 * because they have different vcsel periods.
			 */

			status = VL53L0X_GetSequenceStepEnables(sensor_index, &SchedulerSequenceSteps);
			if(status < VL53L0X_OK)
			{
				return status;
			}

			PreRangeTimeOutMClks = 0;
			if (SchedulerSequenceSteps.PreRangeOn)
			{
				/* Retrieve PRE-RANGE VCSEL Period */
				status = VL53L0X_Get_Vcsel_Pulse_Period(sensor_index, VL53L0X_VCSEL_PERIOD_PRE_RANGE, &CurrentVCSELPulsePeriodPClk);
				if(status < VL53L0X_OK)
				{
					return status;
				}

				/* Retrieve PRE-RANGE Timeout in Macro periods
				 * (MCLKS) */
				status = VL53L0X_IO_Read_Word(sensor_index, 0x51, &PreRangeEncodedTimeOut);
				if(status < VL53L0X_OK)
				{
					return status;
				}

				PreRangeTimeOutMClks = VL53L0X_Decode_Timeout(PreRangeEncodedTimeOut);
			}

			/* Calculate FINAL RANGE Timeout in Macro Periods
			 * (MCLKS) and add PRE-RANGE value
			 */
			status = VL53L0X_Get_Vcsel_Pulse_Period(sensor_index, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, &CurrentVCSELPulsePeriodPClk);
			if(status < VL53L0X_OK)
			{
				return status;
			}
		
			FinalRangeTimeOutMClks = VL53L0X_Calc_Timeout_Mclks(TimeOutMicroSecs, (uint8_t) CurrentVCSELPulsePeriodPClk);

			FinalRangeTimeOutMClks += PreRangeTimeOutMClks;

			FinalRangeEncodedTimeOut = VL53L0X_Encode_Timeout(FinalRangeTimeOutMClks);

			status = VL53L0X_IO_Write_Word(sensor_index, 0x71, FinalRangeEncodedTimeOut);
			if(status < VL53L0X_OK)
			{
				return status;
			}

			p_devData[sensor_index].DeviceSpecificParameters.FinalRangeTimeoutMicroSecs = TimeOutMicroSecs;
		}
		else
		{
			return VL53L0X_InvalidParameter;
		}
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Set_Measurement_Timing_Budget_Micro_Seconds(uint8_t sensor_index, uint32_t MeasurementTimingBudgetMicroSeconds)
{
	VL53L0X_OpResult status;
	uint32_t FinalRangeTimingBudgetMicroSeconds;
	VL53L0X_SchedulerSequenceSteps_t SchedulerSequenceSteps;
	uint32_t MsrcDccTccTimeoutMicroSeconds	= 2000;
	uint32_t StartOverheadMicroSeconds		= 1910;
	uint32_t EndOverheadMicroSeconds		= 960;
	uint32_t MsrcOverheadMicroSeconds		= 660;
	uint32_t TccOverheadMicroSeconds		= 590;
	uint32_t DssOverheadMicroSeconds		= 690;
	uint32_t PreRangeOverheadMicroSeconds	= 660;
	uint32_t FinalRangeOverheadMicroSeconds = 550;
	uint32_t PreRangeTimeoutMicroSeconds	= 0;
	uint32_t cMinTimingBudgetMicroSeconds	= 20000;
	uint32_t SubTimeout = 0;

	if (MeasurementTimingBudgetMicroSeconds < cMinTimingBudgetMicroSeconds)
	{
		return VL53L0X_InvalidParameter;
	}

	FinalRangeTimingBudgetMicroSeconds = MeasurementTimingBudgetMicroSeconds - (StartOverheadMicroSeconds + EndOverheadMicroSeconds);

	status = VL53L0X_GetSequenceStepEnables(sensor_index, &SchedulerSequenceSteps);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	if (SchedulerSequenceSteps.TccOn  || SchedulerSequenceSteps.MsrcOn || SchedulerSequenceSteps.DssOn)
	{
		/* TCC, MSRC and DSS all share the same timeout */
		status = VL53L0X_Get_Sequence_Step_Timeout(sensor_index, VL53L0X_SEQUENCESTEP_MSRC, &MsrcDccTccTimeoutMicroSeconds);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		/* Subtract the TCC, MSRC and DSS timeouts if they are
		 * enabled. */

		/* TCC */
		if (SchedulerSequenceSteps.TccOn)
		{
			SubTimeout = MsrcDccTccTimeoutMicroSeconds + TccOverheadMicroSeconds;

			if (SubTimeout < FinalRangeTimingBudgetMicroSeconds)
			{
				FinalRangeTimingBudgetMicroSeconds -= SubTimeout;
			}
			else
			{
				/* Requested timeout too big. */
				return VL53L0X_InvalidParameter;
			}
		}

		/* DSS */
		if (SchedulerSequenceSteps.DssOn)
		{
			SubTimeout = 2 * (MsrcDccTccTimeoutMicroSeconds + DssOverheadMicroSeconds);

			if (SubTimeout < FinalRangeTimingBudgetMicroSeconds)
			{
				FinalRangeTimingBudgetMicroSeconds -= SubTimeout;
			}
			else
			{
				/* Requested timeout too big. */
				return VL53L0X_InvalidParameter;
			}
		}
		else if (SchedulerSequenceSteps.MsrcOn)
		{
			/* MSRC */
			SubTimeout = MsrcDccTccTimeoutMicroSeconds + MsrcOverheadMicroSeconds;

			if (SubTimeout < FinalRangeTimingBudgetMicroSeconds)
			{
				FinalRangeTimingBudgetMicroSeconds -= SubTimeout;
			}
			else
			{
				/* Requested timeout too big. */
				return VL53L0X_InvalidParameter;
			}
		}

	}

	if (SchedulerSequenceSteps.PreRangeOn)
	{
		/* Subtract the Pre-range timeout if enabled. */
		status = VL53L0X_Get_Sequence_Step_Timeout(sensor_index, VL53L0X_SEQUENCESTEP_PRE_RANGE, &PreRangeTimeoutMicroSeconds);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		SubTimeout = PreRangeTimeoutMicroSeconds + PreRangeOverheadMicroSeconds;

		if (SubTimeout < FinalRangeTimingBudgetMicroSeconds)
		{
			FinalRangeTimingBudgetMicroSeconds -= SubTimeout;
		}
		else
		{
			/* Requested timeout too big. */
			return VL53L0X_InvalidParameter;
		}
	}


	if (SchedulerSequenceSteps.FinalRangeOn)
	{
		FinalRangeTimingBudgetMicroSeconds -= FinalRangeOverheadMicroSeconds;

		/* Final Range Timeout
		 * Note that the final range timeout is determined by the timing
		 * budget and the sum of all other timeouts within the sequence.
		 * If there is no room for the final range timeout, then an error
		 * will be set. Otherwise the remaining time will be applied to
		 * the final range.
		 */
		status = VL53L0X_Set_Sequence_Step_Timeout(sensor_index, VL53L0X_SEQUENCESTEP_FINAL_RANGE, FinalRangeTimingBudgetMicroSeconds);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		p_devData[sensor_index].CurrentParameters.MeasurementTimingBudgetMicroSeconds = MeasurementTimingBudgetMicroSeconds;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Set_Sequence_Step_Enable(uint8_t sensor_index, VL53L0X_SequenceStepId SequenceStepId, uint8_t SequenceStepEnabled)
{
	VL53L0X_OpResult status;
	uint8_t SequenceConfig = 0;
	uint8_t SequenceConfigNew = 0;
	uint32_t MeasurementTimingBudgetMicroSeconds;

	status = VL53L0X_IO_Read_Byte(sensor_index, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, &SequenceConfig);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	SequenceConfigNew = SequenceConfig;

	if (SequenceStepEnabled == 1)
	{
		/* Enable requested sequence step
			*/
		switch (SequenceStepId)
		{
			case VL53L0X_SEQUENCESTEP_TCC:
				SequenceConfigNew |= 0x10;
				break;
			case VL53L0X_SEQUENCESTEP_DSS:
				SequenceConfigNew |= 0x28;
				break;
			case VL53L0X_SEQUENCESTEP_MSRC:
				SequenceConfigNew |= 0x04;
				break;
			case VL53L0X_SEQUENCESTEP_PRE_RANGE:
				SequenceConfigNew |= 0x40;
				break;
			case VL53L0X_SEQUENCESTEP_FINAL_RANGE:
				SequenceConfigNew |= 0x80;
				break;
			default:
				return VL53L0X_InvalidParameter;
		}
	}
	else
	{
		/* Disable requested sequence step
			*/
		switch (SequenceStepId)
		{
			case VL53L0X_SEQUENCESTEP_TCC:
				SequenceConfigNew &= 0xef;
				break;
			case VL53L0X_SEQUENCESTEP_DSS:
				SequenceConfigNew &= 0xd7;
				break;
			case VL53L0X_SEQUENCESTEP_MSRC:
				SequenceConfigNew &= 0xfb;
				break;
			case VL53L0X_SEQUENCESTEP_PRE_RANGE:
				SequenceConfigNew &= 0xbf;
				break;
			case VL53L0X_SEQUENCESTEP_FINAL_RANGE:
				SequenceConfigNew &= 0x7f;
				break;
			default:
				return VL53L0X_InvalidParameter;
		}
	}

	if (SequenceConfigNew != SequenceConfig)
	{
		/* Apply New Setting */
		status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, SequenceConfigNew);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		p_devData[sensor_index].SequenceConfig = SequenceConfigNew;

		/* Recalculate timing budget */
		MeasurementTimingBudgetMicroSeconds = p_devData[sensor_index].CurrentParameters.MeasurementTimingBudgetMicroSeconds;

		status = VL53L0X_Set_Measurement_Timing_Budget_Micro_Seconds(sensor_index, MeasurementTimingBudgetMicroSeconds);
		if(status < VL53L0X_OK)
		{
			return status;
		}
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_Set_Limit_Check_Enable(uint8_t sensor_index, uint16_t LimitCheckId, uint8_t LimitCheckEnable)
{
	VL53L0X_OpResult status;
	FixPoint1616_t TempFix1616 = 0;
	uint8_t LimitCheckEnableInt = 0;
	uint8_t LimitCheckDisable = 0;
	uint8_t Temp8;

	if (LimitCheckId >= VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS)
	{
		return VL53L0X_InvalidParameter;
	}
	else
	{
		if (LimitCheckEnable == 0)
		{
			TempFix1616 = 0;
			LimitCheckEnableInt = 0;
			LimitCheckDisable = 1;
		}
		else
		{
			TempFix1616 = p_devData[sensor_index].CurrentParameters.LimitChecksValue[LimitCheckId];

			LimitCheckDisable = 0;
			/* this to be sure to have either 0 or 1 */
			LimitCheckEnableInt = 1;
		}

		switch (LimitCheckId) {

			case VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE:
				/* internal computation: */
				p_devData[sensor_index].CurrentParameters.LimitChecksEnable[VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE] = LimitCheckEnableInt;

				break;

			case VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE:
				status = VL53L0X_IO_Write_Word(sensor_index, VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, VL53L0X_FIXPOINT1616TOFIXPOINT97(TempFix1616));
				if(status < VL53L0X_OK)
				{
					return status;
				}

				break;

			case VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP:

				/* internal computation: */
				p_devData[sensor_index].CurrentParameters.LimitChecksEnable[VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP] = LimitCheckEnableInt;

				break;

			case VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD:

				/* internal computation: */
				p_devData[sensor_index].CurrentParameters.LimitChecksEnable[VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD] = LimitCheckEnableInt;

				break;

			case VL53L0X_CHECKENABLE_SIGNAL_RATE_MSRC:

				Temp8 = (uint8_t)(LimitCheckDisable << 1);

				status = VL53L0X_IO_Update_Byte(sensor_index, VL53L0X_REG_MSRC_CONFIG_CONTROL, 0xFE, Temp8);
				if(status < VL53L0X_OK)
				{
					return status;
				}

				break;

			case VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE:

				Temp8 = (uint8_t)(LimitCheckDisable << 4);

				status = VL53L0X_IO_Update_Byte(sensor_index, VL53L0X_REG_MSRC_CONFIG_CONTROL, 0xEF, Temp8);
				if(status < VL53L0X_OK)
				{
					return status;
				}

				break;

			default:
				return VL53L0X_InvalidParameter;
		}
	}

	if (LimitCheckEnable == 0)
	{
		p_devData[sensor_index].CurrentParameters.LimitChecksEnable[LimitCheckId] = 0;
	}
	else
	{
		p_devData[sensor_index].CurrentParameters.LimitChecksEnable[LimitCheckId] = 1;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_StaticInit(uint8_t sensor_index)
{
	VL53L0X_OpResult status;
	VL53L0X_DeviceParameters_t CurrentParameters;
	uint8_t *pTuningSettingBuffer;
	uint16_t tempword = 0;
	uint8_t tempbyte = 0;
	uint8_t UseInternalTuningSettings = 0;
	uint32_t count = 0;
	uint8_t isApertureSpads = 0;
	uint32_t refSpadCount = 0;
	uint8_t ApertureSpads = 0;
	uint8_t vcselPulsePeriodPCLK;
	uint32_t seqTimeoutMicroSecs;

	status = VL53L0X_Get_Info_From_Device(sensor_index, 1);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* set the ref spad from NVM */
	count = (uint32_t)p_devData[sensor_index].DeviceSpecificParameters.ReferenceSpadCount;
	ApertureSpads = p_devData[sensor_index].DeviceSpecificParameters.ReferenceSpadType;

	/* NVM value invalid */
	if ((ApertureSpads > 1) || ((ApertureSpads == 1) && (count > 32)) || ((ApertureSpads == 0) && (count > 12)))
	{
		status = VL53L0X_Perform_Ref_Spad_Management(sensor_index, &refSpadCount, &isApertureSpads);
		if(status < VL53L0X_OK)
		{
			return status;
		}
	}
	else
	{
		status = VL53L0X_Set_Reference_Spads(sensor_index, count, ApertureSpads);
		if(status < VL53L0X_OK)
		{
			return status;
		}
	}

	/* Initialize tuning settings buffer to prevent compiler warning. */
	pTuningSettingBuffer = DefaultTuningSettings;

	UseInternalTuningSettings = p_devData[sensor_index].UseInternalTuningSettings;

	if (UseInternalTuningSettings == 0)
		pTuningSettingBuffer = p_devData[sensor_index].pTuningSettingsPointer;
	else
		pTuningSettingBuffer = DefaultTuningSettings;

	status = VL53L0X_Load_Tuning_Settings(sensor_index, pTuningSettingBuffer);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Set interrupt config to new sample ready */
	status = VL53L0X_Set_Gpio_Config(sensor_index, 0, 0, VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY, VL53L0X_INTERRUPTPOLARITY_LOW);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Read_Word(sensor_index, 0x84, &tempword);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	p_devData[sensor_index].DeviceSpecificParameters.OscFrequencyMHz = VL53L0X_FIXPOINT412TOFIXPOINT1616(tempword);

	/* After static init, some device parameters may be changed * so update them */
	status = VL53L0X_GetDeviceParameters(sensor_index, &CurrentParameters);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Get_Fraction_Enable(sensor_index, &tempbyte);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	
	p_devData[sensor_index].RangeFractionalEnable = tempbyte;

	p_devData[sensor_index].CurrentParameters = CurrentParameters;

	/* read the sequence config and save it */
	status = VL53L0X_IO_Read_Byte(sensor_index, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, &tempbyte);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	
	p_devData[sensor_index].SequenceConfig = tempbyte;

	/* Disable MSRC and TCC by default */
	status = VL53L0X_Set_Sequence_Step_Enable(sensor_index, VL53L0X_SEQUENCESTEP_TCC, 0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Set_Sequence_Step_Enable(sensor_index, VL53L0X_SEQUENCESTEP_MSRC, 0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Set PAL State to standby */
	p_devData[sensor_index].PalState = VL53L0X_STATE_IDLE;

	/* Store pre-range vcsel period */
	status = VL53L0X_Get_Vcsel_Pulse_Period(sensor_index, VL53L0X_VCSEL_PERIOD_PRE_RANGE, &vcselPulsePeriodPCLK);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	p_devData[sensor_index].DeviceSpecificParameters.PreRangeVcselPulsePeriod = vcselPulsePeriodPCLK;

	/* Store final-range vcsel period */
	status = VL53L0X_Get_Vcsel_Pulse_Period(sensor_index, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, &vcselPulsePeriodPCLK);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	p_devData[sensor_index].DeviceSpecificParameters.FinalRangeVcselPulsePeriod = vcselPulsePeriodPCLK;

	/* Store pre-range timeout */
	status = VL53L0X_Get_Sequence_Step_Timeout(sensor_index, VL53L0X_SEQUENCESTEP_PRE_RANGE, &seqTimeoutMicroSecs);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	p_devData[sensor_index].DeviceSpecificParameters.PreRangeTimeoutMicroSecs = seqTimeoutMicroSecs;

	/* Store final-range timeout */
	status = VL53L0X_Get_Sequence_Step_Timeout(sensor_index, VL53L0X_SEQUENCESTEP_FINAL_RANGE, &seqTimeoutMicroSecs);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	p_devData[sensor_index].DeviceSpecificParameters.FinalRangeTimeoutMicroSecs = seqTimeoutMicroSecs;

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_StopMeasurement(uint8_t sensor_index)
{
	VL53L0X_OpResult status;

	status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0x00, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0x91, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0x00, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x00);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	/* Set PAL State to Idle */
	p_devData[sensor_index].PalState = VL53L0X_STATE_IDLE;

	/* Check if need to apply interrupt settings */
	status = VL53L0X_Check_And_Load_Interrupt_Settings(sensor_index, 0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_GetStopCompletedstatus(uint8_t sensor_index, uint32_t *pStopstatus)
{
	VL53L0X_OpResult status;
	uint8_t Byte = 0;

	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x01);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Read_Byte(sensor_index, 0x04, &Byte);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x0);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	*pStopstatus = Byte;

	if (Byte == 0)
	{
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x80, 0x01);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x01);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x00, 0x00);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x91, p_devData[sensor_index].StopVariable);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x00, 0x01);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0xFF, 0x00);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_IO_Write_Byte(sensor_index, 0x80, 0x00);
		if(status < VL53L0X_OK)
		{
			return status;
		}
	}

	return VL53L0X_OK;
}

VL53L0X_OpResult VL53L0X_GetSensorId(uint8_t sensor_index, uint16_t* rl_id)
{
    VL53L0X_OpResult status = 0;

    status = VL53L0X_IO_Read_Word(sensor_index, VL53L0X_REG_IDENTIFICATION_MODEL_ID, rl_id);
	if(status < VL53L0X_OK)
	{
		return status;
	}

    return VL53L0X_OK;
}

VL53L0X_OpResult VL53L0X_WaitMeasurementDataReady(uint8_t sensor_index)
{
    VL53L0X_OpResult status;
    uint8_t NewDatReady=0;
    uint32_t LoopNb;

    // Wait until it finished
    // use timeout to avoid deadlock
	LoopNb = 0;
	do {
		status = VL53L0X_Get_Measurement_Data_Ready(sensor_index, &NewDatReady);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		if ((NewDatReady == 0x01))
		{
			break;
		}

		LoopNb = LoopNb + 1;

		NeonRTOS_Sleep(2);
		
		if (LoopNb >= VL53L0X_DEFAULT_MAX_LOOP) {
			return VL53L0X_SlaveTimeout;
		}
	} while (1);

    return VL53L0X_OK;
}

VL53L0X_OpResult VL53L0X_WaitStopCompleted(uint8_t sensor_index)
{
    VL53L0X_OpResult status;
    uint32_t StopCompleted=0;
    uint32_t LoopNb;

    // Wait until it finished
    // use timeout to avoid deadlock
	LoopNb = 0;
	do {
		status = VL53L0X_GetStopCompletedstatus(sensor_index, &StopCompleted);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		if ((StopCompleted == 0x00))
		{
			break;
		}

		LoopNb = LoopNb + 1;
		NeonRTOS_Sleep(2);

		if (LoopNb >= VL53L0X_DEFAULT_MAX_LOOP) {
			return VL53L0X_SlaveTimeout;
		}

	} while (1);

    return VL53L0X_OK;
}

VL53L0X_OpResult VL53L0X_SensorInit(uint8_t sensor_index)
{
    VL53L0X_OpResult status;

	status = VL53L0X_DataInit(sensor_index);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	uint32_t refSpadCount;
	uint8_t isApertureSpads;
	uint8_t VhvSettings;
	uint8_t PhaseCal;

	status = VL53L0X_StaticInit(sensor_index); // Device Initialization
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_Perform_Ref_Calibration(sensor_index, &VhvSettings, &PhaseCal, 1); // Device Initialization
	if(status < VL53L0X_OK)
	{
		return status;
	}
    
	status = VL53L0X_Perform_Ref_Spad_Management(sensor_index, &refSpadCount, &isApertureSpads); // Device Initialization
	if(status < VL53L0X_OK)
	{
		return status;
	}
        
   return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_StartMeasurementSimplified(uint8_t sensor_index, OperatingMode operating_mode)
{
    VL53L0X_OpResult status;
    uint8_t VhvSettings;
    uint8_t PhaseCal;
    // *** from mass market cube expansion v1.1, ranging with satellites.
    // default settings, for normal range.
	FixPoint1616_t signalLimit = (FixPoint1616_t)(0.25*65536);
	FixPoint1616_t sigmaLimit = (FixPoint1616_t)(18*65536);
	uint32_t timingBudget = 33000;
	uint8_t preRangeVcselPeriod = 14;
	uint8_t finalRangeVcselPeriod = 10;

    if (operating_mode == range_single_shot_polling)
    {
        // singelshot, polled ranging
            // no need to do this when we use VL53L0X_Perform_Single_Ranging_Measurement
		status = VL53L0X_Set_Device_Mode(sensor_index, VL53L0X_DEVICEMODE_SINGLE_RANGING); // Setup in single ranging mode
		if(status < VL53L0X_OK)
		{
			return status;
		}

        // Enable/Disable Sigma and Signal check
		status = VL53L0X_Set_Limit_Check_Enable(sensor_index, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
		if(status < VL53L0X_OK)
		{
			return status;
		}
		status = VL53L0X_Set_Limit_Check_Enable(sensor_index, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
		if(status < VL53L0X_OK)
		{
			return status;
		}

// *** from mass market cube expansion v1.1, ranging with satellites.
		/* Ranging configuration */
//*
//        switch(rangingConfig) {
//        case LONG_RANGE:
        	signalLimit = (FixPoint1616_t)(0.1*65536);
        	sigmaLimit = (FixPoint1616_t)(60*65536);
        	timingBudget = 33000;
        	preRangeVcselPeriod = 18;
        	finalRangeVcselPeriod = 14;
/*        	break;
        case HIGH_ACCURACY:
			signalLimit = (FixPoint1616_t)(0.25*65536);
			sigmaLimit = (FixPoint1616_t)(18*65536);
			timingBudget = 200000;
			preRangeVcselPeriod = 14;
			finalRangeVcselPeriod = 10;
			break;
        case HIGH_SPEED:
			signalLimit = (FixPoint1616_t)(0.25*65536);
			sigmaLimit = (FixPoint1616_t)(32*65536);
			timingBudget = 20000;
			preRangeVcselPeriod = 14;
			finalRangeVcselPeriod = 10;
 			break;
        default:
        	debug_printf("Not Supported");
        }
*/

        status = VL53L0X_SetLimitCheckValue(sensor_index, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, signalLimit);
		if(status < VL53L0X_OK)
		{
			return status;
		}

        status = VL53L0X_SetLimitCheckValue(sensor_index, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, sigmaLimit);
		if(status < VL53L0X_OK)
		{
			return status;
		}

        status = VL53L0X_Set_Measurement_Timing_Budget_Micro_Seconds(sensor_index, timingBudget);
		if(status < VL53L0X_OK)
		{
			return status;
		}

        status = VL53L0X_Set_Vcsel_Pulse_Period(sensor_index, VL53L0X_VCSEL_PERIOD_PRE_RANGE, preRangeVcselPeriod);
		if(status < VL53L0X_OK)
		{
			return status;
		}

        status = VL53L0X_Set_Vcsel_Pulse_Period(sensor_index, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, finalRangeVcselPeriod);
		if(status < VL53L0X_OK)
		{
			return status;
		}

        status = VL53L0X_Perform_Ref_Calibration(sensor_index, &VhvSettings, &PhaseCal, 1);
		if(status < VL53L0X_OK)
		{
			return status;
		}
    }

    if (operating_mode == range_continuous_polling)
    {
        status = VL53L0X_Set_Device_Mode(sensor_index, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING); // Setup in continuous ranging mode
		if(status < VL53L0X_OK)
		{
			return status;
		}

        status = VL53L0X_Start_Measurement(sensor_index);
		if(status < VL53L0X_OK)
		{
			return status;
		}
    }

    return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_GetMeasurementSimplified(uint8_t sensor_index, OperatingMode operating_mode, VL53L0X_RangingMeasurementData_t *Data)
{
    VL53L0X_OpResult status;

    if (operating_mode == range_single_shot_polling)
    {
        status = VL53L0X_Perform_Single_Ranging_Measurement(sensor_index, Data);
		if(status < VL53L0X_OK)
		{
			return status;
		}
    }

    if (operating_mode == range_continuous_polling)
    {
   	    status = VL53L0X_Measurement_Poll_For_Completion(sensor_index);
		if(status < VL53L0X_OK)
		{
			return status;
		}

        status = VL53L0X_Get_Ranging_Measurement_Data(sensor_index, Data);
		if(status < VL53L0X_OK)
		{
			return status;
		}

        // Clear the interrupt
        VL53L0X_Clear_Interrupt_Mask(sensor_index, VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY);
        NeonRTOS_Sleep(2);
    }

    return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_StopMeasurementSimplified(uint8_t sensor_index, OperatingMode operating_mode)
{
    VL53L0X_OpResult status;

	// don't need to stop for a singleshot range!
    if (operating_mode==range_single_shot_polling)
    {
    }

    if (operating_mode==range_continuous_interrupt || operating_mode==range_continuous_polling)
    {
    // continuous mode
		status = VL53L0X_StopMeasurement(sensor_index);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		status = VL53L0X_WaitStopCompleted(sensor_index);
		if(status < VL53L0X_OK)
		{
			return status;
		}

		status = VL53L0X_Clear_Interrupt_Mask(sensor_index, VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY);
		if(status < VL53L0X_OK)
		{
			return status;
		}
    }

    return VL53L0X_OK;
}

VL53L0X_OpResult VL53L0X_GetDistance(uint8_t sensor_index, uint32_t *piData)
{
	VL53L0X_OpResult status;
	VL53L0X_RangingMeasurementData_t pRangingMeasurementData;

	status = VL53L0X_StartMeasurementSimplified(sensor_index, range_single_shot_polling);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	status = VL53L0X_GetMeasurementSimplified(sensor_index, range_single_shot_polling, &pRangingMeasurementData);
	if(status < VL53L0X_OK)
	{
		return status;
	}
	
	if (pRangingMeasurementData.RangeStatus == 0)
	{
		// we have a valid range.
		*piData = pRangingMeasurementData.RangeMilliMeter;
	}
	else {
		*piData = 0;
		return VL53L0X_Range_Error;
	}

	status = VL53L0X_StopMeasurementSimplified(sensor_index, range_single_shot_polling);
	if(status < VL53L0X_OK)
	{
		return status;
	}

	return VL53L0X_OK;
}