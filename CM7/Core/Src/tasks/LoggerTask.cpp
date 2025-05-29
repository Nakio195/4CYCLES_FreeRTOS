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

	vQueueAddToRegistry(q, "LoggerQueue");

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
	volatile size_t freeHeap = 0;
	volatile size_t minEverHeap = 0;

	freeHeap = xPortGetFreeHeapSize();
	minEverHeap = xPortGetMinimumEverFreeHeapSize();

	{LockGuard lock(mutex);
		for(auto queue : mQueues)
		{
			while (uxQueueMessagesWaiting(queue) > 0)
			{
				// Read message from queue
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
	//HAL_UART_Transmit_IT(&huart5, (uint8_t*)out.data(), out.size());
}

