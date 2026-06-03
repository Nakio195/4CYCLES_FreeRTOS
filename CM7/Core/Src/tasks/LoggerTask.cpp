#include "LoggerTask.h"

Logger LoggerTask;


Logger::Logger()
{
	mutex = xSemaphoreCreateMutex();

	setRangeFilter(0x1000, 0x1019);
	setCommunicationTimeout(5000);
	setRecoveryMode(5, 500);

	mPreviousTick = 0;
	mPeripheralId = MainLogger;
	mPeripheralType = PeripheralType::Logger;


	CanHandler.attach(this);
}

QueueHandle_t Logger::createLogQueue(const char *name)
{
	LockGuard lock(mutex);
	QueueHandle_t q = xQueueCreate(20, sizeof(Message*));

	vQueueAddToRegistry(q, name);

	if(q == nullptr)
		Error_Handler();

	mQueues.push_back(q);
	return q;
}

void Logger::setup()
{
	osDelay(1000);
}

void Logger::run()
{

	CanPacket* packet = nullptr;
	if(xQueueReceive(mPacketsQueue, &packet, 0) == pdTRUE)
	{
		if(packet != nullptr)
		{
			if(packet->Identifier == 0x1018)
			{
				uint32_t hb = packet->data[0] | (packet->data[1] << 8) | (packet->data[2] << 16) | (packet->data[3] << 24);
				heartbeat(hb);
			}

			CanPacketPool.free(packet);
		}
	}

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
						if((uint32_t)(m->level()) >= mLogLevel)
							print(*m);
						delete m; // TODO : Use Messages Pools
					}
				}
			}
		}
	}

	osDelay(7);

	mPreviousTick = xTaskGetTickCount();
	tick(xTaskGetTickCount());
}

void Logger::cleanup()
{

}

void Logger::onInit()
{
	CanPacket *LoggerControl = CanPacketPool.allocate(0x1019);
	LoggerControl->data.push_back(0x01);
	if(!CanHandler.send(LoggerControl)) //TODO Handle multiple failed init
		CanPacketPool.free(LoggerControl);

}

void Logger::onDiscovered()
{
	log(Message(Message::LogError, LOG_LOGGER_CONNECTED));
}

void Logger::onRecovery()
{
	log(Message(Message::LogError, LOG_LOGGER_RECOVERY_ATTEMPT));
}

void Logger::onAbsent()
{
	log(Message(Message::LogError, LOG_LOGGER_ABSENT));
}

void Logger::onRecovered()
{
	log(Message(Message::LogInfo, LOG_LOGGER_RECOVERED));
}

void Logger::onLost()
{
	log(Message(Message::LogInfo, LOG_LOGGER_LOST));
}

void Logger::onDisabled()
{
	log(Message(Message::LogInfo, LOG_LOGGER_LOST));
}

void Logger::print(Message& m)
{

	uint8_t level = 0;
	uint32_t id = 0x1000;
	if (m.level() == Message::LogCritical)
		level = 1;
	else if (m.level() == Message::LogError)
		level = 2;
	else if (m.level() == Message::LogWarning)
		level = 3;
	else if (m.level() == Message::LogInfo)
		level = 4;
	else if (m.level() == Message::LogDebug)
		level = 5;
	else if (m.level() == Message::Controller)
		id = 0x1005;
	else if (m.level() == Message::Electrics)
		id = 0x1006;
	else if (m.level() == Message::Motor)
		id = 0x1010+m.mData[0]-1;

	CanPacket *Log = CanPacketPool.allocate(id);

	if (m.level() == Message::Controller)
	{
		Log->data.push_back(m.mData[0]);
		Log->data.push_back(m.mData[1]);
		Log->data.push_back(m.mData[2]);
		Log->data.push_back(m.mData[3]);
	}
	else if (m.level() == Message::Motor)
	{
		Log->data.push_back(m.mData[1]);
		Log->data.push_back(m.mData[2]);
		Log->data.push_back(m.mData[3]);
		Log->data.push_back(m.mData[4]);
		Log->data.push_back(m.mData[5]);
		Log->data.push_back(m.mData[6]);
	}

	else
	{
		Log->data.push_back(level);
		Log->data.push_back(m.code() >> 16);
		Log->data.push_back(m.code() >> 8);
		Log->data.push_back(m.code() & 0xFF);
	}

	if(!CanHandler.send(Log))
	{
		// TODO handle full Queue
		CanPacketPool.free(Log);
	}
}

