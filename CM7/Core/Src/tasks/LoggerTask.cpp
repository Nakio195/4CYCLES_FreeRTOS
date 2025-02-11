#include "LoggerTask.h"

JsonLogger Json;


JsonLogger::JsonLogger() : Logger()
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
	setLogLevel(Message::Debug);
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
					if((uint32_t)(m->level()) >= mLogLevel)
						print(*m);
					delete m;
				}
			}
		}
	}

	osDelay(50);

}

void JsonLogger::cleanup()
{

}

void JsonLogger::print(Message& m)
{
	mDocument.clear();

	mDocument["level"] = m.level();
	if(m.code() != 0)
		mDocument["code"] = m.code();
	mDocument["message"] = m.message();

	std::string out;
	serializeJson(mDocument, out);

	CDC_Transmit_FS((uint8_t*)out.data(), out.size());
}

