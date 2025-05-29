/*
 * Switch.cpp
 *
 *  Created on: 13 avr. 2022
 *      Author: TooT
 */

#include "Switch.h"


Switch::Switch()
{
	mCurrentState = RELEASED;
	mPreviousState = RELEASED;
	mNewState = IDLE;

	mDebounce = false;
	mDebounceTime = 100;
	mDebounceTimer = 0;

	mRepeatTime = 100;
	mRepeatDelay = 0;
	mRepeatTimer = 0;
}


void Switch::tick(uint16_t dt)
{
	if(mDebounce)
	{
		mDebounceTimer += dt;

		if(mDebounceTimer >= mDebounceTime)
		{
			mDebounce = false;
			mDebounceTimer = 0;
		}

		else
		{
			return;
		}
	}

	if(mCurrentState == PRESSED && mPreviousState == PRESSED && mRepeatDelay !=0)
	{
		mRepeatTimer += dt;
	}

	if(mCurrentState == PRESSED && mPreviousState == REPEAT)
	{
		mRepeatTimer += dt;
	}
}

void Switch::update(bool state)
{

	if (mDebounce)
		return;

	if(state == false)
		mCurrentState = RELEASED;
	if(state == true)
		mCurrentState = PRESSED;
	//					_
	//Detect a release   |_
	if(mCurrentState == RELEASED && mPreviousState == PRESSED)
	{
		mNewState = RELEASED;
		mPreviousState = RELEASED;
		mDebounce = true;
	}

//						 _
	//Detect a Press   _|
	else if(mCurrentState == PRESSED && mPreviousState == RELEASED)
	{
		mNewState = PRESSED;
		mPreviousState = PRESSED;
		mDebounce = true;
	}

	//                    				     ___*__
	// Detect a start of repeat condition
	else if(mCurrentState == PRESSED && mPreviousState == PRESSED && mRepeatDelay != 0)
	{
		mNewState = REPEAT;
		if(mRepeatTimer >= mRepeatDelay)
		{
			mPreviousState = REPEAT;
			mRepeatTimer = 0;
		}
	}

	// Detect a repeat condition
	else if(mCurrentState == PRESSED && mPreviousState == REPEAT)
	{
		mNewState = REPEAT;
		if(mRepeatTimer >= mRepeatTime)
		{
			mRepeatTimer = 0;
		}
	}

	//Detect a end of repeat condition
	else if(mCurrentState == RELEASED && mPreviousState == REPEAT)
	{
		mNewState = RELEASED;
		mRepeatTimer = 0;
		mPreviousState = RELEASED;
		mDebounce = true;
	}

}

Switch::States Switch::read()
{
	States s = mNewState;
	mNewState = IDLE;
	return s;
}

void Switch::setRepeat(uint16_t delay)
{
	mRepeatDelay = delay;
}

void Switch::setDebounce(uint16_t debounce)
{
	mDebounceTime = debounce;
}

void Switch::setRepeatTime(uint16_t time)
{
	mRepeatTime = time;
}

