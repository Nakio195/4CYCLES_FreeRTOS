/*
 * CanPeripheral.h
 *
 *  Created on: Feb 3, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_CANPERIPHERAL_H_
#define SRC_TASKS_UTILS_CANPERIPHERAL_H_

#include <inttypes.h>
#include <cmsis_os2.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "LockGuard.hpp"
#include "CanPacket.h"

class CanPeripheral
{
	public:
		enum FilterMode{Range, Mask};
		enum State{Unitialized, Initialized, Ready, Recovery, Lost, Absent};
	public:
		CanPeripheral();

		virtual void init()
		{
			mState = Initialized;
		}

		virtual void reInit() = 0;
		virtual void recovery() = 0;
		virtual void absent() = 0;

		void inline setRecoveryMode(uint8_t maxTries, uint16_t ticks)
		{
			mMaxRecovery = maxTries;
			mRecoveryTicks = ticks;
		}

		bool inline push(CanPacket *packet)
		{
			//Ensure data is not being manipulated elsewhere
			LockGuard lock(mutex);

			if(accept(packet->Identifier))
			{
				mLastCommunication = 0;
				mResponding = true;

				mState = Ready;

				// Add packet to Queue for children class to process
				if(xQueueSend(mPacketsQueue, &packet, 0) == pdTRUE)
					return true;
			}

			return false;
		}

	protected:
		bool inline isResponding()
		{
			LockGuard lock(mutex);
			return mResponding;
		}

		virtual void CommunicationTimeout()
		{
			if(mState == Initialized)
				mState = Absent;
			if(mState == Ready)
				mState = Recovery;
		}

		void inline setCommunicationTimeout(uint32_t timeout)
		{
			LockGuard lock(mutex);
			mCommunicationTimeout = timeout;
		}

		void inline tick(uint32_t t)
		{

			{ // Locked scope
				LockGuard lock(mutex);

				if(mState == Ready)
				{
					mLastCommunication += t - previousTick;
					previousTick = t;
				}
				if(mLastCommunication >= mCommunicationTimeout && mState != Recovery)
					CommunicationTimeout();

				if(mState == Absent)
				{
					absent();
				}

			}

			//Realease lock for CanTask to access push
			if(mState == Recovery)
			{
				while(mRecoveryAttempt < mMaxRecovery && mState == Recovery)
				{
					recovery();
					mRecoveryAttempt++;
					osDelay(mRecoveryTicks);
				}
			}
		}

		bool inline accept(uint32_t id)
		{
			if(mFilterMode == Range)
				return id >= mFilterLow && id <= mFilterHigh;
			if(mFilterMode == Mask)
				return (id & mFilterMask) == (mFilterId & mFilterMask);

			return false;
		}

		void inline setRangeFilter(uint32_t low, uint32_t high)
		{
			mFilterMode = Range;
			mFilterLow = low;
			mFilterHigh = high;
		}

		void inline setMaskFilter(uint32_t id, uint32_t mask)
		{
			mFilterMode = Mask;
			mFilterId = id;
			mFilterMask = mask;
		}

	protected:
		// Mutex
		SemaphoreHandle_t mutex;

		// Connection monitoring
		uint32_t previousTick;
		uint32_t mLastCommunication;
		uint32_t mCommunicationTimeout;
		bool mResponding;

		//State Machine
		State mState;
		uint8_t mRecoveryAttempt;
		uint8_t mMaxRecovery;
		uint16_t mRecoveryTicks;


		uint8_t mFilterMode;
		// Inclusive Range Filter
		uint32_t mFilterLow;
		uint32_t mFilterHigh;
		// Mask Filter
		uint32_t mFilterId;
		uint32_t mFilterMask;
		// Frame Queue
		QueueHandle_t mPacketsQueue;
};

#endif /* SRC_TASKS_UTILS_CANPERIPHERAL_H_ */
