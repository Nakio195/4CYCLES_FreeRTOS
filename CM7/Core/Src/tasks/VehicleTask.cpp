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

	//mThrottle.addFilter(threshold);
	//mThrottle.addFilter(new SCurveFilter(5));
	mThrottle.addFilter(new LowPassFilter(100));

	mBrake.addFilter(new LowPassFilter(10));

	mMotorEngaged = false;
	mZeroCrossing = false;

	mLogDynamicsTimer = Timer(300, Timer::Continuous);
	mLogDynamicsTimer.startTimer();

	mMotorUpdateTimer = Timer(50, Timer::Continuous);
	mMotorUpdateTimer.startTimer();

	mHeapStatsTimer = Timer(500, Timer::Continuous);
	mHeapStatsTimer.startTimer();

	mEventsQueue = xQueueCreate(40, sizeof(Event));

	mCurrentController = CanPeripheral::PeripheralId::Unknown;

}

void VehicleTask::setup()
{

	this->attachLogQueue(LoggerTask.createLogQueue("Vehicle"));
	CanHandler.attachLogQueue(LoggerTask.createLogQueue("CanHandler"));
	HandleBarTask.attachLogQueue(LoggerTask.createLogQueue("HandleBar"));
	PS3Task.attachLogQueue(LoggerTask.createLogQueue("HandleBar"));
	Ph_AVD.attachLogQueue(LoggerTask.createLogQueue("Ph_AVD"));
	Ph_AVG.attachLogQueue(LoggerTask.createLogQueue("Ph_AVG"));
	Ph_ARD.attachLogQueue(LoggerTask.createLogQueue("Ph_ARD"));
	Ph_ARG.attachLogQueue(LoggerTask.createLogQueue("Ph_ARG"));
	BatteryHandler.attachLogQueue(LoggerTask.createLogQueue("Battery"));

	HandleBarTask.attachEventQueue(mEventsQueue);
	PS3Task.attachEventQueue(mEventsQueue);
	LoggerTask.attachEventQueue(mEventsQueue);
	BatteryHandler.attachEventQueue(mEventsQueue);

	CanHandler.start("CAN", 256, osPriorityAboveNormal);
	HandleBarTask.start("HandleBar", 256, osPriorityBelowNormal);
	PS3Task.start("PS3", 256, osPriorityBelowNormal);
	BatteryHandler.start("Battery", 256, osPriorityBelowNormal);
	DirectionHandler.start("Direction", 128, osPriorityHigh3);
	ModbusHandler.start("ModbusMaster", 256, osPriorityHigh);
	Ph_AVG.start("Ph_AVG", 512, osPriorityHigh2);
	Ph_AVD.start("Ph_AVD", 512, osPriorityHigh2);
	Ph_ARG.start("Ph_ARG", 512, osPriorityHigh2);
	Ph_ARD.start("Ph_ARD", 512, osPriorityHigh2);
	LoggerTask.start("Logger", 1024, osPriorityBelowNormal);

	mLastWakeTime = osKernelGetTickCount();

	//Wait for at least a Controller to be Ready
	while(!mRemoteControllerAvailable && !mLocalControllerAvailable)
	{
	    osDelay(10);
		processEvents();
	}

	updateControllerSelection();
	vQueueAddToRegistry(mControllerQueue, "ControllerActions");
}

void VehicleTask::run()
{
    updateTimers();
    processEvents();
    processActions();
    compute();
    updateVehicle();

    mLastWakeTime += VEHICLE_TICK_MS;
    osDelayUntil(mLastWakeTime);
}

void VehicleTask::updateTimers()
{
	//Update timers
	mLogDynamicsTimer.tick(osKernelGetTickCount());
	mMotorUpdateTimer.tick(osKernelGetTickCount());
	mHeapStatsTimer.tick(osKernelGetTickCount());

	if(mHeapStatsTimer.triggered())
	{
		vPortGetHeapStats(&stats);
		freeHeap = xPortGetFreeHeapSize();
		minEver = xPortGetMinimumEverFreeHeapSize();
	}

}

