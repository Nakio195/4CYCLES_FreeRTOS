/*
 * CANTask.cpp
 *
 *  Created on: Jan 29, 2025
 *      Author: To
 */

#include "CANTask.h"

CAN_Task CanHandler;
CANPacketPoolHandler CanPacketPool;

CAN_Task::CAN_Task()
{
}

void CAN_Task::setup()
{
	FDCAN_FilterTypeDef sFilterConfig;
	/* Configure Rx filter */
	sFilterConfig.IdType = FDCAN_EXTENDED_ID;
	sFilterConfig.FilterIndex = 0;
	sFilterConfig.FilterType = FDCAN_FILTER_RANGE;
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilterConfig.FilterID1 = 0x00;
	sFilterConfig.FilterID2 = 0x1FFFFFFF;

	if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig) != HAL_OK)
		Error_Handler();

	/* Start the FDCAN module */
	if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK)
		Error_Handler();

	Sem_MessageAvailable = xSemaphoreCreateBinary();
	TX_Queue = xQueueCreate(10, sizeof(CanPacket*));

	if (HAL_FDCAN_ActivateNotification(&hfdcan2, (FDCAN_IT_RX_FIFO0_NEW_MESSAGE |
													FDCAN_IT_TX_EVT_FIFO_FULL |
													FDCAN_IT_RX_FIFO0_FULL |
													FDCAN_IT_ERROR_WARNING |
													FDCAN_IT_ERROR_PASSIVE|
													FDCAN_IT_BUS_OFF|
													FDCAN_IT_ARB_PROTOCOL_ERROR |
													FDCAN_IT_DATA_PROTOCOL_ERROR
												 ) , 0) != HAL_OK)
		Error_Handler();

}

void CAN_Task::run()
{

	// Take Semaphore and wait 10ms  for a give
	xSemaphoreTake(Sem_MessageAvailable, 0);

	if(xSemaphoreTake(Sem_MessageAvailable, 1) == pdTRUE)
	{
		while ((hfdcan2.Instance->RXF0S & FDCAN_RXF0S_F0FL) != 0)
		{
			// Retrieve Message from FIFO
			uint8_t RX_Data[64] = {0};
			uint32_t RxFifoIndex = (hfdcan2.Instance->RXF0S & FDCAN_RXF0S_F0GI) >> FDCAN_RXF0S_F0GI_Pos; // Get message index
			FDCAN_RxHeaderTypeDef header;
			HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &header, RX_Data); // Read message
			hfdcan2.Instance->RXF0A = RxFifoIndex; // Acknowledge RX FIFO index

			// Build CanPacket from received message
			CanPacket* packet = CanPacketPool.allocate(header.Identifier);
			for(uint8_t i = 0; i < CanPacket::dataLength(header.DataLength); i++)
				packet->data.push_back(RX_Data[i]);

			bool accepted = false;
			for(auto peripheral : mPeripherals)
			{
				accepted = peripheral->push(packet);

				if(accepted)
					break;
			}

			if(!accepted)
			{
				CanPacketPool.free(packet);
				RxErrorCounter++;
				//TODO Notify Identifier of lost packet
			}

		}
	}

	CanPacket* packet = nullptr;
	if(xQueueReceive(TX_Queue, &packet, 0) == pdTRUE)
	{
		if(packet != nullptr)
		{
			FDCAN_TxHeaderTypeDef header = packet->TxHeader();
			if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &header, packet->data.data()) != HAL_OK)
			{
				TxErrorCounter++;
				//TODO Notify TX send error
				//Error_Handler();
			}

			CanPacketPool.free(packet);
		}
	}
}

void CAN_Task::attach(CanPeripheral* peripheral)
{
	if(peripheral != nullptr)
		mPeripherals.push_back(peripheral);
}

bool CAN_Task::send(CanPacket* packet)
{
	if(xQueueSend(TX_Queue, &packet, 1) != pdTRUE)
		return false;

	return true;
}

void CAN_Task::cleanup()
{

}
