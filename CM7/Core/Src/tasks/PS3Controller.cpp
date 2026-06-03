/*
 * PS3Controller.cpp
 *
 *  Created on: Feb 3, 2025
 *      Author: To
 */

#include "PS3Controller.h"

PS3Controller PS3Task;
ActionPacketPoolHandler ActionPacketPool;

PS3Controller::PS3Controller()
{
	setRangeFilter(0x10, 0x15);
	setCommunicationTimeout(3000);
	setRecoveryMode(50, 1000);

	mPreviousTick = 0;
	mPeripheralId = PeripheralId::RemoteController;
	mPeripheralType = PeripheralType::Controller;

	CanHandler.attach(this);
}

void PS3Controller::onInit()
{
	CanPacket *ControllerSettings = CanPacketPool.allocate(0x19);
	ControllerSettings->data.push_back(0x01);
	ControllerSettings->data.push_back(0x00);
	ControllerSettings->data.push_back(0x00);
	if(!CanHandler.send(ControllerSettings)) //TODO Handle multiple failed init
		CanPacketPool.free(ControllerSettings);

}

void PS3Controller::onDiscovered()
{
	log(Message(Message::LogCritical, LOG_PS3_CONTROLLER_CONNECTED));
}
void PS3Controller::onAbsent()
{
	log(Message(Message::LogCritical, LOG_PS3_CONTROLLER_ABSENT));
	while(1)
	{
		osDelay(10000);
	}
}

void PS3Controller::onRecovery()
{
	log(Message(Message::LogError, LOG_PS3_CONTROLLER_RECOVERY_ATTEMPT));
}

void PS3Controller::onRecovered()
{
	log(Message(Message::LogInfo, LOG_PS3_CONTROLLER_RECOVERED));
}

void PS3Controller::onLost()
{
	log(Message(Message::LogError, LOG_PS3_CONTROLLER_LOST));
}

void PS3Controller::setup()
{
	Mut_Data = xSemaphoreCreateMutex();
	xSemaphoreGive(Mut_Data);
}

void PS3Controller::onDisabled()
{
    log(Message(Message::LogInfo, LOG_HANDLEBAR_DISABLED));

    Event e;
    e.type = Event::PeripheralDisconnect;
    e.PeripheralDiscovered.id = mPeripheralId;
    emit(e);
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
			CanPacketPool.free(packet);
		}
	}

	uint32_t dt = xTaskGetTickCount() - mPreviousTick;
	mPreviousTick = xTaskGetTickCount();

	controller.buttons.cross.tick(dt);
	controller.buttons.square.tick(dt);
	controller.buttons.triangle.tick(dt);
	controller.buttons.circle.tick(dt);

	controller.dpad.left.tick(dt);
	controller.dpad.up.tick(dt);
	controller.dpad.right.tick(dt);
	controller.dpad.down.tick(dt);

	controller.trig.L2.tick(dt);
	controller.trig.R2.tick(dt);

	controller.buttons.L1.tick(dt);
	controller.buttons.R1.tick(dt);
	controller.buttons.L3.tick(dt);
	controller.buttons.R3.tick(dt);

	controller.buttons.start.tick(dt);
	controller.buttons.select.tick(dt);
	controller.buttons.ps.tick(dt);
	controller.status.connected.tick(dt);
	tick(xTaskGetTickCount());
	osDelay(10);
}

