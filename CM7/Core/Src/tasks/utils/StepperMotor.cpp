/*
 * StepperMotor.cpp
 *
 *  Created on: May 16, 2025
 *      Author: To
 */

#include "StepperMotor.h"

StepperMotor::StepperMotor(bool motPosition, uint32_t stepTime, GPIO_TypeDef * stepPort, uint32_t stepPin, GPIO_TypeDef * dirPort, uint32_t dirPin)
{
	// TODO Auto-generated constructor stub
	mStepPort = stepPort;
	mStepPin = stepPin;
	mDirPort = dirPort;
	mDirPin = dirPin;

	HAL_GPIO_WritePin(mStepPort, mStepPin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(mDirPort, mDirPin, GPIO_PIN_RESET);

	mPosition = 0;
	mRealPosition = 9999;
	mTargetPosition = 0;
	mDirection = 0;

	mLowLimit = -5000;
	mHighLimit = 5000;

	mPulseState = true;
	mReady = false;
	mResetPosition = true;
	mResetSequence = false;
	mMotPosition = motPosition;

	mTickCount = 0;
	mStepTime = stepTime;

	mRunSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(mRunSemaphore);
}

void StepperMotor::setTargetPosition(int32_t position)
{
	if (xSemaphoreTake(mRunSemaphore, 100) == pdTRUE)
	{
		if (position < mLowLimit)
			mTargetPosition = mLowLimit;
		else if (position > mHighLimit)
			mTargetPosition = mHighLimit;
		else
			mTargetPosition = position;

		xSemaphoreGive(mRunSemaphore);
	}
}

void StepperMotor::setRealPosition(int32_t position)
{
	if (xSemaphoreTake(mRunSemaphore, 100) == pdTRUE)
	{
		mRealPosition = -position;
		xSemaphoreGive(mRunSemaphore);
	}
}
void StepperMotor::run()
{
	/*if(mResetSequence)
	{
		if(mMotPosition)
			mPosition = mRealPosition;
		else
			mPosition = -mRealPosition;

		if(mRealPosition > -500 && mRealPosition < 500)
			mResetSequence = false;
	}


	if(!mReady)
	{
		if(mTargetPosition < mRealPosition +500 && mTargetPosition > mRealPosition-500)
		{
			mReady = true;
			mResetPosition = true;
			mResetSequence = true;
		}
		else
			return;
	}*/

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if(xSemaphoreTakeFromISR(mRunSemaphore, &xHigherPriorityTaskWoken) == pdTRUE)
	{
		mTickCount++;

		if(mTickCount >= mStepTime)
		{
			mTickCount = 0;
			if (mPosition != mTargetPosition)
			{
				if (mPosition < mTargetPosition)
					HAL_GPIO_WritePin(mDirPort, mDirPin, GPIO_PIN_RESET);
				else
					HAL_GPIO_WritePin(mDirPort, mDirPin, GPIO_PIN_SET);

				if(mPulseState)
				{
					HAL_GPIO_WritePin(mStepPort, mStepPin, GPIO_PIN_SET);
					if(mPosition < mTargetPosition)
						mPosition++;
					else
						mPosition--;
				}
				else
					HAL_GPIO_WritePin(mStepPort, mStepPin, GPIO_PIN_RESET);

				mPulseState = !(bool)(mPulseState);

			}
		}

		xSemaphoreGiveFromISR(mRunSemaphore, &xHigherPriorityTaskWoken);
	}

	if (xHigherPriorityTaskWoken == pdTRUE)
	{
		//portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

