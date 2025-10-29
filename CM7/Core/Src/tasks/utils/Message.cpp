#include "Message.h"

Message::Message(Type type)
{
	mType = type;
	mTimestamp = xTaskGetTickCount();
}

Message::Type Message::level()
{
	return mType;
}

/*
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
}*/

uint32_t Message::code()
{
	return mMessageCode;
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

Message& Message::operator<<(uint32_t number)
{
	mMessageCode = number;
	return *this;
}

//Message& Message::operator<<(std::initializer_list<std::pair<const char*, JsonVariant>> values)
//{
////	for (const auto& pair : values)
////	{
////		mObject.[pair.first] = pair.second;
////	}
////	return *this;
//}


