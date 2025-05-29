/*
 * DirectionTask.h
 *
 *  Created on: May 16, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_DIRECTIONTASK_H_
#define SRC_TASKS_DIRECTIONTASK_H_

#include <stdint.h>
#include "RTOSTask.h"
#include "spi.h"
#include "gpio.h"
#include "cmsis_os.h"
#include "utils/StepperMotor.h"
#include "tim.h"

class DirectionTask: public RTOS_Task
{
	public:
		DirectionTask();
		void setup() override;
		void run() override;
		void cleanup() override;

		void setDirectionAV(int32_t target);
		void setDirectionAR(int32_t target);

		void tick(uint8_t dt)
		{
			mMotorAV->run();
			mMotorAR->run();
		}

	private:
		uint8_t mDirSensorAV;
		uint8_t mDirSensorAR;

		StepperMotor *mMotorAV;
		StepperMotor *mMotorAR;

};

extern DirectionTask DirectionHandler;

#endif /* SRC_TASKS_DIRECTIONTASK_H_ */
