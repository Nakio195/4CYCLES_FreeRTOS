/*
 * IRQWrapper.cpp
 *
 *  Created on: Jan 23, 2025
 *      Author: To
 */

#include "IRQWrapper.h"
#include "CANTask.h"
#include "DirectionTask.h"

extern "C" void FDCAN_IRQ(FDCAN_HandleTypeDef* fdcan)
{
	CanHandler.IRQ_Handler(fdcan);
	SEGGER_SYSVIEW_RecordExitISR();
}

extern "C" void TIM16_IRQ(TIM_HandleTypeDef* htim)
{
	DirectionHandler.tick(1);
}
