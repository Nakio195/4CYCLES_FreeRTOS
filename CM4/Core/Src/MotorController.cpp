/*
 * MotorController.cpp
 *
 *  Created on: Jan 16, 2025
 *      Author: To
 */

#include "MotorController.h"
#include "gpio.h"

MotorController::MotorController()
{
	// TODO Auto-generated constructor stub
	ModbusHandler = new ModbusDriver();

	Motor = new Phaserunner(1);
}

MotorController::~MotorController()
{
	// TODO Auto-generated destructor stub
}

void MotorController::toggleRed()
{
}

void MotorController::tick()
{
	HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
}

void MotorController::process()
{
	while(1)
	{
		ModbusHandler->process();
		Motor->process();
	}
}
