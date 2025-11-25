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

	mBrake.addFilter(new LowPassFilter(100));

	mMotorEngaged = false;
	mZeroCrossing = false;

	mLogDynamicsTimer = Timer(100, Timer::Continuous);
	mLogDynamicsTimer.startTimer();


}

void VehicleTask::setup()
{

	this->attachLogQueue(LoggerTask.createLogQueue());
	CanHandler.attachLogQueue(LoggerTask.createLogQueue());
	HandleBarTask.attachLogQueue(LoggerTask.createLogQueue());

	mControllerQueue = HandleBarTask.getQueue();
	vQueueAddToRegistry(mControllerQueue, "ControllerActions");

	CanHandler.start("CAN", 256, osPriorityBelowNormal1);
	HandleBarTask.start("HandleBar", 256, osPriorityBelowNormal);
	DirectionHandler.start("Direction", 128, osPriorityHigh3);
	ModbusHandler.start("ModbusMaster", 256, osPriorityHigh);
	Ph_AVG.start("Ph_AVG", 512, osPriorityHigh2);
	Ph_AVD.start("Ph_AVD", 512, osPriorityHigh2);
	Ph_ARG.start("Ph_ARG", 512, osPriorityHigh2);
	Ph_ARD.start("Ph_ARD", 512, osPriorityHigh2);
	LoggerTask.start("Logger", 1024, osPriorityBelowNormal);
}

void VehicleTask::run()
{
	Action* action = nullptr;

	freeHeap = xPortGetFreeHeapSize();
	minEver = xPortGetMinimumEverFreeHeapSize();

	// Read received action from controller
	while(xQueueReceive(mControllerQueue, &action, pdMS_TO_TICKS(10)) == pdTRUE)
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
					DirectionHandler.setDirectionAV(action->getSteeringValue());
					DirectionHandler.setDirectionAR(-action->getSteeringValue());
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

	//Update timers
	mLogDynamicsTimer.tick(osKernelGetTickCount());

	//	Computing data
	mThrottle.update();
	mBrake.update();

	//Changing zero-crossing parameter
//	if(mThrottle.getOutput() < 2.0)
//	{
//		mZeroCrossing = true;
		if(mMotorReversePending)
		{
			mMotorReverseEngaged = mMotorReversePendingValue;
			mMotorReversePending = false;
		}
//	}
//
//	else
//		mZeroCrossing = false;

	setMotorSpeed(mThrottle.getOutput(), mMotorReverseEngaged);

	//Logging dynamics data
	if(mLogDynamicsTimer.triggered())
	{
		Message::ControllerData data;
		data.rawThrottle = mRawThottle;
		data.throttle = mThrottle.getOutput();
		data.rawBrake = mRawBrake;
		data.brake = mBrake.getOutput();
		Message msg(data);
		this->log(msg);
	}
	//osDelay(10);
}

void VehicleTask::cleanup()
{

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
			speed = -speed;

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

