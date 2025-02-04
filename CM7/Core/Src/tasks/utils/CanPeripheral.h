/*
 * CanPeripheral.h
 *
 *  Created on: Feb 3, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_CANPERIPHERAL_H_
#define SRC_TASKS_UTILS_CANPERIPHERAL_H_

#include <inttypes.h>
#include "FreeRTOS.h"
#include "queue.h"

#include "CanPacket.h"

class CanPeripheral
{
	public:
		enum FilterMode{Range, Mask};
	public:
		CanPeripheral();
		bool inline push(CanPacket *packet)
		{
			if(accept(packet->Identifier))
			{
				mLastCommunication = 0;
				mResponding = true;
				// Add packet to Queue for children class to process
				if(xQueueSend(mPacketsQueue, &packet, 0) == pdTRUE)
					return true;
			}

			return false;
		}

		bool inline isResponding()
		{
			return mResponding;
		}

		virtual void CommunicationTimeout()
		{
		}

		void inline setCommunicationTimeout(uint32_t timeout)
		{
			mCommunicationTimeout = timeout;
		}

		void inline tick(uint32_t t)
		{
			mLastCommunication += t - previousTick;
			previousTick = t;

			if(mLastCommunication >= mCommunicationTimeout)
			{
				mResponding = false;
				CommunicationTimeout();
			}
		}

	protected:
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
		// Connection monitoring
		uint32_t previousTick;
		uint32_t mLastCommunication;
		uint32_t mCommunicationTimeout;
		bool mResponding;


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
