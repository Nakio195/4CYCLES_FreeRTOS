/*
 * Timer.cpp
 *
 *  Created on: 28 d�c. 2022
 *      Author: TooT
 */

#include "Timer.hpp"


Timer::Timer(uint16_t pPeriod, unsigned int pMode)
{
    mPeriod = pPeriod;
    mMode = pMode;

    mCounter = 0;

    mRunning = false;
    mTrigger = false;
    mCounts = 0;

    mToggled = false;

    onTriggerAction = nullptr;
}


bool Timer::isRunning() const
{
    if(mCounter <= mPeriod && mRunning == false)
        return false;
    else
        return true;
}

float Timer::getPeriod() const
{
    return mPeriod;
}

void Timer::setPeriod(uint16_t pPeriod)
{
    mPeriod = pPeriod;
    mCounter = 0.f;
}

float Timer::getCounter() const
{
    return mCounter;
}


void Timer::tick(uint16_t dt)
{
    if(mRunning)
    {
        mCounter += dt;

        if(mCounter >= mPeriod)
        {
            mTrigger = true;
        }
    }
}


void Timer::setTriggerAction(void (*Press)(Timer *t))
{
	onTriggerAction = Press;
}


bool Timer::triggered() // Read timer Trigger and reset it if enabled
{
    bool TriggerValue = mTrigger;

    if(mTrigger == true)
    {
        mTrigger = false;
        mCounter = 0;
        mCounts++;

        if(mMode == OneShot)
            mRunning = false;

        else if(mMode == Continuous)
            mRunning = true;

        if(onTriggerAction != nullptr)
        	onTriggerAction(this);


        mToggled = !mToggled;
    }

    return TriggerValue;
}

void Timer::startTimer(float pPeriod) // Start Timer from beginning (see ReleaseTimer() and PauseTimer() )
{
    if(pPeriod != 0)
        mPeriod = pPeriod;

    mCounter = 0;
    mRunning = true;
}

bool Timer::toggled()
{
	return mToggled;
}

uint16_t Timer::counts()
{
	return mCounts;
}

void Timer::resetCounts()
{
	mCounts = 0;
}

void Timer::stopTimer() // Stop timer and reset it
{
    mRunning = 0;
    mCounter = 0;
}


void Timer::pauseTimer()
{
    mRunning = false;
}

void Timer::releaseTimer()
{
    mRunning = true;
}

unsigned int Timer::getMode() const
{
    return mMode;
}

void Timer::setMode(unsigned int pMode)
{
    if(pMode == Continuous || pMode == OneShot)
        mMode = pMode;
}


float Timer::getProgression() const
{
    return mCounter/mPeriod;
}
