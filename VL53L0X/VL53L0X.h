/*******************************************************************************
 Copyright Ã‚Â© 2016, STMicroelectronics International N.V.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of STMicroelectronics nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS ARE DISCLAIMED.
 IN NO EVENT SHALL STMICROELECTRONICS INTERNATIONAL N.V. BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef __VL53L0X_CLASS_H
#define __VL53L0X_CLASS_H


#ifdef _MSC_VER
#   ifdef VL53L0X_API_EXPORTS
#       define VL53L0X_API  __declspec(dllexport)
#   else
#       define VL53L0X_API
#   endif
#else
#   define VL53L0X_API
#endif


/* Includes ------------------------------------------------------------------*/
#include "Arduino.h"
#include "RangeSensor.h"
#include "Wire.h"

#include "vl53l0x_def.h"
#include "vl53l0x_platform.h"


/**
 * The device model ID
 */
//#define IDENTIFICATION_MODEL_ID                 0x000


//#define STATUS_OK              0x00
//#define STATUS_FAIL            0x01


/** default device address */
#define VL53L0x_DEFAULT_DEVICE_ADDRESS		0x52 /* (8-bit) */

/* Classes -------------------------------------------------------------------*/
/** Class representing a VL53L0 sensor component
 */
class VL53L0X : public RangeSensor
{
 public:
    /** Constructor
     * @param[in] i2c device I2C to be used for communication
     * @param[in] pin shutdown pin to be used as component GPIO0
     */
    VL53L0X(TwoWire *i2c, int pin) : RangeSensor(), dev_i2c(i2c), gpio0(pin)
    {
       Device=&MyDevice;
       memset((void *)Device, 0x0, sizeof(VL53L0X_Dev_t));
       MyDevice.I2cDevAddr=VL53L0x_DEFAULT_DEVICE_ADDRESS;
       MyDevice.comms_type=1; // VL53L0X_COMMS_I2C
       MyDevice.comms_speed_khz=400;
    }
    
   /** Destructor
    */
    virtual ~VL53L0X(){}
    /* warning: VL53L0X class inherits from GenericSensor, RangeSensor and LightSensor, that haven`t a destructor.
       The warning should request to introduce a virtual destructor to make sure to delete the object */

    virtual int begin()
    {
       if(gpio0 >= 0)
       {
          pinMode(gpio0, OUTPUT);
       }
       return 0;
    }

    virtual int end()
    {
       if(gpio0 >= 0)
       {
          pinMode(gpio0, INPUT);
       }
       return 0;
    }

	/*** Interface Methods ***/
	/*** High level API ***/
	/**
	 * @brief       PowerOn the sensor
	 * @return      void
	 */
    /* turns on the sensor */
    virtual void VL53L0X_On(void)
    {
       if(gpio0 >= 0)
       {
         digitalWrite(gpio0, HIGH);
       }
       delay(10);
    }

	/**
	 * @brief       PowerOff the sensor
	 * @return      void
	 */
    /* turns off the sensor */
    virtual void VL53L0X_Off(void)
    {
       if(gpio0 >= 0)
       {
         digitalWrite(gpio0, LOW);
       }
       delay(10);
    }

	/**
	 * @brief       Initialize the sensor with default values
	 * @return      0 on Success
	 */
    int InitSensor(uint8_t NewAddr);

	/**
	 * @brief       Start the measure indicated by operating mode
	 * @param[in]   operating_mode specifies requested measure
	 * @param[in]   fptr specifies call back function must be !NULL in case of interrupt measure
	 * @return      0 on Success
	 */
    int StartMeasurementSimplified(OperatingMode operating_mode, void (*fptr)(void));

	/**
	 * @brief       Get results for the measure indicated by operating mode
	 * @param[in]   operating_mode specifies requested measure results
	 * @param[out]  Data pointer to the MeasureData_t structure to read data in to
	 * @return      0 on Success
	 */
    int GetMeasurementSimplified(OperatingMode operating_mode, VL53L0X_RangingMeasurementData_t *Data);

	/**
	 * @brief       Stop the currently running measure indicate by operating_mode
	 * @param[in]   operating_mode specifies requested measure to stop
	 * @return      0 on Success
	 */
    int StopMeasurementSimplified(OperatingMode operating_mode);

