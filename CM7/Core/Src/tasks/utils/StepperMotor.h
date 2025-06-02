/*
 * StepperMotor.h
 *
 *  Created on: May 16, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_STEPPERMOTOR_H_
#define SRC_TASKS_UTILS_STEPPERMOTOR_H_

#include "stdint.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

#define STEPPER_MOTOR_STEP_TIME 2 //us

class StepperMotor
{
	public:
		enum Direction{Forward = 0, Backward = 1};
	public:
		StepperMotor(bool motPosition, GPIO_TypeDef * stepPort, uint32_t stepPin, GPIO_TypeDef * dirPort, uint32_t dirPin);

		void setTargetPosition(int32_t position);
		void setRealPosition(int32_t position);
		void run();

	private:
		void setDirection(bool direction);


	private:
		bool mMotPosition; // true = Front Motor

		int32_t mPosition;
		int32_t mTargetPosition;
		int32_t mRealPosition;

		int32_t mLowLimit;
		int32_t mHighLimit;

		bool mReady;
		bool mResetPosition;
		bool mResetSequence;
		bool mPulseState;
		GPIO_TypeDef * mStepPort;
		uint32_t mStepPin;
		bool mDirection;
		GPIO_TypeDef * mDirPort;
		uint32_t mDirPin;

		uint32_t mTickCount;
		SemaphoreHandle_t mRunSemaphore;
};

#endif /* SRC_TASKS_UTILS_STEPPERMOTOR_H_ */
