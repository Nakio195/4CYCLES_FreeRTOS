/*
 * IRQWrapper.h
 *
 *  Created on: Jan 23, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_IRQWRAPPER_H_
#define SRC_TASKS_UTILS_IRQWRAPPER_H_

#include "fdcan.h"

#ifdef __cplusplus
extern "C" {
#endif

void FDCAN_IRQ(FDCAN_HandleTypeDef* huart);

#ifdef __cplusplus
}
#endif

#endif /* SRC_TASKS_UTILS_IRQWRAPPER_H_ */