void VehicleTask::processEvents()
{
	//Read received events from can Peripherals
	Event event;

	while(xQueueReceive(mEventsQueue, &event, pdMS_TO_TICKS(0)) == pdTRUE)
	{
		if(event.type == Event::PeripheralEvent)
		{
			if(event.peripheral.type == CanPeripheral::Controller)
			{
				switch (event.peripheral.event)
				{
					case Event::Peripheral::Disabled:
					case Event::Peripheral::Lost:
					case Event::Peripheral::Missing:
						if(event.peripheral.id == CanPeripheral::RemoteController)
							mRemoteControllerAvailable = false;
						else if(event.peripheral.id == CanPeripheral::HandlebarController)
							mLocalControllerAvailable = false;
						updateControllerSelection();
						break;
					case Event::Peripheral::Ready:
						if(event.peripheral.id == CanPeripheral::RemoteController)
							mRemoteControllerAvailable = true;
						else if(event.peripheral.id == CanPeripheral::HandlebarController)
							mLocalControllerAvailable = true;
						updateControllerSelection();
						break;

					default:
						// TODO: log invalid type
						break;
				}
			}
		}

		if(event.type == Event::ControllerEvent)
		{
			if(event.controller.request == Event::Controller::Change)
			{
				if(mCurrentController == CanPeripheral::RemoteController && mLocalControllerAvailable)
					changeController(CanPeripheral::HandlebarController);
				else if(mCurrentController == CanPeripheral::HandlebarController && mRemoteControllerAvailable)
					changeController(CanPeripheral::RemoteController);
			}
		}
	}
}

void VehicleTask::processActions()
{
	// Read received action from controller
	Action* action = nullptr;
	while(xQueueReceive(mControllerQueue, &action, pdMS_TO_TICKS(0)) == pdTRUE)
	{
		if (action != nullptr)
		{
			switch (action->type())
			{
				case Action::Throttle:
					mRawThottle = action->getThrottleValue();
					mThrottle.setInput(action->getThrottleValue());
					break;
				case Action::Brake:
					mRawBrake = action->getBrakeValue();
					mBrake.setInput(action->getBrakeValue());
					break;
				case Action::Lights:
					handleLightsAction(action);
					break;
				case Action::Steering:
					mSteeringCommand_AV = action->getSteeringValue();
					mSteeringCommand_AR = action->getSteeringValue();
					break;
				case Action::Engage:
					mMotorEngaged = !mMotorEngaged;

					if(mMotorEngaged)
						engageMotor();
					else
						disengageMotor();
					break;

				case Action::Motor:
					if (action->getReverse() == Action::Gear::Reverse)
					{
						mMotorReversePending = true;
						mMotorReversePendingValue = true;
					}
					else if(action->getReverse() == Action::Gear::Forward)
					{
						mMotorReversePending = true;
						mMotorReversePendingValue = false;
					}
					break;
				default:
					// TODO: log invalid type
					break;
			}

			ActionPacketPool.free(action);
		}
	}
}

void VehicleTask::compute()
{
    mThrottle.update();
    mBrake.update();

    if (mThrottle.getOutput() < 2.0 && mMotorReversePending)
    {
        mMotorReverseEngaged = mMotorReversePendingValue;
        mMotorReversePending = false;
    }
}


void VehicleTask::updateVehicle()
{
    DirectionHandler.setDirectionAV(mSteeringCommand_AV);
    DirectionHandler.setDirectionAR(-mSteeringCommand_AR);
    DirectionHandler.setBrakeAV( mBrake.getOutput() * 200);
    DirectionHandler.setBrakeAR(-mBrake.getOutput() * 200);

    if (mMotorUpdateTimer.triggered())
    {
        setMotorSpeed(mThrottle.getOutput(), mMotorReverseEngaged);
        setMotorEBrake(mBrake.getOutput());
    }

    if (mLogDynamicsTimer.triggered())
    {
        Message msg(Message::Controller);
        msg << mRawThottle << mThrottle.getOutput() << mRawBrake << mBrake.getOutput();
        log(msg);

        Ph_AVD.logMotorInfo();
        Ph_AVG.logMotorInfo();
        Ph_ARD.logMotorInfo();
        Ph_ARG.logMotorInfo();
    }
}

