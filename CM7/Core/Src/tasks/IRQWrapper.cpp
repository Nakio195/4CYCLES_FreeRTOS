/*
 * IRQWrapper.cpp
 *
 *  Created on: Jan 23, 2025
 *      Author: To
 */

#include "IRQWrapper.h"
#include "CANTask.h"
#include "DirectionTask.h"
#include "utils/drivers/ModbusDriver.h"

extern "C" void FDCAN_IRQ(FDCAN_HandleTypeDef* fdcan)
{
	CanHandler.IRQ_Handler(fdcan);
}

extern "C" void TIM16_IRQ(TIM_HandleTypeDef* htim)
{
	DirectionHandler.tick(1);
}


extern "C" void UART_IRQ(UART_HandleTypeDef* huart)
{
	if(huart == &huart1)
		Serial1.onIRQ();
	if(huart == &huart2)
		Serial2.onIRQ();
	if(huart == &huart4)
		Serial4.onIRQ();
	if(huart == &huart6)
		Serial6.onIRQ();
}
