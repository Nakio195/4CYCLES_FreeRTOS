/*
 * CANTask.h
 *
 *  Created on: Jan 29, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_CANTASK_H_
#define SRC_TASKS_CANTASK_H_

#include "RTOSTask.h"
#include "cmsis_os2.h"
#include "semphr.h"
#include "fdcan.h"
#include <vector>

#include "utils/CanPacket.h"
#include "utils/CanPeripheral.h"


class CAN_Task : public RTOS_Task
{
	public:
		CAN_Task();

		void setup() override;
		void run() override;
		void cleanup() override;

		void attach(CanPeripheral* peripheral);
		bool send(CanPacket* packet);


		inline void IRQ_Handler(FDCAN_HandleTypeDef* hfdcan)
		{
			if(__HAL_FDCAN_GET_IT(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE) && __HAL_FDCAN_GET_IT_SOURCE(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE))
			{
				BaseType_t xHigherPriorityTaskWoken = pdFALSE;
				xSemaphoreGiveFromISR(Sem_MessageAvailable, &xHigherPriorityTaskWoken);
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
				__HAL_FDCAN_CLEAR_FLAG(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE);
			}
		}

	public:
		xQueueHandle TX_Queue;

		uint32_t TxErrorCounter;
		uint32_t RxErrorCounter;

	private:
		std::vector<CanPeripheral*> mPeripherals;
		xSemaphoreHandle Sem_MessageAvailable;


};

extern CAN_Task CanHandler;

#endif /* SRC_TASKS_CANTASK_H_ */
