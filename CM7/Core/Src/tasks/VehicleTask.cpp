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
	CanHandler.start("CAN", 256, osPriorityHigh);
	MainController.start("PS3", 128, osPriorityAboveNormal);
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

	if(!MainController.isResponding())
	{
		std::string value = "[WARNING] - PS3Controller not responding";
		value += "\n";
		CDC_Transmit_FS((uint8_t*)value.data(), value.size());
		osDelay(500);
	}

}

void VehicleTask::cleanup()
{

}

