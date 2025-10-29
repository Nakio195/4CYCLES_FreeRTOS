#include "LoggerTask.h"

Logger LoggerTask;


Logger::Logger()
{
	mutex = xSemaphoreCreateMutex();

	setRangeFilter(0x1000, 0x1009);
	setCommunicationTimeout(3000);
	setRecoveryMode(50, 1000);

	mPreviousTick = 0;

	CanHandler.attach(this);
}

QueueHandle_t Logger::createLogQueue()
{
	LockGuard lock(mutex);
	QueueHandle_t q = xQueueCreate(20, sizeof(Message*));

	vQueueAddToRegistry(q, "LoggerQueue");

	if(q == nullptr)
		Error_Handler();

	mQueues.push_back(q);
	return q;
}

void Logger::setup()
{
	osDelay(1000);
	init();
}

void Logger::run()
{

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
						delete m;
					}
				}
			}
		}
	}

	osDelay(7);

	uint32_t dt = xTaskGetTickCount() - mPreviousTick;
	mPreviousTick = xTaskGetTickCount();
	tick(xTaskGetTickCount());
}

void Logger::cleanup()
{

}

void Logger::init()
{
	CanPacket *LoggerControl = CanPacketPool.allocate(0x1019);
	LoggerControl->data.push_back(0x01);
	if(CanHandler.send(LoggerControl)) //TODO Handle multiple failed init
	{
		CanPeripheral::init();
	}
}

void Logger::reInit()
{
	CanPacket *LoggerControl = CanPacketPool.allocate(0x1019);
	LoggerControl->data.push_back(0x01);
	LoggerControl->data.push_back(0x00);
	LoggerControl->data.push_back(0x00);
	if(CanHandler.send(LoggerControl))
	{
		// Todo handle full Queue
	}
}

void Logger::recovery()
{

}

void Logger::absent()
{

}

void Logger::recovered()
{

}

void Logger::lost()
{

}

void Logger::print(Message& m)
{
	uint32_t id = 0x1000;
	if (m.level() == Message::LogCritical)
		id = 0x1000;
	else if (m.level() == Message::LogError)
		id = 0x1001;
	else if (m.level() == Message::LogWarning)
		id = 0x1002;
	else if (m.level() == Message::LogInfo)
		id = 0x1003;
	else if (m.level() == Message::LogDebug)
		id = 0x1004;
	else if (m.level() == Message::Dynamics)
		id = 0x1005;
	else if (m.level() == Message::Electrics)
		id = 0x1006;

	CanPacket *Log = CanPacketPool.allocate(id);
	Log->data.push_back(m.timestamp() >> 24);
	Log->data.push_back(m.timestamp() >> 16);
	Log->data.push_back(m.timestamp() >> 8);
	Log->data.push_back(m.timestamp() & 0xFF);

	if (m.level() == Message::Dynamics)
	{
		Log->data.push_back(m.mDynamicsData.speed);
		Log->data.push_back(m.mDynamicsData.throttle);
		Log->data.push_back(m.mDynamicsData.brake);
	}
	else if (m.level() == Message::Electrics)
	{
		// TODO add electrics data
	}

	else
	{
		Log->data.push_back(m.code() >> 24);
		Log->data.push_back(m.code() >> 16);
		Log->data.push_back(m.code() >> 8);
		Log->data.push_back(m.code() & 0xFF);
	}

	if(CanHandler.send(Log))
	{
		// TODO handle full Queue
	}
}

