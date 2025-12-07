#pragma once
#include <string>
#include <vector>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "cmsis_os2.h"
#include "usbd_cdc_if.h"
#include "usart.h"

#include "RTOSTask.h"
#include "CANTask.h"
#include "utils/CanPeripheral.h"
#include "utils/LockGuard.hpp"

class Logger : public RTOS_Task, public CanPeripheral
{
	public:
		Logger();

		void print(Message &m);

		QueueHandle_t createLogQueue();

		void setup() override;
		void run() override;
		void cleanup() override;

		void init() override;
		void discovered();
		void reInit();
		void recovery();
		void absent();
		void recovered();
		void lost();

	private:
		std::vector<QueueHandle_t> mQueues;
		SemaphoreHandle_t mutex;

		Message::Type mLogLevel = Message::LogDebug;
		uint32_t mPreviousTick	 = 0;
};

extern Logger LoggerTask;
