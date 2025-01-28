/*
 * Phaserunner.cpp
 *
 *  Created on: 10 déc. 2024
 *      Author: To
 */

#include "Phaserunner.hpp"

Phaserunner::Phaserunner(uint8_t slaveID)
{
	mConnection.slaveID = slaveID;
	mRegisters = new Registers;

	TimerHeartbeat.setMode(Timer::Continuous);
	TimerHeartbeat.setPeriod(HeartBeat_Rate);
	TimerHeartbeat.startTimer();

	//Controller initialization

}

void Phaserunner::setup()
{
	ModbusHandler.start("ModbusMaster", 128, osPriorityNormal7);

	setCommunicationTimeout(0);
	setControlSource(0);
	setCurrentsLimits(100.0, 100.0);
	setSpeedRegulatorMode(2);
	setTorqueCommand(50.0);
	stopMotor();
	clearFaults();
}

void Phaserunner::run()
{

	if(TimerHeartbeat.triggered())
	{
		heartbeat();
	}

	// Check for modified register that would need writing
	std::vector<Register> pendingRegisters;
	bool needTransmit = false;

	for(auto& r : mRegisters->map)
	{
		if(r.pendingWrite)
		{
			pendingRegisters.push_back(r);
			r.pendingWrite = false;
			needTransmit = true;
		}
	}

	if(needTransmit)
	{
		ModbusPacket* request = new ModbusPacket(mConnection.slaveID, ModbusPacket::Write);
		request->registers = pendingRegisters;
		ModbusHandler.request(request);
	}

	//Check registers that would need to be read
	pendingRegisters.clear();
	needTransmit = false;

	for(auto& r : mRegisters->map)
	{
		if(r.pendingRead)
		{
			pendingRegisters.push_back(r);
			r.pendingRead = false;
			needTransmit = true;
		}
	}

	if(needTransmit)
	{
		ModbusPacket* request = new ModbusPacket(mConnection.slaveID, ModbusPacket::Read);
		request->registers = pendingRegisters;
		ModbusHandler.request(request);
	}

	// Read one received Answer
	ModbusPacket* answer = ModbusHandler.response(mConnection.slaveID);

	if(answer != nullptr)
	{
		if(!answer->success)
		{
			//TODO Warn a about a invalid answer

			for(const auto& r : answer->registers)
			{
				//TODO Print answer
			}
			delete answer;
		}

		else
		{
	//		Serial.print("\tSuccess ");
	//		answer->direction == ModbusPacket::Read ? Serial.println("reading :") : Serial.println("writing :");
	//		for(const auto& r : answer->registers)
	//		{
	//			Serial.print("\t@");
	//			Serial.println(r.address);
	//		}
			for(const auto& r : answer->registers)
			{
				switch(r.address)
				{
					case 258:
						mMotorFaults.faults = r.value;
						break;

					case 299:
						mControllerFaults.faults = r.value;
						break;
				}
			}

			delete answer;
		}
	}

	sleep(10);
}

void Phaserunner::startMotor()
{
	setSpeedCommand(0);
	setRemoteState(2);
}
void Phaserunner::stopMotor()
{
	setSpeedCommand(0);
	setRemoteState(1);
}
void Phaserunner::setSpeed(float speed)
{
	//TODO Filter input and check motor state
	setSpeedCommand(speed);
}
MotorFaults Phaserunner::getMotorFaults()
{
	return mMotorFaults;
}
ControllerFaults Phaserunner::getControllerFaults()
{
	return mControllerFaults;
}
void Phaserunner::clearFaults()
{
	/*
	 *  @508
	 *  Clear faults register
	 *  	Write non zero value clear faults
	 */

	mRegisters->set(508, 1);
}



bool Phaserunner::instantRequest(uint8_t add, uint16_t val)
{
	ModbusPacket* packet = new ModbusPacket(mConnection.slaveID, ModbusPacket::Write);
	packet->push(Register(add, 0, val));
	return ModbusHandler.request(packet);
}
bool Phaserunner::setCommunicationTimeout(uint16_t timeout)
{
	/*
	 *  @32 + @49
	 *  Max time before timeout
	 *  	Value in ms
	 */
	mRegisters->set(32, timeout);
	if(timeout == 0)
		mRegisters->set(49, 0);
	else
		mRegisters->set(49, timeout);

	return true;
}
void Phaserunner::heartbeat()
{
	if(mRegisters->get(493).value == 2)
	{
		setCurrentsLimits(mMotorCommands.MotoringCurrentLimit, mMotorCommands.BrakingCurrentLimit);
		setSpeedCommand(mMotorCommands.Speed);
		setTorqueCommand(mMotorCommands.Torque);
		setRemoteState(mMotorCommands.State);
	}

	setCommunicationTimeout(0);
	readControllerFaults();
	readMotorFaults();
}
bool Phaserunner::readAllParameters()
{
	return true;
}
void Phaserunner::readMotorFaults()
{
	/*
	 *  @258
	 *  Faults register
	 */
	mRegisters->read(258);
}
void Phaserunner::readControllerFaults()
{
	/*
	 *  @299
	 *  Faults register
	 */
	mRegisters->read(299);
}
bool Phaserunner::setControlSource(uint8_t source)
{
	/*
	 *  @208
	 *  Specify command source
	 * 		0 is serial, ..., 5
	 */
	if(source < 0 || source > 5)
		return false;

	mRegisters->set(208, source);

	return true;
}
bool Phaserunner::setSpeedRegulatorMode(uint8_t mode)
{
	/*
	 *  @11
	 *  Specify regulator mode
	 *  	0 - Speed, 1 - Torque, 2 - Speed Limit + Torque
	 */
	if(mode < 0 || mode > 2)
		return false;

	mRegisters->set(11, mode);
	return true;

}
bool Phaserunner::setSpeedCommand(float speed)
{
	/* CRITICAL
	 *  @490
	 *  Value is % of Rated RPM (kV*Bat Voltage)
	 * 		100% is 4096
	 */

	if(speed > 100.0)
		return false;

	mMotorCommands.Speed = speed;

	mRegisters->set(490, 4095*(speed/100.0));
	return true;
}
bool Phaserunner::setCurrentsLimits(float motor, float brake)
{
	/*
	 *  @491 - Motor current
	 *  @492 - Brake current
	 *  Value are % of Nominal currents
	 *  	100% is 4096
	 */

	if(motor > 100.0 || brake > 100.0)
		return false;

	mMotorCommands.MotoringCurrentLimit = motor;
	mMotorCommands.BrakingCurrentLimit = brake;

	mRegisters->set(491, 4096*(motor/100.0));
	mRegisters->set(492, 4096*(brake/100.0));

	return true;
}
bool Phaserunner::setRemoteState(uint8_t state)
{
	/*	CRITICAL
	 *  @493
	 *  Specify motor state
	 *  	0 - OFF, 1 - IDLE, 2 - RUNNING
	 */

	if(state < 0 || state > 2)
		return false;

	mMotorCommands.State = state;

	mRegisters->set(493, state);
	return true;

}
bool Phaserunner::setTorqueCommand(float torque)
{
	/* CRITICAL
	 *  @494
	 *  Value is % of Rated torque
	 * 		100% is 4096
	 */

	if(torque > 100.0)
		return false;

	mMotorCommands.Torque = torque;

	mRegisters->set(494, 4096*(torque/100.0));
	return true;

}
bool Phaserunner::setRemoteThottleVoltage(uint16_t voltage)
{
	/*
	 *  @495
	 *  Value is voltage of a remote throttle command
	 */

	mRegisters->set(495, voltage);
	return true;
}

