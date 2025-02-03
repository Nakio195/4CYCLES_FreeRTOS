/*
 * PS3Controller.cpp
 *
 *  Created on: Feb 3, 2025
 *      Author: To
 */

#include "PS3Controller.h"

PS3Controller MainController;

PS3Controller::PS3Controller()
{
	setRangeFilter(0x10, 0x15);
	CanHandler.attach(this);
	// TODO Auto-generated constructor stub

}

void PS3Controller::setup()
{
	CanPacket *ControllerSettings = new CanPacket(0x19);
	ControllerSettings->data.push_back(0x01);
	ControllerSettings->data.push_back(0x00);
	ControllerSettings->data.push_back(0x00);
	if(CanHandler.send(ControllerSettings))
		transmissionEnabled = true;

	Mut_Data = xSemaphoreCreateMutex();
	xSemaphoreGive(Mut_Data);
}

void PS3Controller::run()
{
	CanPacket* packet = nullptr;
	if(xQueueReceive(mPacketsQueue, &packet, 0) == pdTRUE)
	{
		if(packet != nullptr)
		{
			if(packet->Identifier == 0x10)
				ControllerData(packet);
			else if(packet->Identifier == 0x11)
				ControllerStatus(packet);
			delete packet;
		}
	}

	osDelay(50);
}

PS3::Data PS3Controller::getData()
{
	if(xSemaphoreTake(Mut_Data, 10) == pdTRUE)
	{
		PS3::Data data = controller;
		xSemaphoreGive(Mut_Data);

		return data;
	}

	return PS3::Data();
}

void PS3Controller::ControllerStatus(CanPacket* packet)
{
	//Invalid packet
	if(packet->data.size() < 5)
		return;

	if(xSemaphoreTake(Mut_Data, 10) == pdTRUE)
	{
		controller.status.battery = packet->data[0];
		controller.status.connected = packet->data[1];
		lastTimestamp |= packet->data[2] << 24;
		lastTimestamp |= packet->data[3] << 16;
		lastTimestamp |= packet->data[4] << 8;
		lastTimestamp |= packet->data[5] & 0xFF;

		xSemaphoreGive(Mut_Data);
	}
}

void PS3Controller::ControllerData(CanPacket* packet)
{
	//Invalid packet
	if(packet->data.size() < 8)
		return;

	if(xSemaphoreTake(Mut_Data, 10) == pdTRUE)
	{
		controller.sticks.L.x = int8_t(packet->data[0]);
		controller.sticks.L.y = int8_t(packet->data[1]);
		controller.sticks.R.x = int8_t(packet->data[2]);
		controller.sticks.R.y = int8_t(packet->data[3]);
		controller.trig.L = packet->data[4];
		controller.trig.R = packet->data[5];

		controller.buttons.cross = packet->data[6] & 0x01;
		controller.buttons.square = packet->data[6] & 0x02;
		controller.buttons.triangle = packet->data[6] & 0x04;
		controller.buttons.circle = packet->data[6] & 0x08;

		controller.dpad.left = packet->data[6] & 0x10;
		controller.dpad.up = packet->data[6] & 0x20;
		controller.dpad.right = packet->data[6] & 0x40;
		controller.dpad.down = packet->data[6] & 0x80;

		controller.buttons.L1 = packet->data[7] & 0x01;
		controller.buttons.R1 = packet->data[7] & 0x02;
		controller.buttons.L3 = packet->data[7] & 0x04;
		controller.buttons.R3 = packet->data[7] & 0x08;

		controller.buttons.start = packet->data[7] & 0x10;
		controller.buttons.select = packet->data[7] & 0x20;
		controller.buttons.ps = packet->data[7] & 0x40;
		controller.status.connected = packet->data[7] & 0x80;

		xSemaphoreGive(Mut_Data);
	}
}

void PS3Controller::cleanup()
{

}
