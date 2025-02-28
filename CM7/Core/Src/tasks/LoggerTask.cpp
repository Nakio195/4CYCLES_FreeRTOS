#include "LoggerTask.h"

JsonLogger Json;


JsonLogger::JsonLogger()
{
	mutex = xSemaphoreCreateMutex();
}

QueueHandle_t JsonLogger::createLogQueue()
{
	LockGuard lock(mutex);
	QueueHandle_t q = xQueueCreate(20, sizeof(Message*));
	if(q == nullptr)
		Error_Handler();

	mQueues.push_back(q);
	return q;
}

void JsonLogger::setup()
{

	osDelay(3000); //Wait for USB CDC to init and connect
}

void JsonLogger::run()
{

	{LockGuard lock(mutex);
		for(auto queue : mQueues)
		{
			Message *m = nullptr;
			if(xQueueReceive(queue, &m, 0) == pdTRUE)
			{
				if(m != nullptr)
				{
					if((uint32_t)(m->type()) >= mLogLevel)
						print(*m);
					delete m;
				}
			}
		}
	}

	osDelay(7);

}

void JsonLogger::cleanup()
{

}

void JsonLogger::print(Message& m)
{
	mDocument.clear();

	mDocument["msgType"] = m.type();
	if(m.timestamp() != 0)
		mDocument["timestamp"] = m.timestamp();

	if(m.type() <= Message::LogCritical)
	{
		mDocument["message"] = m.message();
	}

	else if(m.type() == Message::ThrottleOut)
	{
		mDocument["message"] = std::stoi(m.message());
	}

	else if(m.type() == Message::Wheel)
	{
		mDocument["message"] = m.message();
	}

	else if(m.type() == Message::Direction)
	{
		mDocument["message"] = m.message();
	}

	std::string out;
	serializeJson(mDocument, out);

	out += "\n";

	CDC_Transmit_FS((uint8_t*)out.data(), out.size());
}

