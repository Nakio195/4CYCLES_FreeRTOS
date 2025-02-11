#pragma once
#include "utils/Logger.h"
#include <string>
#include <vector>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "cmsis_os2.h"
#include "usbd_cdc_if.h"

#include "RTOSTask.h"
#include "utils/LockGuard.hpp"
#include "utils/ArduinoJson-v7.3.0.h"

class JsonLogger : public Logger, public RTOS_Task
{
	public:
		JsonLogger();

		void print(Message &m) override;

		QueueHandle_t createLogQueue();

		void setup() override;
		void run() override;
		void cleanup() override;

	private:
		JsonDocument mDocument;
		std::vector<QueueHandle_t> mQueues;
		SemaphoreHandle_t mutex;
};

extern JsonLogger Json;
