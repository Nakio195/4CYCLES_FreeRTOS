/*
 * DirectionTask.cpp
 *
 *  Created on: May 16, 2025
 *      Author: To
 */

#include "DirectionTask.h"

DirectionTask DirectionHandler;

DirectionTask::DirectionTask() : mSensorCenterAR(2841), mSensorCenterAV(1971)
{
	// TODO Auto-generated constructor stub
	mMotorAV = new StepperMotor(true, 2, -5000, 5000, PULSE_DIR_AV_GPIO_Port, PULSE_DIR_AV_Pin, DIR_DIR_AV_GPIO_Port, DIR_DIR_AV_Pin);
	mMotorAR = new StepperMotor(false, 2, -5000, 5000, PULSE_DIR_AR_GPIO_Port, PULSE_DIR_AR_Pin, DIR_DIR_AR_GPIO_Port, DIR_DIR_AR_Pin);

	mBrakeAV = new StepperMotor(true, 2, -5000, 5000, PULSE_BRK_AV_GPIO_Port, PULSE_BRK_AV_Pin, DIR_BRK_AV_GPIO_Port, DIR_BRK_AV_Pin);
	mBrakeAR = new StepperMotor(true, 2, -5000, 5000, PULSE_BRK_AR_GPIO_Port, PULSE_BRK_AR_Pin, DIR_BRK_AR_GPIO_Port, DIR_BRK_AR_Pin);
}


void DirectionTask::setDirectionAV(int32_t target)
{
	mMotorAV->setTargetPosition(target);
}

void DirectionTask::setDirectionAR(int32_t target)
{
	mMotorAR->setTargetPosition(target);
}

void DirectionTask::setBrakeAV(int32_t target)
{
	mBrakeAV->setTargetPosition(target);
}

void DirectionTask::setBrakeAR(int32_t target)
{
	mBrakeAR->setTargetPosition(target);
}

void DirectionTask::setup()
{

	HAL_GPIO_WritePin(NSS_AV_GPIO_Port, NSS_AV_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(NSS_AR_GPIO_Port, NSS_AR_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(EN_DIR_AV_GPIO_Port, EN_DIR_AV_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(EN_DIR_AR_GPIO_Port, EN_DIR_AR_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(EN_BRK_AV_GPIO_Port, EN_BRK_AV_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(EN_BRK_AR_GPIO_Port, EN_BRK_AR_Pin, GPIO_PIN_SET);
	osDelay(200);
	HAL_TIM_Base_Start_IT(&htim16);
	HAL_TIM_OC_Start(&htim16, TIM_CHANNEL_1);
}

void DirectionTask::run()
{
	//AV : Max Right : 3068 - Max Left 14684 - Mid 8184 -- Zero 8876
	//AR : Max Right : 15528 - Max Left 4152 - Mid 42844 -- Zero 9944

    bool Checksum_Error = true;

    // Read AV Sensor
	uint16_t Data = 0;//readAMT232(&hspi6, NSS_AV_GPIO_Port, NSS_AV_Pin, &Checksum_Error);

	if(!Checksum_Error)
	{
		mDirSensorAV = int16_t(Data);
		mMotorAV->setRealPosition(mDirSensorAV);
		mDirSensorErrorAV = 0;
	}

	else
	{
		mDirSensorErrorAV++;
		mDirSensorMaxErrorAV = mDirSensorErrorAV > mDirSensorMaxErrorAV ? mDirSensorErrorAV : mDirSensorMaxErrorAV;

		if (mDirSensorErrorAV > 20)
		{
			// TODO : 20 consecutive read error, system might be damaged
			asm("NOP");
		}
	}

	// Read AR Sensor
	Data = 0;//readAMT232(&hspi5, NSS_AR_GPIO_Port, NSS_AR_Pin, &Checksum_Error);

	if(!Checksum_Error)
	{
		mDirSensorAR = int16_t(Data);
		mMotorAR->setRealPosition(mDirSensorAR);
		mDirSensorErrorAR = 0;
	}

	else
	{
		mDirSensorErrorAR++;
		if (mDirSensorErrorAR > 10)
		{
			// TODO : 10 consecutive read error, system might be damaged
		}
	}

	osDelay(10);

//	rxData = 0;
//
//	HAL_GPIO_WritePin(NSS_AR_GPIO_Port, NSS_AR_Pin, GPIO_PIN_RESET);
//	HAL_SPI_TransmitReceive(&hspi5, (uint8_t*)&txDummy, (uint8_t*)&rxData, 1, 100);
//	HAL_GPIO_WritePin(NSS_AR_GPIO_Port, NSS_AR_Pin, GPIO_PIN_SET);
//
//	rxData &= 0x3FFF;
//	rxData -= mSensorCenterAR;
//	mDirSensorAR = int16_t(rxData);
//	mMotorAR->setRealPosition(mDirSensorAR);
}


uint16_t DirectionTask::readAMT232(SPI_HandleTypeDef* hspi, GPIO_TypeDef* port, uint16_t pin, bool* error)
{
    int16_t rx = 0;
	int16_t Data = 0;
    bool Checksum_K1 = 0;
    bool Checksum_K2 = 0;
    uint16_t txDummy = 0x0000;

    *error = false;

	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(hspi, (uint8_t*)&txDummy, (uint8_t*)&rx, 1, 100);
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);

	Data = (rx & 0x3FFF) >> 2;
	Checksum_K1 = bool(rx & 0x8000);
	Checksum_K2 = bool(rx & 0x4000);

	if (!checksumAMT232(rx, Checksum_K1, Checksum_K2))
		*error = true;

	return Data;

}

bool DirectionTask::checksumAMT232(uint16_t data, bool k1, bool k0)
{
    bool h5 = (data >> 13) & 1;
    bool h3 = (data >> 11) & 1;
    bool h1 = (data >> 9) & 1;
    bool l7 = (data >> 7) & 1;
    bool l5 = (data >> 5) & 1;
    bool l3 = (data >> 3) & 1;
    bool l1 = (data >> 1) & 1;
    bool h4 = (data >> 12) & 1;
    bool h2 = (data >> 10) & 1;
    bool h0 = (data >> 8) & 1;
    bool l6 = (data >> 6) & 1;
    bool l4 = (data >> 4) & 1;
    bool l2 = (data >> 2) & 1;
    bool l0 = (data >> 0) & 1;

    bool isValid = false;

    k1 == !(h5 ^ h3 ^ h1 ^ l7 ^ l5 ^ l3 ^ l1) ? isValid = true : isValid = false;
    k0 == !(h4 ^ h2 ^ h0 ^ l6 ^ l4 ^ l2 ^ l0) ? isValid = true : isValid = false;

    return isValid;
}

int32_t DirectionTask::mapSensorToStepper(uint16_t d)
{
	return 0;
}

void DirectionTask::cleanup()
{
	// TODO Auto-generated destructor stub
}
