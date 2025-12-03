/*
 * Phaserunner.cpp
 *
 *  Created on: 10 déc. 2024
 *      Author: To
 */

#include "Phaserunner.hpp"
#include <algorithm>

Phaserunner Ph_AVG(1);
Phaserunner Ph_AVD(2);
Phaserunner Ph_ARG(3);
Phaserunner Ph_ARD(4);

Phaserunner::Phaserunner(uint8_t slaveID)
{
	mConnection.slaveID = slaveID;
	mRegisters = new Registers;

	TimerHeartbeat.setMode(Timer::Continuous);
	TimerHeartbeat.setPeriod(HeartBeat_Rate);
	TimerHeartbeat.startTimer();
	mHeartbeatCounter = 0;

	RegisterQueue = xQueueCreate(32, sizeof(Register));

}

void Phaserunner::setup()
{
	osDelay(2000);
	setCommunicationTimeout(HeartBeat_Rate*2); // Gestion timeout
	setControlSource(0);	// 0 Serial
	setCurrentsLimits(20.0, 15.0);
	setSpeedRegulatorMode(0);
	setRemoteState(1); // 1 IDLE
	clearFaults();
	stopMotor();
}


void Phaserunner::run()
{
	uint32_t loopStartTick = osKernelGetTickCount();

    TimerHeartbeat.tick(osKernelGetTickCount());

    if (TimerHeartbeat.triggered())
    {
        heartbeat();
        getMotorInfo();
    }

    // ----- Collect registers from queue -----
    Register receivedRegister;
    while (xQueueReceive(RegisterQueue, &receivedRegister, 0) == pdTRUE)
    {
        if (receivedRegister.pendingWrite && mPendingWriteCount < MAX_PENDING_REGS)
            mPendingWriteRegisters[mPendingWriteCount++] = receivedRegister;
        else if (receivedRegister.pendingRead && mPendingReadCount < MAX_PENDING_REGS)
            mPendingReadRegisters[mPendingReadCount++] = receivedRegister;
    }

    // ----- Process write registers -----
    size_t i = 0;
    while (i < mPendingWriteCount)
    {

    	// Create a block of consecutive registers to write
        writeBlockSize = 0;
        writeBlock[writeBlockSize] = mPendingWriteRegisters[i];
        writeBlockSize++;

        size_t j = i + 1;
        // Find consecutive registers
        while (j < mPendingWriteCount &&
               mPendingWriteRegisters[j].address == writeBlock[writeBlockSize-1].address + 1 &&
               writeBlockSize < MODBUS_MAX_REGS)
        {
            writeBlock[writeBlockSize++] = mPendingWriteRegisters[j];
            j++;
        }

        // End of block found, create and send Modbus packet

        ModbusPacket* packet = ModbusPacketPool.allocate(mConnection.slaveID, ModbusPacket::Write);
        packet->registers.assign(writeBlock, writeBlock + writeBlockSize);

        if (ModbusHandler.request(packet) != pdTRUE)
            ModbusPacketPool.free(packet);

        i = j;
    }
    mPendingWriteCount = 0;

    // ----- Process read registers -----
    i = 0;
    while (i < mPendingReadCount)
    {
        readBlockSize = 0;
        readBlock[readBlockSize] = mPendingReadRegisters[i].address;
        readBlockSize++;

        size_t j = i + 1;
        while (j < mPendingReadCount && mPendingReadRegisters[j].address == readBlock[readBlockSize-1].address + 1 && readBlockSize < MODBUS_MAX_REGS)
        {
            readBlock[readBlockSize++] = mPendingReadRegisters[j].address;
            j++;
        }

        ModbusPacket* packet = ModbusPacketPool.allocate(mConnection.slaveID, ModbusPacket::Read);
        packet->registers.assign(readBlock, readBlock + readBlockSize);

        if (ModbusHandler.request(packet) != pdTRUE)
            ModbusPacketPool.free(packet);

        i = j;
    }
    mPendingReadCount = 0;

    // ----- Process Modbus answers -----
    ModbusPacket* answer = nullptr;
    while ((answer = ModbusHandler.response(mConnection.slaveID)) != nullptr)
    {
        if (!answer->success)
        {
            // TODO: Log warning for invalid answer
            ModbusPacketPool.free(answer);
        }
        else
        {
            for (Register& r : answer->registers)
            {
                r = mRegisters->update(r.address, r.value);

                switch (r.address)
                {
                    case 258: mMotorFaults.faults = r.value; break;
                    case 259: mMotorInfo.controllerTemp = r.getValue(); break;
                    case 260: mMotorInfo.vehicleSpeed = r.getValue(); break;
                    case 261: mMotorInfo.motorTemp = r.getValue(); break;
                    case 262: mMotorInfo.motorCurrent = r.getValue(); break;
                    case 263: mMotorInfo.motorRPM = r.getValue(); break;
                    case 264: mMotorInfo.motorSpeed = r.getValue(); break;
                    case 265: mMotorInfo.busVoltage = r.getValue(); break;
                    case 266: mMotorInfo.busCurrent = r.getValue(); break;
                    case 299: mControllerFaults.faults = r.value; break;
                }
            }
            ModbusPacketPool.free(answer);
        }
    }

    // Ensure a minimum loop time
    uint32_t loopEndTick = osKernelGetTickCount();
    uint32_t loopDuration = loopEndTick - loopStartTick;
    if(loopDuration < 10)
    	osDelay(10 - loopDuration); // Yield to other tasks
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

void Phaserunner::setBrake(float brake)
{
	//TODO Filter input and check motor state
	setCurrentsLimits(mMotorCommands.MotoringCurrentLimit, brake);
}


void Phaserunner::logMotorInfo()
{
	Message msg(Message::Motor);
	msg << (uint8_t)(mConnection.slaveID);
	msg << (uint16_t)(mMotorInfo.vehicleSpeed*256.0);
	msg << (uint16_t)(mMotorInfo.motorCurrent*32.0);
	msg << (uint16_t)(mMotorInfo.busVoltage*32.0);
	this->log(msg);
}

MotorInfo Phaserunner::getMotorInfo() const
{
	/*
	 *  @260 - 265
	 *  Vehicle speed, motor temperature, motor current, motor rpm, motor speed, bus voltage
	 */
	return mMotorInfo;
}

MotorFaults Phaserunner::getMotorFaults()
{
	return mMotorFaults;
}
ControllerFaults Phaserunner::getControllerFaults()
{
	return mControllerFaults;
}


bool Phaserunner::readRegister(uint16_t address)
{
	Register reg = mRegisters->get(address);
	reg.pendingRead = true;
	reg.pendingWrite = false;
	return xQueueSend(RegisterQueue, &reg, 0);
}

bool Phaserunner::writeRegister(uint16_t address, uint16_t value)
{
	Register reg = mRegisters->get(address);
	reg.value = value;
	reg.pendingWrite = true;
	reg.pendingRead = false;
	return xQueueSend(RegisterQueue, &reg, 0);
}


void Phaserunner::clearFaults()
{
	/*
	 *  @508
	 *  Clear faults register
	 *  	Write non zero value clear faults
	 */

	writeRegister(508, 1);

}



//bool Phaserunner::instantRequest(uint8_t add, uint16_t val)
//{
//	ModbusPacket* packet = ModbusPacketPool.allocate(mConnection.slaveID, ModbusPacket::Write);
//	packet->push(Register(add, 0, val));
//	return ModbusHandler.request(packet);
//}
bool Phaserunner::setCommunicationTimeout(uint16_t timeout)
{
	/*
	 *  @32 + @49
	 *  Max time before timeout
	 *  	Value in ms
	 */
	writeRegister(32, timeout);
//	if(timeout == 0)
//		writeRegister(49, 0);
//	else
		writeRegister(49, 0);


	return true;
}
void Phaserunner::heartbeat()
{
	setRemoteState(mMotorCommands.State);
	readMotorInfo();
	readControllerFaults();

	if(!mMotorFaults.ready())
		log(Message(Message::LogCritical) << LOG_VEHICLE_MOTOR_FAULTS_DETECTED);
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
	readRegister(258);
}


void Phaserunner::readMotorInfo()
{
	/*
	 *  @258 - 265
	 *  Motor Faults, Controller Temp, Vehicle speed, motor temperature, motor current, motor rpm, motor speed, bus voltage
	 */
	readRegister(258);
	readRegister(259);
	readRegister(260);
	readRegister(261);
	readRegister(262);
	readRegister(263);
	readRegister(264);
	readRegister(265);
}

void Phaserunner::readControllerFaults()
{
	/*
	 *  @299
	 *  Faults register
	 */
	readRegister(299);
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


	writeRegister(208, source);

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

	writeRegister(11, mode);
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

	if(speed < -100.0)
		return false;

	if(speed > -2.0 && speed < 2.0)
		speed = 0.0;


	mMotorCommands.Speed = speed;

	writeRegister(490, (int16_t)((4095*(speed/400.0))));

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

	writeRegister(491, 4095*(motor/100.0));
	writeRegister(492, 4095*(brake/100.0));

	return true;
}

bool Phaserunner::setBrakeCurrent(float brake)
{
	/*
	 *  @492 - Brake current
	 *  Value are % of Nominal currents
	 *  	100% is 4096
	 */

	if(brake > 100.0 || brake < 0.0)
		return false;

	mMotorCommands.BrakingCurrentLimit = brake;

	writeRegister(492, 4095*(brake/100.0));

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
	mHeartbeatCounter++;
	uint16_t value = (state | (uint16_t)(mHeartbeatCounter) << 8); // Keep upper byte for heartbeat detecti
	writeRegister(493, value);
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

	writeRegister(494, 4095*(torque/100.0));
	return true;

}
bool Phaserunner::setRemoteThottleVoltage(uint16_t voltage)
{
	/*
	 *  @495
	 *  Value is voltage of a remote throttle command
	 */

	writeRegister(495, voltage);
	return true;
}

