#pragma once

#include "Message.h"
#include <deque>

class Logger
{
	public:
		Logger();
		void setLogLevel(Message::Level level);

		// Override this function to implement specific IO hardware
		virtual void print(Message &m);


	protected:
		Message::Level mLogLevel;

};

