/*
 * RedLEDTask.h
 *
 *  Created on: Jan 21, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_REDLEDTASK_H_
#define SRC_TASKS_REDLEDTASK_H_

#include "RTOSTask.h"
#include "gpio.h"

class RedLED_Task: public RTOS_Task
{
	public:
		RedLED_Task();

		void setup() override;
		void run() override;
};

#endif /* SRC_TASKS_REDLEDTASK_H_ */
