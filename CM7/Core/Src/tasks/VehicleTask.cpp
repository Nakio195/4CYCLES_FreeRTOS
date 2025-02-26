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

	LowPassFilter* lowPass = new LowPassFilter(0.5);
	SCurveFilter* sCurve = new SCurveFilter(0, 0, 10.0f);
	mThrottle.addFilter(lowPass);
	mThrottle.addFilter(sCurve);
}

void VehicleTask::setup()
{

	this->attachLogQueue(Json.createLogQueue());
	CanHandler.attachLogQueue(Json.createLogQueue());
	MainController.attachLogQueue(Json.createLogQueue());
	mControllerQueue = MainController.getQueue();

	CanHandler.start("CAN", 256, osPriorityHigh);
	MainController.start("PS3", 128, osPriorityAboveNormal);
	Json.start("JSON Logger", 256, osPriorityBelowNormal);
}

void VehicleTask::run()
{
	Action* action = nullptr;
	if(xQueueReceive(mControllerQueue, &action, 0) == pdTRUE)
	{
		if (action != nullptr)
		{
			if(action->type() == Action::Throttle)
			{
				mThrottle.setInput(action->getThrottleValue());
			}
			else if(action->type() == Action::Brake)
			{
				log(Message(Message::LogInfo) << "Brake: " << action->getBrakeValue());
			}

			delete action;
		}
	}

	mThrottle.update();

	if (mThrottle.hasChanged())
	{
		log(Message(Message::Controller) << mThrottle.getOutput());
	}

	osDelay(50);
}

void VehicleTask::cleanup()
{

}

