#include "Logger.h"

Logger::Logger()
{
	mLogLevel = Message::Error;
}

void Logger::setLogLevel(Message::Level level)
{
	mLogLevel = level;
}

void Logger::print(Message &m)
{
	
	std::string out;

	out += m.levelToString();
	if (m.code() != 0)
	{
		out += " - ";
		out += std::to_string(m.code());
	}

	if (!m.message().empty())
	{
		out += " : ";
		out += m.message();
	}

	out += "\n";
	//std::cout << out;
}