void PS3Controller::ControllerStatus(CanPacket* packet)
{
	//Invalid packet
	if(packet->data.size() < 5)
		return;

	if(xSemaphoreTake(Mut_Data, 10) == pdTRUE)
	{
		controller.status.battery = packet->data[0];
		controller.status.connected.update(packet->data[1]);
		controller.status.timestamp = 0;
		controller.status.timestamp |= packet->data[2] << 24;
		controller.status.timestamp |= packet->data[3] << 16;
		controller.status.timestamp |= packet->data[4] << 8;
		controller.status.timestamp |= packet->data[5] & 0xFF;

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
		controller.trig.L2.update(packet->data[4] > 0);
		controller.trig.R2.update(packet->data[5] > 0);

		controller.buttons.cross.update(packet->data[6] & 0x01);
		controller.buttons.square.update(packet->data[6] & 0x02);
		controller.buttons.triangle.update(packet->data[6] & 0x04);
		controller.buttons.circle.update(packet->data[6] & 0x08);

		controller.dpad.left.update(packet->data[6] & 0x10);
		controller.dpad.up.update(packet->data[6] & 0x20);
		controller.dpad.right.update(packet->data[6] & 0x40);
		controller.dpad.down.update(packet->data[6] & 0x80);

		controller.buttons.L1.update(packet->data[7] & 0x01);
		controller.buttons.R1.update(packet->data[7] & 0x02);
		controller.buttons.L3.update(packet->data[7] & 0x04);
		controller.buttons.R3.update(packet->data[7] & 0x08);

		controller.buttons.start.update(packet->data[7] & 0x10);
		controller.buttons.select.update(packet->data[7] & 0x20);
		controller.buttons.ps.update(packet->data[7] & 0x40);
		controller.status.connected.update(packet->data[7] & 0x80);

		setThrottleCommand(controller.trig.R);
		setBrakeCommand(controller.trig.L);
		setSteeringCommand(controller.sticks.L.x*48);

		switch(controller.buttons.L1.read())
		{
			case Switch::States::PRESSED:
				setLightsCommand(Action::Signals::Left, true);
				HAL_GPIO_WritePin(SND_0_GPIO_Port, SND_0_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(SND_1_GPIO_Port, SND_1_Pin, GPIO_PIN_RESET);
				break;
			case Switch::States::RELEASED:
				setLightsCommand(Action::Signals::Left, false);
				HAL_GPIO_WritePin(SND_0_GPIO_Port, SND_0_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(SND_1_GPIO_Port, SND_1_Pin, GPIO_PIN_SET);
				break;
			default:
				break;
		}
		switch(controller.buttons.R1.read())
		{
			case Switch::States::PRESSED:
				setLightsCommand(Action::Signals::Right, true);
				HAL_GPIO_WritePin(SND_0_GPIO_Port, SND_0_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(SND_1_GPIO_Port, SND_1_Pin, GPIO_PIN_RESET);
				break;
			case Switch::States::RELEASED:
				setLightsCommand(Action::Signals::Right, false);
				HAL_GPIO_WritePin(SND_0_GPIO_Port, SND_0_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(SND_1_GPIO_Port, SND_1_Pin, GPIO_PIN_SET);
				break;
			default:
				break;
		}

		switch (controller.buttons.triangle.read())
		{
			case Switch::States::PRESSED:
				setLightsCommand(Action::Signals::Hazard, true);
				HAL_GPIO_WritePin(SND_0_GPIO_Port, SND_0_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SND_1_GPIO_Port, SND_1_Pin, GPIO_PIN_SET);
				break;
			case Switch::States::RELEASED:
				setLightsCommand(Action::Signals::Hazard, false);
				HAL_GPIO_WritePin(SND_0_GPIO_Port, SND_0_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(SND_1_GPIO_Port, SND_1_Pin, GPIO_PIN_SET);
				break;
			default:
				break;
		}

		switch (controller.status.connected.read())
		{
			case Switch::States::PRESSED:
				setLightsCommand(Action::Signals::Hazard, true);
				HAL_GPIO_WritePin(SND_0_GPIO_Port, SND_0_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SND_1_GPIO_Port, SND_1_Pin, GPIO_PIN_RESET);
				break;
			case Switch::States::RELEASED:
				setLightsCommand(Action::Signals::Hazard, false);
				break;
			default:
				break;
		}

		switch (controller.buttons.start.read())
		{
			case Switch::States::RELEASED:
				setMotorEngage(true);
				break;
			default:
				break;
		}

		switch (controller.trig.L2.read())
		{
			case Switch::States::PRESSED:
				setLightsCommand(Action::Signals::BrakeSignal, true);
				break;
			case Switch::States::RELEASED:
				setLightsCommand(Action::Signals::BrakeSignal, false);
				break;
			default:
				break;
		}

		switch (controller.buttons.circle.read())
		{
			case Switch::States::PRESSED:
				setLightsCommand(Action::Signals::NighLight, true);
				break;
			case Switch::States::RELEASED:
				setLightsCommand(Action::Signals::NighLight, false);
				break;
			default:
				break;
		}

		switch (controller.buttons.square.read())
		{
			case Switch::States::PRESSED:
				setReverseCommand(Action::Gear::Reverse);
				break;
			case Switch::States::RELEASED:
				setReverseCommand(Action::Gear::Forward);
				break;
			default:
				break;
		}

		xSemaphoreGive(Mut_Data);
	}
}

void PS3Controller::cleanup()
{

}
