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

#define HeartBeat_Rate 100

extern ModbusMaster ModbusHandler;

class Phaserunner : public RTOS_Task
{
	public:
		Phaserunner(uint8_t slaveID);

		struct ConnectionParameters
		{
			bool isConnected;
			uint8_t slaveID;
			//Timer hearthBeat;
		};

		void setup() override;
		void run() override;

		// Methods
		void startMotor();
		void stopMotor();
		void setSpeed(float speed);

		MotorFaults getMotorFaults();
		ControllerFaults getControllerFaults();
		void clearFaults();

	private:

		bool readAllParameters();

		bool setCommunicationTimeout(uint16_t timeout);

		bool setRemoteState(uint8_t state);
		bool setControlSource(uint8_t source);
		bool setSpeedRegulatorMode(uint8_t mode);

		bool setCurrentsLimits(float motor, float brake);
		bool setSpeedCommand(float speed);
		bool setTorqueCommand(float torque);
		bool setRemoteThottleVoltage(uint16_t voltage);

		bool instantRequest(uint8_t add, uint16_t val);

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

		ConnectionParameters mConnection;
		Registers *mRegisters;

		MotorCommands mMotorCommands;
		MotorFaults mMotorFaults;
		ControllerFaults mControllerFaults;
};

#endif /* UTILITIES_SRC_PHASERUNNER_H_ */
