/*
 * StepperMotor.cpp
 *
 *  Created on: May 16, 2025
 *      Author: To
 */

#include "StepperMotor.h"

StepperMotor::StepperMotor(bool motPosition, uint32_t stepTime, int32_t lowLimit, int32_t highLimit, GPIO_TypeDef * stepPort, uint32_t stepPin, GPIO_TypeDef * dirPort, uint32_t dirPin)
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

	mLowLimit = lowLimit;
	mHighLimit = highLimit;

	mPulseState = true;
	mReady = false;
	mResetPosition = true;
	mResetSequence = true;
	mMotPosition = motPosition;

	mTickCount = 0;
	mStepTime = stepTime;
	mStepTimeResetSequence = stepTime;

	mRunSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(mRunSemaphore);
}

void StepperMotor::setTargetPosition(int32_t position)
{
	if (xSemaphoreTake(mRunSemaphore, 100) == pdTRUE)
	{
		// Constrain between limit
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
		if(mSensor.enabled)
		{
			float a = 10000.0 / (mSensor.max - mSensor.min);

			mRealPosition = a*(-position + mSensor.center); //Add
		}
		xSemaphoreGive(mRunSemaphore);
	}
}

void StepperMotor::setInitialPosition(int32_t initial)
{
	if (xSemaphoreTake(mRunSemaphore, 100) == pdTRUE)
	{
		// Constrain between limit
		if (initial < mLowLimit)
			mPosition = mLowLimit;
		else if (initial > mHighLimit)
			mPosition = mHighLimit;
		else
			mPosition = initial;

		xSemaphoreGive(mRunSemaphore);
	}
}

void StepperMotor::configureSensor(const RotationSensor sensor)
{
	if (xSemaphoreTake(mRunSemaphore, 100) == pdTRUE)
	{
		mSensor = sensor;
		mSensor.enabled = true;
		xSemaphoreGive(mRunSemaphore);
	}
}

void StepperMotor::run()
{
	if(mResetSequence && mSensor.enabled)
	{
		mPosition = mRealPosition;
		mStepTime = 10;
		if(mRealPosition > mTargetPosition -7 && mRealPosition < mTargetPosition +7)
		{
			mResetSequence = false;
			mStepTime = mStepTimeResetSequence;
		}
	}

	else
	{
		mResetSequence = false;
		mStepTime = mStepTimeResetSequence;
	}

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if(xSemaphoreTakeFromISR(mRunSemaphore, &xHigherPriorityTaskWoken) == pdTRUE)
	{
		mTickCount++;

		if(mTickCount >= mStepTime)
		{
			mTickCount = 0;
			if (mPosition != mTargetPosition)
			{
				//Handle Pulse
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

