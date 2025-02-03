/*
 * CanPeripheral.cpp
 *
 *  Created on: Feb 3, 2025
 *      Author: To
 */

#include "CanPeripheral.h"

CanPeripheral::CanPeripheral()
{
	mFilterMode = Range;
	mFilterLow = 0x00;
	mFilterHigh = 0x00;
	mFilterId = 0x00;
	mFilterMask = 0x000;

	mPacketsQueue = xQueueCreate(10, sizeof(CanPacket*));

}

bool CanPeripheral::push(CanPacket *packet)
{
	if(accept(packet->Identifier))
	{
		// Add packet to Queue for children class to process
		if(xQueueSend(mPacketsQueue, &packet, 0) == pdTRUE)
			return true;
	}

	return false;
}

bool CanPeripheral::accept(uint32_t id)
{
	if(mFilterMode == Range)
		return id >= mFilterLow && id <= mFilterHigh;
	if(mFilterMode == Mask)
		return (id & mFilterMask) == (mFilterId & mFilterMask);

	return false;
}
