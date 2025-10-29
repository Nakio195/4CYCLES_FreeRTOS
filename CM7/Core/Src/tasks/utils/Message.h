#pragma once

#include <stdint.h>
#include <string>
//#include "ArduinoJson-v7.3.0.h"

#include "FreeRTOS.h"
#include "task.h"

class Message
{
	public:

		struct DynamicsData
		{
			uint8_t speed = 0;
			uint8_t throttle = 0;
			uint8_t brake = 0;
		};

	public:
		enum Type{LogDebug, LogInfo, LogWarning, LogError, LogCritical, Dynamics, Electrics};

		Message(Type type);
		Message(DynamicsData data) : mDynamicsData(data), mType(Dynamics)
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
		DynamicsData mDynamicsData;

	protected:
		Type mType;
};

enum {
	LOG_PS3_CONTROLLER_ABSENT, //PS3 Controller absent from bus
	LOG_PS3_CONTROLLER_CONNECTED, //PS3 Controller connected to bus
	LOG_PS3_CONTROLLER_RECOVERY_ATTEMPT, //PS3 Controller recovery attempt
	LOG_PS3_CONTROLLER_LOST, //PS3 Controller lost
	LOG_PS3_CONTROLLER_RECOVERED, //PS3 Controller recovered
	LOG_VEHICLE_MOTOR_ENGAGED, // Vehicle motor engaged
	LOG_VEHICLE_MOTOR_DISENGAGED, // Vehicle motor disengaged
	LOG_VEHICLE_REVERSE_ENGAGED, // Vehicle reverse gear engaged
	LOG_VEHICLE_REVERSE_DISENGAGED // Vehicle reverse gear disengaged
};

