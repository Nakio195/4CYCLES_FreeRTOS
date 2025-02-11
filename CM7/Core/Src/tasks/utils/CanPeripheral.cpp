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

	mMaxRecovery = 3;
	mRecoveryTicks = 1000;

	mFilterMode = Range;
	mFilterLow = 0x00;
	mFilterHigh = 0x00;
	mFilterId = 0x00;
	mFilterMask = 0x000;

	mState = Unitialized;

	mPacketsQueue = xQueueCreate(10, sizeof(CanPacket*));
	mutex = xSemaphoreCreateRecursiveMutex();
}