void VehicleTask::cleanup()
{

}

void VehicleTask::enterCriticalError()
{
	log(Message(Message::LogCritical, LOG_VEHICLE_CRITICAL_STATE));
	disengageMotor();

	suspend();
	while(1); // Should never reach this point
}

void VehicleTask::updateControllerSelection()
{
	if(mCurrentController == CanPeripheral::RemoteController)
	{
		if(mRemoteControllerAvailable)
			return;
		else
		{
			if(mLocalControllerAvailable)
				changeController(CanPeripheral::HandlebarController);
			else
				enterCriticalError();
		}
	}

	if(mCurrentController == CanPeripheral::HandlebarController)
	{
		if(mLocalControllerAvailable)
			return;
		else
		{
			if(mRemoteControllerAvailable)
				changeController(CanPeripheral::RemoteController);
			else
				enterCriticalError();
		}
	}

	if(mCurrentController == CanPeripheral::PeripheralId::Unknown)
	{
		if(mLocalControllerAvailable)
			changeController(CanPeripheral::RemoteController);
		else if(mRemoteControllerAvailable)
			changeController(CanPeripheral::HandlebarController);
		else
			enterCriticalError();
	}
}

void VehicleTask::changeController(CanPeripheral::PeripheralId newController)
{
	if(newController == CanPeripheral::RemoteController)
	{
		HandleBarTask.setActive(false);
		mControllerQueue = PS3Task.getControllerQueue();
		PS3Task.setActive(true);
		mCurrentController = CanPeripheral::RemoteController;
	}

	else if(newController == CanPeripheral::HandlebarController)
	{
		PS3Task.setActive(false);
		mControllerQueue = HandleBarTask.getControllerQueue();
		HandleBarTask.setActive(true);
		mCurrentController = CanPeripheral::HandlebarController;
	}
}

void VehicleTask::engageMotor()
{
	Ph_AVG.startMotor();
	Ph_AVD.startMotor();
	Ph_ARG.startMotor();
	Ph_ARD.startMotor();
}

void VehicleTask::disengageMotor()
{
	Ph_AVG.stopMotor();
	Ph_AVD.stopMotor();
	Ph_ARG.stopMotor();
	Ph_ARD.stopMotor();
}

void VehicleTask::setMotorSpeed(float speed, bool reverse)
{
	if(mMotorEngaged)
	{
		if(reverse)
			speed = -speed/2.0;

		Ph_AVG.setSpeed(float(speed)/255.0*100.0);
		Ph_AVD.setSpeed(float(speed)/255.0*100.0);
		Ph_ARG.setSpeed(float(speed)/255.0*100.0);
		Ph_ARD.setSpeed(float(speed)/255.0*100.0);
	}
}


void VehicleTask::setMotorEBrake(float brake)
{
	if(mMotorEngaged)
	{
		Ph_AVG.setBrake(float(brake)/255.0*100.0);
		Ph_AVD.setBrake(float(brake)/255.0*100.0);
		Ph_ARG.setBrake(float(brake)/255.0*100.0);
		Ph_ARD.setBrake(float(brake)/255.0*100.0);
	}
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

float VehicleTask::getMeanSpeed()
{
	return (Ph_AVG.getMotorInfo().motorSpeed +
			Ph_AVD.getMotorInfo().motorSpeed +
			Ph_ARG.getMotorInfo().motorSpeed +
			Ph_ARD.getMotorInfo().motorSpeed)
			/ 4.0;
}

float VehicleTask::getMaxSpeed()
{
	return Ph_AVD.getMotorInfo().speedLimit;
}


int32_t VehicleTask::normalizeSterring(int32_t steering)
{
	int16_t indice = 0;
	float meanSpeed = getMeanSpeed();

	if(meanSpeed > 19.9)
		meanSpeed = 19.9;
	indice = (meanSpeed / 20.0) *255;

	indice = indice > 255 ? 255 : indice;
	indice = indice < 0 ? 0 : indice;

	float coeff = float(LUT_DirectionVsSpeed[indice])/255.0;
	return int32_t(float(steering) * coeff);
}

