/*
 * DirectionTask.cpp
 *
 *  Created on: May 16, 2025
 *      Author: To
 */

#include "DirectionTask.h"

DirectionTask DirectionHandler;

DirectionTask::DirectionTask() : mSensorCenterAR(9312), mSensorCenterAV(8876)
{
	// TODO Auto-generated constructor stub

}


void DirectionTask::setDirectionAV(int32_t target)
{
	mMotorAV->setTargetPosition(target);
}

void DirectionTask::setDirectionAR(int32_t target)
{
	mMotorAR->setTargetPosition(target);
}

void DirectionTask::setup()
{
	mMotorAV = new StepperMotor(true, PULSE_DIR_AV_GPIO_Port, PULSE_DIR_AV_Pin, DIR_DIR_AV_GPIO_Port, DIR_DIR_AV_Pin);
	mMotorAR = new StepperMotor(false, PULSE_DIR_AR_GPIO_Port, PULSE_DIR_AR_Pin, DIR_DIR_AR_GPIO_Port, DIR_DIR_AR_Pin);

	HAL_GPIO_WritePin(NSS_AV_GPIO_Port, NSS_AV_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(NSS_AR_GPIO_Port, NSS_AR_Pin, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(EN_DIR_AV_GPIO_Port, EN_DIR_AV_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN_DIR_AR_GPIO_Port, EN_DIR_AR_Pin, GPIO_PIN_RESET);
	osDelay(200);
	HAL_TIM_Base_Start_IT(&htim16);
	HAL_TIM_OC_Start(&htim16, TIM_CHANNEL_1);
}

void DirectionTask::run()
{
	//AV : Max Right : 3068 - Max Left 14684 - Mid 8184 -- Zero 8876
	//AR : Max Right : 15528 - Max Left 4152 - Mid 42844 -- Zero 9944
    int16_t rxData = 0;
    uint16_t txDummy = 0x0000;

	HAL_GPIO_WritePin(NSS_AV_GPIO_Port, NSS_AV_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi5, (uint8_t*)&txDummy, (uint8_t*)&rxData, 1, 100);
	HAL_GPIO_WritePin(NSS_AV_GPIO_Port, NSS_AV_Pin, GPIO_PIN_SET);

	rxData &= 0x3FFF;
	rxData -= mSensorCenterAV;
	mDirSensorAV = int16_t(rxData);
	mMotorAV->setRealPosition(mDirSensorAV);

	osDelay(10);

	rxData = 0;

	HAL_GPIO_WritePin(NSS_AR_GPIO_Port, NSS_AR_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi5, (uint8_t*)&txDummy, (uint8_t*)&rxData, 1, 100);
	HAL_GPIO_WritePin(NSS_AR_GPIO_Port, NSS_AR_Pin, GPIO_PIN_SET);

	rxData &= 0x3FFF;
	rxData -= mSensorCenterAR;
	mDirSensorAR = int16_t(rxData);
	mMotorAR->setRealPosition(mDirSensorAR);
	osDelay(10);
}

int32_t DirectionTask::mapSensorToStepper(uint16_t d)
{
	return 0;
}

void DirectionTask::cleanup()
{
	// TODO Auto-generated destructor stub
}
