/*
 * RTOSTask.h
 *
 *  Created on: Jan 21, 2025
 *      Author: To
 */

#ifndef SRC_UTILS_RTOSTASK_H_
#define SRC_UTILS_RTOSTASK_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "utils/Message.h"
#include "utils/Event.h"
#include "utils/Controller.h"
//Toto
class RTOS_Task
{
	public:
		virtual ~RTOS_Task() {}

		inline bool start(const char * const pcName, const uint16_t usStackDepth, UBaseType_t uxPriority)
		{
			this->stopCalled = false;
			return xTaskCreate(RTOS_Task::bootstrap, pcName, usStackDepth, this, uxPriority, &this->xHandle);
		}

		inline void stop()
		{
			this->stopCalled = true;
		}

		void inline attachLogQueue(QueueHandle_t q)
		{
			if(q != nullptr)
				mLogQueue = q;
		}

		void inline attachEventQueue(QueueHandle_t q)
		{
			if(q != nullptr)
				mEventQueue = q;
		}

		QueueHandle_t inline getEventQueue()
		{
			if(mEventQueue != nullptr)
				return mEventQueue;

			return nullptr;
		}

		void inline log(const Message &m)
		{
			Message* p = new Message(m); // TODO Use Message pool
			if(mLogQueue != nullptr)
				xQueueSend(mLogQueue, &p, 100);
		}

		void inline emit(const Event e)
		{
			if(mEventQueue != nullptr)
				xQueueSend(mEventQueue, &e, 100);
		}

	protected:
		inline void sleep(int time_ms)
		{
			vTaskDelay(time_ms/portTICK_PERIOD_MS);
		}

		inline void suspend()
		{
			if(xHandle != 0)
				vTaskSuspend(xHandle);
		}

		inline void resume()
		{
			if(xHandle != 0)
			{
				vTaskResume(xHandle);
				taskYIELD();
			}
		}

	protected:
		virtual void setup() {}
		virtual void run() {}
		virtual void cleanup() {}

		static void bootstrap(void* pvParameters)
		{
			RTOS_Task* taskObject = reinterpret_cast<RTOS_Task*>(pvParameters);
			taskObject->setup();
			while (!taskObject->stopCalled)
			{
				taskObject->run();
			}
			// task clean up
			taskObject->cleanup();
			vTaskDelete(taskObject->xHandle);
		}

		bool stopCalled = false;
		TaskHandle_t xHandle = 0;
		QueueHandle_t mLogQueue = nullptr;
		QueueHandle_t mEventQueue = nullptr;
};

#endif /* SRC_UTILS_RTOSTASK_H_ */
