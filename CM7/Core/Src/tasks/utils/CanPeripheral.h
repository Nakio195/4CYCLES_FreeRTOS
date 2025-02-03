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
		bool push(CanPacket *packet);

	protected:
		bool accept(uint32_t id);
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
