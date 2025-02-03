/*
 * CanPacket.h
 *
 *  Created on: Feb 3, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_CANPACKET_H_
#define SRC_TASKS_UTILS_CANPACKET_H_

#include "fdcan.h"
#include <vector>

class CanPacket
{
	public:
		enum {Invalid, Receive = 1, Transmit = 2};

	public:
		CanPacket(uint32_t id = 0) : Identifier(id)
		{
		}

		CanPacket(FDCAN_RxHeaderTypeDef header)
		{
			direction = Receive;
			Identifier = header.Identifier;
			DataLengthCode = header.DataLength;
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

		inline FDCAN_TxHeaderTypeDef TxHeader()
		{
			FDCAN_TxHeaderTypeDef header;

			if(data.size() <= 8)
				DataLengthCode = data.size();
			else if (data.size() == 12)
				DataLengthCode = FDCAN_DLC_BYTES_12;
			else if (data.size() == 16)
				DataLengthCode = FDCAN_DLC_BYTES_16;
			else if (data.size() == 20)
				DataLengthCode = FDCAN_DLC_BYTES_20;
			else if (data.size() == 24)
				DataLengthCode = FDCAN_DLC_BYTES_24;
			else if (data.size() == 32)
				DataLengthCode = FDCAN_DLC_BYTES_32;
			else if (data.size() == 48)
				DataLengthCode = FDCAN_DLC_BYTES_48;

			header.Identifier = Identifier;
			header.IdType = FDCAN_STANDARD_ID;             // Standard ID type (11 bits)
			header.TxFrameType = FDCAN_DATA_FRAME;         // Data frame
			header.DataLength = DataLengthCode;         // Data length code (DLC) - 8 bytes of data
			header.ErrorStateIndicator = FDCAN_ESI_PASSIVE; // Error state indicator (PASSIVE)
			header.BitRateSwitch = FDCAN_BRS_OFF;          // No bit-rate switch (for classic CAN)
			header.FDFormat = FDCAN_CLASSIC_CAN;           // Classic CAN frame format
			header.TxEventFifoControl = FDCAN_NO_TX_EVENTS; // No event FIFO control
			header.MessageMarker = 0;

			return header;
		}

	public:
		bool direction = Invalid;
		uint32_t Identifier;
		uint32_t DataLengthCode;
		std::vector<uint8_t> data;
};


#endif /* SRC_TASKS_UTILS_CANPACKET_H_ */
