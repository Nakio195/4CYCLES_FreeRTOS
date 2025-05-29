/*
 * VehicleTask.cpp
 *
 *  Created on: Feb 3, 2025
 *      Author: To
 */

#include "VehicleTask.h"

VehicleTask Vehicle;

VehicleTask::VehicleTask()
{

	ThresholdFilter* threshold = new ThresholdFilter();

	for (int i = 10; i <= 260; i += 10)
	{
		if(i == 10)
			threshold->addThreshold(0, 10, 0);
		else
			threshold->addThreshold(i-9, i, i);
		if (i >= 255)
			threshold->addThreshold(250, 255, 255);
	}

	mThrottle.addFilter(threshold);
	mThrottle.addFilter(new SCurveFilter(5));
	mThrottle.addFilter(new LowPassFilter(10));

	mBrake.addFilter(new LowPassFilter(10));


}

void VehicleTask::setup()
{

	this->attachLogQueue(Json.createLogQueue());
	CanHandler.attachLogQueue(Json.createLogQueue());
	MainController.attachLogQueue(Json.createLogQueue());
	mControllerQueue = MainController.getQueue();
	vQueueAddToRegistry(mControllerQueue, "ControllerActions");

	CanHandler.start("CAN", 256, osPriorityHigh);
	MainController.start("PS3", 256, osPriorityAboveNormal);
	DirectionHandler.start("Direction", 128, osPriorityHigh);
	//Json.start("JSON Logger", 1024, osPriorityBelowNormal);
}

void VehicleTask::run()
{
	Action* action = nullptr;

	// Read received action from controller
	while(uxQueueMessagesWaiting(mControllerQueue))
	{
		if(xQueueReceive(mControllerQueue, &action, 0) == pdTRUE)
		{
			if (action != nullptr)
			{
				if(action->type() == Action::Throttle)
				{
					mThrottle.setInput(action->getThrottleValue());
				}
				else if(action->type() == Action::Brake)
					mBrake.setInput(action->getBrakeValue());
				else if(action->type() == Action::Type::Lights)
					handleLightsAction(action);
				else if(action->type() == Action::Steering)
				{
					DirectionHandler.setDirectionAV(action->getSteeringValue());
					DirectionHandler.setDirectionAR(-action->getSteeringValue());
				}

				delete action;
			}
		}
	}

	//	Computing data
	mThrottle.update();
	mBrake.update();


	osDelay(10);
}

void VehicleTask::cleanup()
{

}

void VehicleTask::handleLightsAction(Action* action)
{
	switch(action->getLightsType())
	{
		case Action::Signals::Left:
			if(action->getLightState() == 0)
			{
				uint8_t Buffer[] = "TL off\n";
				HAL_UART_Transmit(&huart5, Buffer, 7, 5);
			}
			else
			{
				uint8_t Buffer[] = "TL on\n";
				HAL_UART_Transmit(&huart5, Buffer, 6, 5);
			}
			break;

		case Action::Signals::Right:
			if(action->getLightState() == 0)
			{
				uint8_t Buffer[] = "TR off\n";
				HAL_UART_Transmit(&huart5, Buffer, 7, 5);
			}
			else
			{
				uint8_t Buffer[] = "TR on\n";
				HAL_UART_Transmit(&huart5, Buffer, 6, 5);
			}
			break;

		case Action::Signals::BrakeSignal:
			if(action->getLightState() == 0)
			{
				uint8_t Buffer[] = "BR off\n";
				HAL_UART_Transmit(&huart5, Buffer, 7, 5);
			}
			else
			{
				uint8_t Buffer[] = "BR on\n";
				HAL_UART_Transmit(&huart5, Buffer, 6, 5);
			}
			break;

		case Action::Signals::Hazard:
			if(action->getLightState() == 0)
			{
				uint8_t Buffer[] = "HA off\n";
				HAL_UART_Transmit(&huart5, Buffer, 7, 5);
			}
			else
			{
				uint8_t Buffer[] = "HA on\n";
				HAL_UART_Transmit(&huart5, Buffer, 6, 5);
			}
			break;

		case Action::Signals::NighLight:
			if(action->getLightState() == 0)
			{
				uint8_t Buffer[] = "NL off\n";
				HAL_UART_Transmit(&huart5, Buffer, 7, 5);
			}
			else
			{
				uint8_t Buffer[] = "NL on\n";
				HAL_UART_Transmit(&huart5, Buffer, 6, 5);
			}
			break;
	}
}

