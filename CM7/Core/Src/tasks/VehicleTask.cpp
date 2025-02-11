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
	// TODO Auto-generated constructor stub

}

void VehicleTask::setup()
{

	this->attachLogQueue(Json.createLogQueue());
	CanHandler.attachLogQueue(Json.createLogQueue());
	MainController.attachLogQueue(Json.createLogQueue());

	CanHandler.start("CAN", 256, osPriorityHigh);
	MainController.start("PS3", 128, osPriorityAboveNormal);
	Json.start("JSON Logger", 256, osPriorityBelowNormal);
}

void VehicleTask::run()
{
	osDelay(50);
	PS3::Data controller = MainController.getData();
	if(controller.buttons.cross)
	{
		std::string value = std::to_string(controller.sticks.L.x);
		value += ",";
		value += std::to_string(controller.sticks.L.y);
		value += "\n";

		CDC_Transmit_FS((uint8_t*)value.data(), value.size());
	}

}

void VehicleTask::cleanup()
{

}

