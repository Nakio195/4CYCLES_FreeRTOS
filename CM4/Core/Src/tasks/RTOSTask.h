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

	protected:
	  inline void sleep(int time_ms)
	  {
	    vTaskDelay(time_ms/portTICK_PERIOD_MS);
	  }

	private:
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
};

#endif /* SRC_UTILS_RTOSTASK_H_ */
