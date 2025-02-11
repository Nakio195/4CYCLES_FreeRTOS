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

#include <string>

class VehicleTask : public RTOS_Task
{
	public:
		VehicleTask();

		void setup() override;
		void run() override;
		void cleanup() override;

	private:
		PS3Controller mController;
};

extern VehicleTask Vehicle;

#endif /* SRC_TASKS_VEHICLETASK_H_ */
