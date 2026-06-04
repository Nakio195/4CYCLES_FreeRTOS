#pragma once

#include <stdint.h>
#include <string>
//#include "ArduinoJson-v7.3.0.h"

#include "FreeRTOS.h"
#include "task.h"

class Message
{

	public:
		enum Type{LogDebug, LogInfo, LogWarning, LogError, LogCritical, Controller, Motor, Electrics};

		Message(Type type, uint32_t messageCode = 0);

		Type type();
		Type level();

		uint32_t code();
		uint32_t timestamp();

		Message& operator<<(uint8_t number);
		Message& operator<<(uint16_t number);
		Message& operator<<(uint32_t number);

	    Message& operator<<(int32_t v)  { return *this << static_cast<uint32_t>(v); }
		//Message& operator<<(std::initializer_list<std::pair<const char*, JsonVariant>> values);

		uint32_t mTimestamp;
		uint32_t mMessageCode;
		uint8_t mData[8] = {0};

	protected:
		Type mType;
		uint8_t mDataPointer;

		Message& pushData(const uint32_t &data, const uint8_t length);
};

enum {
	// CAN Peripherals
	NONE = 0,
	LOG_PS3_CONTROLLER_ABSENT = 1, //PS3 Controller absent from bus
	LOG_PS3_CONTROLLER_DISCOVERED, //PS3 Controller connected to bus
	LOG_PS3_CONTROLLER_CONNECTED, //PS3 Controller connected to bus
	LOG_PS3_CONTROLLER_RECOVERY_ATTEMPT, //PS3 Controller recovery attempt
	LOG_PS3_CONTROLLER_LOST, //Logger lost
	LOG_PS3_CONTROLLER_RECOVERED, //Logger recovered
	LOG_PS3_CONTROLLER_DISABLED, // HandleBar disabled
	LOG_PS3_CONTROLLER_READY, // HandleBar enabled

	LOG_LOGGER_ABSENT, //Logger absent from bus
	LOG_LOGGER_CONNECTED, //Logger connected to bus
	LOG_LOGGER_DISCOVERED, //Logger connected to bus
	LOG_LOGGER_RECOVERY_ATTEMPT, //Logger recovery attempt
	LOG_LOGGER_LOST, //Logger lost
	LOG_LOGGER_RECOVERED, //Logger recovered
	LOG_LOGGER_DISABLED, // HandleBar disabled
	LOG_LOGGER_READY, // HandleBar enabled

	LOG_HANDLEBAR_ABSENT, //HandleBar absent from bus
	LOG_HANDLEBAR_CONNECTED, //HandleBar connected to bus
	LOG_HANDLEBAR_DISCOVERED, //HandleBar connected to bus
	LOG_HANDLEBAR_RECOVERY_ATTEMPT, //HandleBar recovery attempt
	LOG_HANDLEBAR_LOST, //HandleBar lost
	LOG_HANDLEBAR_RECOVERED, //HandleBar recovered
	LOG_HANDLEBAR_DISABLED, // HandleBar disabled
	LOG_HANDLEBAR_READY, // HandleBar disabled


	LOG_BATTERY_ABSENT, //BATTERY absent from bus
	LOG_BATTERY_CONNECTED, //BATTERY connected to bus
	LOG_BATTERY_DISCOVERED, //BATTERY connected to bus
	LOG_BATTERY_RECOVERY_ATTEMPT, //BATTERY recovery attempt
	LOG_BATTERY_LOST, //BATTERY lost
	LOG_BATTERY_RECOVERED, //BATTERY recovered
	LOG_BATTERY_DISABLED, // BATTERY disabled
	LOG_BATTERY_READY, // BATTERY disabled

	// Motor Status
	LOG_VEHICLE_MOTOR_ENGAGED, // Vehicle motor engaged
	LOG_VEHICLE_MOTOR_DISENGAGED, // Vehicle motor disengaged
	LOG_VEHICLE_REVERSE_ENGAGED, // Vehicle reverse gear engaged
	LOG_VEHICLE_REVERSE_DISENGAGED, // Vehicle reverse gear disengaged
	LOG_VEHICLE_MOTOR_FAULTS_DETECTED, // Vehicle motor faults detected
	LOG_VEHICLE_CRITICAL_STATE // Entering Critical state
};

