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
#include "DirectionTask.h"
#include "usbd_cdc_if.h"
#include "usart.h"

#include "FreeRTOS.h"
#include "timers.h"

#include "utils/filters/FilterChain.h"
#include "utils/filters/LowPassFilter.h"
#include "utils/filters/SCurveFilter.h"
#include "utils/filters/ThresholdFilter.h"
#include "utils/Phaserunner.hpp"

#include <string>

class VehicleTask : public RTOS_Task
{
	public:
		VehicleTask();

		void setup() override;
		void run() override;
		void cleanup() override;

		void handleLightsAction(Action* action);

		void engageMotor();
		void disengageMotor();

		void setMotorSpeed(float speed, bool reverse = false);
	private:
		QueueHandle_t mControllerQueue;

		bool mMotorEngaged; // true if motor is engaged

		//Motor Reverse
		bool mMotorReversePending; // true if reverse gear is pending engagement
		bool mMotorReversePendingValue; // true if reverse gear is pending value change
		bool mMotorReverseEngaged; // true if reverse gear is engaged
		// Filters
		FilterChain mThrottle;
		FilterChain mBrake;

		//Motor Zero crossing detection
		bool mZeroCrossing;

};

extern VehicleTask Vehicle;

#endif /* SRC_TASKS_VEHICLETASK_H_ */
