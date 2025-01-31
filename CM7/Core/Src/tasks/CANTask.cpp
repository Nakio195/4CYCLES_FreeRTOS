/*
 * CANTask.cpp
 *
 *  Created on: Jan 29, 2025
 *      Author: To
 */

#include "CANTask.h"

CAN_Task CanHandler;

CAN_Task::CAN_Task()
{

}

void CAN_Task::setup()
{
	FDCAN_FilterTypeDef sFilterConfig;
	/* Configure Rx filter */
	sFilterConfig.IdType = FDCAN_STANDARD_ID;
	sFilterConfig.FilterIndex = 0;
	sFilterConfig.FilterType = FDCAN_FILTER_RANGE;
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilterConfig.FilterID1 = 0x010;
	sFilterConfig.FilterID2 = 0x7FF;
	if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig) != HAL_OK)
	{
		Error_Handler();
	}

	/* Start the FDCAN module */
	if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK)
	{
		Error_Handler();
	}

	if (HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
	{
		Error_Handler();
	}

	Sem_MessageAvailable = xSemaphoreCreateBinary();
	TX_Queue = xQueueCreate(10, sizeof(CANPacket*));

}

void CAN_Task::run()
{
	xSemaphoreTake(Sem_MessageAvailable, 0);

	if(xSemaphoreTake(Sem_MessageAvailable, 10) == pdTRUE)
	{
		while ((hfdcan2.Instance->RXF0S & FDCAN_RXF0S_F0FL) != 0)
		{
			uint8_t RX_Data[64] = {0};
			CANPacket RX_Packet(CANPacket::Receive);
			uint32_t RxFifoIndex = (hfdcan2.Instance->RXF0S & FDCAN_RXF0S_F0GI) >> FDCAN_RXF0S_F0GI_Pos; // Get message index
			HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &RX_Packet.RxHeader, RX_Data); // Read message
			hfdcan2.Instance->RXF0A = RxFifoIndex; // Acknowledge RX FIFO index

			for(uint8_t i = 0; i < CANPacket::dataLength(RX_Packet.RxHeader.DataLength); i++)
				RX_Packet.data.push_back(RX_Data[i]);

			mRX_Packets.push_back(RX_Packet);
		}
	}

	CANPacket* packet = nullptr;
	if(xQueueReceive(TX_Queue, &packet, 10) == pdTRUE)
	{
		if(packet != nullptr)
		{
			if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &packet->TxHeader, packet->data.data()) != HAL_OK)
			{
			  Error_Handler();
			}
			delete packet;
		}
	}

	osDelay(10);
}

bool CAN_Task::send(CANPacket* packet)
{
	if(xQueueSend(TX_Queue, &packet, 20) != pdTRUE)
		return false;

	return true;
}

void CAN_Task::cleanup()
{

}
