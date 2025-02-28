/*
 * IRQWrapper.cpp
 *
 *  Created on: Jan 23, 2025
 *      Author: To
 */

#include "IRQWrapper.h"
#include "CANTask.h"

extern "C" void FDCAN_IRQ(FDCAN_HandleTypeDef* fdcan)
{
	SEGGER_SYSVIEW_RecordEnterISR();
	CanHandler.IRQ_Handler(fdcan);
	SEGGER_SYSVIEW_RecordExitISR();
}
