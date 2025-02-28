#include "Message.h"

Message::Message(Type type)
{
	mType = type;
	mTimestamp = xTaskGetTickCount();
	mMessage = "";
}

Message::Type Message::type()
{
	return mType;
}

std::string Message::levelToString()
{
	switch (mType)
	{
		case LogInfo:
			return std::string("[INFO]");
		case LogWarning:
			return std::string("[WARN]");
		case LogError:
			return std::string("[ERROR]");
		case LogCritical:
			return std::string("[CRITICAL]");
		case LogDebug:
			return std::string("[DEBUG]");
		default:
			return std::string("[UNKNOWN]");
	}
}

std::string Message::message()
{
	return mMessage;
}

uint32_t Message::timestamp()
{
	return mTimestamp;
}

Message& Message::operator<<(Type type)
{
	mType = type;
	return *this;
}


Message& Message::operator<<(std::string text)
{
	mMessage += text;
	return *this;
}

Message& Message::operator<<(uint32_t number)
{
	mMessage += std::to_string(number);
	return *this;
}

Message& Message::operator<<(std::initializer_list<std::pair<const char*, JsonVariant>> values)
{
//	for (const auto& pair : values)
//	{
//		mObject.[pair.first] = pair.second;
//	}
//	return *this;
}


