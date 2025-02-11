/*
 * LockGuard.hpp
 *
 *  Created on: Feb 4, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_LOCKGUARD_HPP_
#define SRC_TASKS_UTILS_LOCKGUARD_HPP_

#include "FreeRTOS.h"
#include "semphr.h"

class LockGuard
{

	public:
	LockGuard(SemaphoreHandle_t &m) : mutex(m)
	{
		xSemaphoreTake(mutex, portMAX_DELAY);
	}

	LockGuard(SemaphoreHandle_t &m, uint32_t ticks) : mutex(m)
	{
		xSemaphoreTake(mutex, ticks);
	}

		~LockGuard()
		{
			xSemaphoreGive(mutex);
		}

	private:
		SemaphoreHandle_t &mutex;
};

#endif /* SRC_TASKS_UTILS_LOCKGUARD_HPP_ */
