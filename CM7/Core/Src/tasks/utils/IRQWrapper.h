/*
 * IRQWrapper.h
 *
 *  Created on: Jan 23, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_IRQWRAPPER_H_
#define SRC_TASKS_UTILS_IRQWRAPPER_H_

#include "usart.h"

#ifdef __cplusplus
extern "C" {
#endif

void UART_IRQ(UART_HandleTypeDef* huart);

#ifdef __cplusplus
}
#endif

#endif /* SRC_TASKS_UTILS_IRQWRAPPER_H_ */