    /** Wrapper functions */
/** @defgroup api_init Init functions
 *  @brief    API init functions
 *  @ingroup api_hl
 *  @{
 */
/**
 * @brief Wait for device booted after chip enable (hardware standby)
 * @par Function Description
 * After Chip enable Application you can also simply wait at least 1ms to ensure device is ready
 * @warning After device chip enable (gpio0) de-asserted  user must wait gpio1 to get asserted (hardware standby).
 * or wait at least 400usec prior to do any low level access or api call .
 *
 * This function implements polling for standby but you must ensure 400usec from chip enable passed\n
 * @warning if device get prepared @a VL53L0X_Prepare() re-using these function can hold indefinitely\n
 *
 * @param 		void
 * @return     0 on success
 */
    int WaitDeviceBooted()
    {
       return VL53L0X_WaitDeviceBooted(Device);
    }

/**
 *
 * @brief One time device initialization
 *
 * To be called once and only once after device is brought out of reset (Chip enable) and booted see @a VL6180x_WaitDeviceBooted()
 *
 * @par Function Description
 * When not used after a fresh device "power up" or reset, it may return @a #CALIBRATION_WARNING
 * meaning wrong calibration data may have been fetched from device that can result in ranging offset error\n
 * If application cannot execute device reset or need to run VL6180x_InitData  multiple time
 * then it  must ensure proper offset calibration saving and restore on its own
 * by using @a VL6180x_GetOffsetCalibrationData() on first power up and then @a VL6180x_SetOffsetCalibrationData() all all subsequent init
 *
 * @param void
 * @return     0 on success,  @a #CALIBRATION_WARNING if failed
 */
    virtual int Init()
    {
       return VL53L0X_DataInit(Device);
    }

/**
  * @brief  Prepare device for operation
  * @par Function Description
  * Does static initialization and reprogram common default settings \n
  * Device is prepared for new measure, ready single shot ranging or ALS typical polling operation\n
  * After prepare user can : \n
  * @li Call other API function to set other settings\n
  * @li Configure the interrupt pins, etc... \n
  * @li Then start ranging or ALS operations in single shot or continuous mode
  *
  * @param void
  * @return      0 on success
  */
    int Prepare()
    {
        // taken from rangingTest() in vl53l0x_SingleRanging_Example.c
		VL53L0X_Error Status = VL53L0X_ERROR_NONE;
        uint32_t refSpadCount;
        uint8_t isApertureSpads;
        uint8_t VhvSettings;
        uint8_t PhaseCal;

        if(Status == VL53L0X_ERROR_NONE)
        {
            Status = VL53L0X_StaticInit(Device); // Device Initialization
        }

        if(Status == VL53L0X_ERROR_NONE)
        {
           Status = VL53L0X_PerformRefCalibration(Device, &VhvSettings, &PhaseCal); // Device Initialization
        }

        if(Status == VL53L0X_ERROR_NONE)
        {
            Status = VL53L0X_PerformRefSpadManagement(Device, &refSpadCount, &isApertureSpads); // Device Initialization
        }

        return Status;
    }

/**
 * @brief Get ranging result and only that
 *
 * @par Function Description
 * Unlike @a VL6180x_RangeGetMeasurement() this function only retrieves the range in millimeter \n
 * It does any required up-scale translation\n
 * It can be called after success status polling or in interrupt mode \n
 * @warning these function is not doing wrap around filtering \n
 * This function doesn't perform any data ready check!
 *
 * @param pRange_mm  Pointer to range distance
 * @return           0 on success
 */
    virtual int GetDistance(uint32_t *piData)
    {
        int status=0;
        VL53L0X_RangingMeasurementData_t pRangingMeasurementData;

        status=StartMeasurementSimplified(range_single_shot_polling, NULL);
        if (!status) {
            status=GetMeasurementSimplified(range_single_shot_polling, &pRangingMeasurementData);
        }
        if (pRangingMeasurementData.RangeStatus == 0) {
        // we have a valid range.
            *piData = pRangingMeasurementData.RangeMilliMeter;
        }
        else {
            *piData = 0;
            status = VL53L0X_ERROR_RANGE_ERROR;
        }
        StopMeasurementSimplified(range_single_shot_polling);
        return status;
    }

/**
 * @brief Low level ranging and ALS register static settings (you should call @a VL6180x_Prepare() function instead)
 *
 * @return 0 on success
 */
    int StaticInit()
    {
	  return VL53L0X_StaticInit(Device);
    }

/**
 * @brief Set new device i2c address
 *
 * After completion the device will answer to the new address programmed.
 *
 * @sa AN4478: Using multiple VL6180X's in a single design
 * @param NewAddr   The new i2c address (7bit)
 * @return          0 on success
 */
    int SetDeviceAddress(int NewAddr)
    {
       int status;

       status=VL53L0X_SetDeviceAddress(Device, NewAddr);
       if(!status)
          Device->I2cDevAddr=NewAddr;
       return status;
    }

	int PerformRefCalibration(uint8_t *pVhvSettings, uint8_t *pPhaseCal)
	{
		return VL53L0X_PerformRefCalibration(Device, pVhvSettings, pPhaseCal);
	}

	int PerformRefSpadManagement(uint32_t *refSpadCount, uint8_t *isApertureSpads)
	{
		return VL53L0X_PerformRefSpadManagement(Device, refSpadCount, isApertureSpads);
	}

	int SetDeviceMode(VL53L0X_DeviceModes DeviceMode)
	{
		return VL53L0X_SetDeviceMode(Device, DeviceMode);
	}

	int SetMeasurementTimingBudgetMicroSeconds(uint32_t MeasurementTimingBudgetMicroSeconds)
	{
		return VL53L0X_SetMeasurementTimingBudgetMicroSeconds(Device, MeasurementTimingBudgetMicroSeconds);
	}

	int StartMeasurement()
	{
		return VL53L0X_StartMeasurement(Device);
	}

	int StopMeasurement()
	{
		return VL53L0X_StopMeasurement(Device);
	}

	int GetMeasurementDataReady(uint8_t *pMeasurementDataReady)
	{
		return VL53L0X_GetMeasurementDataReady(Device, pMeasurementDataReady);
	}

	int GetRangingMeasurementData(VL53L0X_RangingMeasurementData_t *pRangingMeasurementData)
	{
		return VL53L0X_GetRangingMeasurementData(Device, pRangingMeasurementData);
	}

	int ClearInterruptMask(uint32_t InterruptMask)
	{
		return VL53L0X_ClearInterruptMask(Device, InterruptMask);
	}



#endif /* _VL53L0X_CLASS_H_ */
