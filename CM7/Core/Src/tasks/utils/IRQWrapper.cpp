/*
 * IRQWrapper.cpp
 *
 *  Created on: Jan 23, 2025
 *      Author: To
 */

#include "IRQWrapper.h"
#include "drivers/ModbusDriver.h"


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
