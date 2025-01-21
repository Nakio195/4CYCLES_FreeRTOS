/*
 * ModbusMasterWrapper.cpp
 *
 *  Created on: Jan 20, 2025
 *      Author: To
 */

#include "ModbusMaster.h"
#include "ModbusMasterWrapper.h"

extern "C" void ModbusIRQ_Wrapper(UART_HandleTypeDef* huart)
{
	ModbusMaster::enterIRQ(huart);
}


