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

	for (int i = 10; i <= 255; i += 10)
	{
		if(i == 10)
			threshold->addThreshold(0, 10, 0);
		else
			threshold->addThreshold(i-9, i, i);
		if (i == 255)
			threshold->addThreshold(250, 255, 255);
	}

	mThrottle.addFilter(threshold);
	mThrottle.addFilter(new SCurveFilter(5));
	mThrottle.addFilter(new LowPassFilter(10));
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

//	if (mThrottle.hasChanged())
//	{
//
//	}
	log(Message(Message::ThrottleOut) << mThrottle.getOutput());

	osDelay(100);
}

void VehicleTask::cleanup()
{

}

