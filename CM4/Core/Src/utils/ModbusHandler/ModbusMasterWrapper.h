/*
 * ModbusMasterWrapper.h
 *
 *  Created on: Jan 20, 2025
 *      Author: To
 */

#ifndef SRC_UTILS_MODBUSHANDLER_MODBUSMASTERWRAPPER_H_
#define SRC_UTILS_MODBUSHANDLER_MODBUSMASTERWRAPPER_H_

#include "usart.h"
#ifdef __cplusplus
extern "C" {
#endif

void ModbusIRQ_Wrapper(UART_HandleTypeDef* huart);

#ifdef __cplusplus
}
#endif

#endif /* SRC_UTILS_MODBUSHANDLER_MODBUSMASTERWRAPPER_H_ */
