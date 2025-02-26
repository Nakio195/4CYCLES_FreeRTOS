#pragma once

#include <stdint.h>
#include <string>
#include "ArduinoJson-v7.3.0.h"

class Message
{
	public:
		enum Type{LogDebug, LogInfo, LogWarning, LogError, LogCritical, Controller, Wheel, Direction};

		Message(Type type);

		Type type();
		Type level();
		std::string levelToString();

		std::string message();
		uint32_t code();

		Message& operator<<(Type type);
		Message& operator<<(std::string text);
		Message& operator<<(uint32_t number);
		Message& operator<<(std::initializer_list<std::pair<const char*, JsonVariant>> values);


	protected:
		Type mType;
		uint32_t mCode;
		std::string mMessage;
		JsonObject mObject;
};

