/*
 * DirectionTask.cpp
 *
 *  Created on: May 16, 2025
 *      Author: To
 */

#include "DirectionTask.h"

DirectionTask DirectionHandler;

DirectionTask::DirectionTask()
{
	// TODO Auto-generated constructor stub
	mMotorAV = new StepperMotor(PULSE_DIR_AV_GPIO_Port, PULSE_DIR_AV_Pin, DIR_DIR_AV_GPIO_Port, DIR_DIR_AV_Pin);
	mMotorAR = new StepperMotor(PULSE_DIR_AR_GPIO_Port, PULSE_DIR_AR_Pin, DIR_DIR_AR_GPIO_Port, DIR_DIR_AR_Pin);

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
	// TODO Auto-generated destructor stub
	HAL_GPIO_WritePin(NSS_AV_GPIO_Port, NSS_AV_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(NSS_AR_GPIO_Port, NSS_AR_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(EN_DIR_AV_GPIO_Port, EN_DIR_AV_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(EN_DIR_AR_GPIO_Port, EN_DIR_AR_Pin, GPIO_PIN_SET);
	osDelay(200);
	HAL_TIM_Base_Start_IT(&htim16);
	HAL_TIM_OC_Start(&htim16, TIM_CHANNEL_1);
}

void DirectionTask::run()
{

	/*HAL_GPIO_WritePin(NSS_AV_GPIO_Port, NSS_AV_Pin, GPIO_PIN_RESET);
	HAL_SPI_Receive(&hspi5, &mDirSensorAV, 1, 100);
	HAL_GPIO_WritePin(NSS_AV_GPIO_Port, NSS_AV_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(NSS_AR_GPIO_Port, NSS_AV_Pin, GPIO_PIN_RESET);
	HAL_SPI_Receive(&hspi5, &mDirSensorAR, 1, 100);
	HAL_GPIO_WritePin(NSS_AR_GPIO_Port, NSS_AV_Pin, GPIO_PIN_SET);
*/
	osDelay(20);
	// TODO Auto-generated destructor stub
}

void DirectionTask::cleanup()
{
	// TODO Auto-generated destructor stub
}
