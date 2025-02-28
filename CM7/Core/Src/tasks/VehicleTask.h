/*
 * VehicleTask.h
 *
 *  Created on: Feb 3, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_VEHICLETASK_H_
#define SRC_TASKS_VEHICLETASK_H_

#include "RTOSTask.h"
#include "LoggerTask.h"
#include "PS3Controller.h"
#include "usbd_cdc_if.h"

#include "FreeRTOS.h"
#include "timers.h"

#include "utils/filters/FilterChain.h"
#include "utils/filters/LowPassFilter.h"
#include "utils/filters/SCurveFilter.h"
#include "utils/filters/ThresholdFilter.h"

#include <string>

class VehicleTask : public RTOS_Task
{
	public:
		VehicleTask();

		void setup() override;
		void run() override;
		void cleanup() override;

	private:
		QueueHandle_t mControllerQueue;

		// Filters
		FilterChain mThrottle;

};

extern VehicleTask Vehicle;

#endif /* SRC_TASKS_VEHICLETASK_H_ */
