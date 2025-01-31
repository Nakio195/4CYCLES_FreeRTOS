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
#include "queue.h"

#include "fdcan.h"
#include <vector>
#include <deque>

class CANPacket
{
	public:
		enum {Invalid, Receive = 1, Transmit = 2};

	public:
		CANPacket(uint8_t dir) : direction(dir)
		{
		    TxHeader.IdType = FDCAN_STANDARD_ID;             // Standard ID type (11 bits)
		    TxHeader.TxFrameType = FDCAN_DATA_FRAME;         // Data frame
		    TxHeader.DataLength = FDCAN_DLC_BYTES_8;         // Data length code (DLC) - 8 bytes of data
		    TxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE; // Error state indicator (PASSIVE)
		    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;          // No bit-rate switch (for classic CAN)
		    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;           // Classic CAN frame format
		    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS; // No event FIFO control
		    TxHeader.MessageMarker = 0;
		}

		static inline uint8_t dataLength(uint32_t DLC)
		{
			if(DLC <= 8)
				return DLC;
			else if(DLC == FDCAN_DLC_BYTES_12)
				return 12;
			else if(DLC == FDCAN_DLC_BYTES_16)
				return 16;
			else if(DLC == FDCAN_DLC_BYTES_20)
				return 20;
			else if(DLC == FDCAN_DLC_BYTES_24)
				return 24;
			else if(DLC == FDCAN_DLC_BYTES_32)
				return 32;
			else if(DLC == FDCAN_DLC_BYTES_48)
				return 48;
			else if(DLC == FDCAN_DLC_BYTES_64)
				return 64;

			return 0;
		}


	public:
		bool direction = Invalid;
		FDCAN_TxHeaderTypeDef TxHeader;
		FDCAN_RxHeaderTypeDef RxHeader;
		std::vector<uint8_t> data;
};

class CAN_Task : public RTOS_Task
{
	public:
		CAN_Task();

		void setup() override;
		void run() override;
		void cleanup() override;

		bool send(CANPacket* packet);


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

	private:
		std::deque<CANPacket> mRX_Packets;
		std::deque<CANPacket> mTX_Packets;
		xSemaphoreHandle Sem_MessageAvailable;


};

extern CAN_Task CanHandler;

#endif /* SRC_TASKS_CANTASK_H_ */
