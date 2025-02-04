/*
 * CanPeripheral.cpp
 *
 *  Created on: Feb 3, 2025
 *      Author: To
 */

#include "CanPeripheral.h"

CanPeripheral::CanPeripheral()
{
	mLastCommunication = 0;
	mCommunicationTimeout = 0xFFFFFFFF;

	mFilterMode = Range;
	mFilterLow = 0x00;
	mFilterHigh = 0x00;
	mFilterId = 0x00;
	mFilterMask = 0x000;

	mPacketsQueue = xQueueCreate(10, sizeof(CanPacket*));
}
