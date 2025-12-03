/*
 * Phaserunner.h
 *
 *  Created on: 10 déc. 2024
 *      Author: To
 */

#ifndef UTILITIES_SRC_PHASERUNNER_H_
#define UTILITIES_SRC_PHASERUNNER_H_

#include <stdint.h>

#include "ModbusMaster.hpp"
#include "PhaserunnerRegisterMap.h"
#include "tools/Timer.hpp"

#include "../RTOSTask.h"

#define HeartBeat_Rate 300

extern ModbusMaster ModbusHandler;

class Phaserunner : public RTOS_Task
{
	public:
		Phaserunner(uint8_t slaveID);

		struct ConnectionParameters
		{
			bool isConnected;
			uint8_t slaveID;
		};

		void setup() override;
		void run() override;

		// Methods
		void startMotor();
		void stopMotor();
		void setSpeed(float speed);
		void setBrake(float speed);

		MotorInfo getMotorInfo() const;
		MotorFaults getMotorFaults();

		void logMotorInfo();

		ControllerFaults getControllerFaults();
		void clearFaults();

	private:

		bool readAllParameters();
		bool readRegister(uint16_t address);
		bool writeRegister(uint16_t address, uint16_t value);

		bool setCommunicationTimeout(uint16_t timeout);

		bool setRemoteState(uint8_t state);
		bool setControlSource(uint8_t source);
		bool setSpeedRegulatorMode(uint8_t mode);

		bool setCurrentsLimits(float motor, float brake);
		bool setBrakeCurrent(float brake);
		bool setSpeedCommand(float speed);
		bool setTorqueCommand(float torque);
		bool setRemoteThottleVoltage(uint16_t voltage);

		bool instantRequest(uint8_t add, uint16_t val);

		void readMotorInfo();
		void readMotorFaults();
		void readControllerFaults();

		void heartbeat();

	private:

		struct MotorCommands
		{
			float MotoringCurrentLimit;
			float BrakingCurrentLimit;
			float Speed;
			float Torque;
			uint8_t State;
		};

		Timer TimerHeartbeat;
		uint8_t mHeartbeatCounter;

		ConnectionParameters mConnection;
		Registers *mRegisters;
		QueueHandle_t RegisterQueue;

	    static constexpr size_t MAX_PENDING_REGS = 64;
	    static constexpr size_t MODBUS_MAX_REGS = 125;

	    Register mPendingWriteRegisters[MAX_PENDING_REGS];
	    size_t mPendingWriteCount = 0;

	    Register mPendingReadRegisters[MAX_PENDING_REGS];
	    size_t mPendingReadCount = 0;

	    Register mReceivedRegisters[MODBUS_MAX_REGS];
	    size_t mReceivedCount = 0;

	    // Optional double buffer for blocks to avoid malloc
	    Register writeBlock[MODBUS_MAX_REGS];
	    size_t writeBlockSize = 0;

	    Register readBlock[MODBUS_MAX_REGS];
	    size_t readBlockSize = 0;

		MotorInfo mMotorInfo;
		MotorCommands mMotorCommands;
		MotorFaults mMotorFaults;
		ControllerFaults mControllerFaults;
};

extern Phaserunner Ph_AVG;
extern Phaserunner Ph_AVD;
extern Phaserunner Ph_ARG;
extern Phaserunner Ph_ARD;

#endif /* UTILITIES_SRC_PHASERUNNER_H_ */
