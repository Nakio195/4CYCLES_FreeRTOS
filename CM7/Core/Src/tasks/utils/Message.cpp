#include "Message.h"

Message::Message(Type type, uint32_t messageCode)
{
	mType = type;
	mMessageCode = messageCode;
	mTimestamp = xTaskGetTickCount();
	mDataPointer = 0;
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

Message& Message::pushData(const uint32_t& data, const uint8_t length)
{
    if (mDataPointer + length > 8)
        return *this;

    switch (length)
    {
        case 1:
            mData[mDataPointer] = static_cast<uint8_t>(data);
            break;

        case 2:
            mData[mDataPointer]     = static_cast<uint8_t>((data >> 8) & 0xFF);
            mData[mDataPointer + 1] = static_cast<uint8_t>( data        & 0xFF);
            break;

        case 4:
            mData[mDataPointer]     = static_cast<uint8_t>((data >> 24) & 0xFF);
            mData[mDataPointer + 1] = static_cast<uint8_t>((data >> 16) & 0xFF);
            mData[mDataPointer + 2] = static_cast<uint8_t>((data >> 8 ) & 0xFF);
            mData[mDataPointer + 3] = static_cast<uint8_t>( data        & 0xFF);
            break;

        default:
            return *this; // invalid size, ignore
    }

    mDataPointer += length;
    return *this;
}



Message& Message::operator<<(uint8_t number)
{
	return pushData(number, sizeof(number));
}

Message& Message::operator<<(uint16_t number)
{
	return pushData(number, sizeof(number));
}

Message& Message::operator<<(uint32_t number)
{
	return pushData(number, sizeof(number));
}

//Message& Message::operator<<(std::initializer_list<std::pair<const char*, JsonVariant>> values)
//{
////	for (const auto& pair : values)
////	{
////		mObject.[pair.first] = pair.second;
////	}
////	return *this;
//}


