#pragma once

#include <stdint.h>
#include <string>
#include "ArduinoJson-v7.3.0.h"

#include "FreeRTOS.h"
#include "task.h"

class Message
{
	public:
		enum Type{LogDebug, LogInfo, LogWarning, LogError, LogCritical, ThrottleOut, Wheel, Direction};

		Message(Type type);

		Type type();
		Type level();
		std::string levelToString();

		std::string message();
		uint32_t timestamp();

		Message& operator<<(Type type);
		Message& operator<<(std::string text);
		Message& operator<<(uint32_t number);
		Message& operator<<(std::initializer_list<std::pair<const char*, JsonVariant>> values);


	protected:
		Type mType;
		uint32_t mTimestamp;
		std::string mMessage;
		JsonObject mObject;
};

