#pragma once

#include <stdint.h>
#include <string>
//#include "ArduinoJson-v7.3.0.h"

#include "FreeRTOS.h"
#include "task.h"

class Message
{
	public:

		struct ControllerData
		{
			uint8_t rawThrottle = 0;
			uint8_t throttle = 0;
			uint8_t rawBrake = 0;
			uint8_t brake = 0;
			uint8_t commands = 0;
		};

	public:
		enum Type{LogDebug, LogInfo, LogWarning, LogError, LogCritical, Controller, Electrics};

		Message(Type type);
		Message(ControllerData data) : mControllerData(data), mType(Controller)
		{
			mTimestamp = xTaskGetTickCount();
		}

		Type type();
		Type level();

		uint32_t code();
		uint32_t timestamp();

		Message& operator<<(Type type);
		Message& operator<<(uint32_t number);
		//Message& operator<<(std::initializer_list<std::pair<const char*, JsonVariant>> values);

		uint32_t mTimestamp;

		uint32_t mMessageCode;
		ControllerData mControllerData;

	protected:
		Type mType;
};

enum {
	// CAN Peripherals
	LOG_PS3_CONTROLLER_ABSENT = 1, //PS3 Controller absent from bus
	LOG_PS3_CONTROLLER_CONNECTED, //PS3 Controller connected to bus
	LOG_PS3_CONTROLLER_RECOVERY_ATTEMPT, //PS3 Controller recovery attempt
	LOG_PS3_CONTROLLER_LOST, //Logger lost
	LOG_PS3_CONTROLLER_RECOVERED, //Logger recovered

	LOG_LOGGER_ABSENT, //Logger absent from bus
	LOG_LOGGER_CONNECTED, //Logger connected to bus
	LOG_LOGGER_RECOVERY_ATTEMPT, //Logger recovery attempt
	LOG_LOGGER_LOST, //Logger lost
	LOG_LOGGER_RECOVERED, //Logger recovered

	LOG_HANDLEBAR_ABSENT, //HandleBar absent from bus
	LOG_HANDLEBAR_CONNECTED, //HandleBar connected to bus
	LOG_HANDLEBAR_RECOVERY_ATTEMPT, //HandleBar recovery attempt
	LOG_HANDLEBAR_LOST, //HandleBar lost
	LOG_HANDLEBAR_RECOVERED, //HandleBar recovered

	// Motor Status
	LOG_VEHICLE_MOTOR_ENGAGED, // Vehicle motor engaged
	LOG_VEHICLE_MOTOR_DISENGAGED, // Vehicle motor disengaged
	LOG_VEHICLE_REVERSE_ENGAGED, // Vehicle reverse gear engaged
	LOG_VEHICLE_REVERSE_DISENGAGED, // Vehicle reverse gear disengaged
	LOG_VEHICLE_MOTOR_FAULTS_DETECTED // Vehicle motor faults detected
};

