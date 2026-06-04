/*
 * PS3Controller.cpp
 *
 *  Created on: Feb 3, 2025
 *      Author: To
 */

#include "HandleBarController.h"

HandleBarController HandleBarTask;
extern ActionPacketPoolHandler ActionPacketPool;

HandleBarController::HandleBarController()
{
	setRangeFilter(0x20, 0x29);
	setDataRangeFilter(0x20, 0x21);
	setHeartbeatRequired(true);

	setCommunicationTimeout(200);
	setRecoveryMode(5, 100);

	mPreviousTick = 0;
	mPeripheralId = PeripheralId::HandlebarController;
	mPeripheralType = PeripheralType::Controller;

	Mut_Data = xSemaphoreCreateMutex();
	xSemaphoreGive(Mut_Data);

	CanHandler.attach(this);
}

void HandleBarController::setup()
{

}

void HandleBarController::run()
{
	CanPacket* packet = nullptr;
	if(xQueueReceive(mPacketsQueue, &packet, 0) == pdTRUE)
	{
		if(packet != nullptr)
		{
			if(packet->Identifier == 0x20)
				ControllerData(packet);
			else if(packet->Identifier == 0x21)
				ControllerStatus(packet);
			else if (packet->Identifier == 0x28)
			{
				uint32_t hb = packet->data[0] | (packet->data[1] << 8) | (packet->data[2] << 16) | (packet->data[3] << 24);
				heartbeat(hb);
			}
			CanPacketPool.free(packet);
		}
	}

	uint32_t dt = xTaskGetTickCount() - mPreviousTick;
	mPreviousTick = xTaskGetTickCount();

	mBrakeSwitch.tick(dt);
	mLightsSwitch.tick(dt);
	mParkBrakeSwitch.tick(dt);
	mWarningSwitch.tick(dt);
	mHornSwitch.tick(dt);
	mTurnLSwitch.tick(dt);
	mTurnRSwitch.tick(dt);
	mReverseSwitch.tick(dt);

	tick(xTaskGetTickCount());
	osDelay(10);
}

void HandleBarController::ControllerStatus(CanPacket* packet)
{
	//Invalid packet
	if(packet->data.size() < 4)
		return;

	if(xSemaphoreTake(Mut_Data, 10) == pdTRUE)
	{
		uint32_t hb = packet->data[0] | (packet->data[1] << 8) | (packet->data[2] << 16) | (packet->data[3] << 24);
		heartbeat(hb);

		xSemaphoreGive(Mut_Data);
	}
}

void HandleBarController::ControllerData(CanPacket* packet)
{
	//Invalid packet
	if(packet->data.size() < 4)
		return;

	if(xSemaphoreTake(Mut_Data, 10) == pdTRUE)
	{
		mThrottle = uint8_t(packet->data[0]);
		mBrake = uint8_t(packet->data[1]);
		mSteering = int8_t(packet->data[2])*48;
		mParkBrakeSwitch.update(packet->data[3] & 0x80);
		mBrakeSwitch.update((packet->data[1]) > 240); // Brake pressed if value > 240);
		mTurnLSwitch.update((packet->data[3]) & 0x20);
		mTurnRSwitch.update((packet->data[3]) & 0x10);
		mWarningSwitch.update((packet->data[3]) & 0x08);
		mLightsSwitch.update((packet->data[3]) & 0x04);
		mHornSwitch.update((packet->data[3]) & 0x02);
		mReverseSwitch.update((packet->data[3]) & 0x01);

		if(mActive)
		{
			if(mBrake > 20)
				setThrottleCommand(0);
			else
				setThrottleCommand(mThrottle);

			setBrakeCommand(mBrake);
			setSteeringCommand(mSteering);

			switch(mTurnLSwitch.read())
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

			switch(mTurnRSwitch.read())
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

			switch (mWarningSwitch.read())
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

			switch (mHornSwitch.read())
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

			switch (mParkBrakeSwitch.read())
			{
				case Switch::States::RELEASED:
					setMotorEngage(true);
					break;
				default:
					break;
			}

			switch (mBrakeSwitch.read())
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

			switch (mLightsSwitch.read())
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

			switch (mReverseSwitch.read())
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
		}
		xSemaphoreGive(Mut_Data);
	}
}

void HandleBarController::cleanup()
{

}

// ############## CAN Peripheral methods ###############

void HandleBarController::onInit()
{
    CanPacket *settings = CanPacketPool.allocate(0x29);
    settings->data.push_back(0x01);
    if(!CanHandler.send(settings)) // TODO Handle send fail
    	CanPacketPool.free(settings);
}

void HandleBarController::onDiscovered()
{
	log(Message(Message::LogInfo, LOG_HANDLEBAR_DISCOVERED));
	Event e;
	e.type = Event::PeripheralDiscover;
	e.peripheral.id = mPeripheralId;
	e.peripheral.type = mPeripheralType;
	emit(e);
}

void HandleBarController::onReady()
{

}

void HandleBarController::onAbsent()
{
	log(Message(Message::LogCritical, LOG_HANDLEBAR_ABSENT));
	Event e;
	e.type = Event::PeripheralMissing;
	e.peripheral.id = mPeripheralId;
	e.peripheral.type = mPeripheralType;
	emit(e);
}

void HandleBarController::onRecovery()
{
	log(Message(Message::LogError, LOG_HANDLEBAR_RECOVERY_ATTEMPT));

    CanPacket *settings = CanPacketPool.allocate(0x29);
    settings->data.push_back(0x01);
    if(!CanHandler.send(settings)) // TODO Handle send fail
    	CanPacketPool.free(settings);
}

void HandleBarController::onRecovered()
{
	log(Message(Message::LogInfo, LOG_HANDLEBAR_RECOVERED));

	Event e;
	e.type = Event::PeripheralRecovered;
	e.peripheral.id = mPeripheralId;
	e.peripheral.type = mPeripheralType;
	emit(e);
}

void HandleBarController::onLost()
{
	log(Message(Message::LogCritical, LOG_HANDLEBAR_LOST));

	Event e;
	e.type = Event::PeripheralLost;
	e.peripheral.id = mPeripheralId;
	e.peripheral.type = mPeripheralType;
	emit(e);
}

void HandleBarController::onDisabled()
{
    log(Message(Message::LogInfo, LOG_HANDLEBAR_DISABLED));

    CanPacket *settings = CanPacketPool.allocate(0x29);
    settings->data.push_back(0x00);
    if(!CanHandler.send(settings)) // TODO Handle send fail
    	CanPacketPool.free(settings);

    Event e;
    e.type = Event::PeripheralDisabled;
    e.peripheral.id = mPeripheralId;
	e.peripheral.type = mPeripheralType;
    emit(e);
}

