/*
 * ModbusDriver.h
 *
 *  Created on: Jan 16, 2025
 *      Author: To
 */

#ifndef SRC_UTILITIES_MODBUSHANDLER_ModbusDriver_H_
#define SRC_UTILITIES_MODBUSHANDLER_ModbusDriver_H_

#include <inttypes.h>
#include <vector>
#include <deque>
#include "usart.h"

#include "../../RTOSTask.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "semphr.h"

#define READ_HOLDING_REGISTERS 3 // Reads the binary contents of holding registers in the slave.
#define WRITE_MULTIPLE_REGISTERS 16 // Presets values into a sequence of holding registers.

#define MB_BUFFER_SIZE 128
#define MB_DEFAULT_TIMEOUT 1000

class ModbusDriver : public RTOS_Task
{
	public:
		enum MB_Error{
			TX_Buffer_Ov				= 1	<< 0,
			RX_Buffer_Ov				= 1 << 1,
			Frame_tooShort				= 1 << 2,
			Frame_CRC_Error 			= 1 << 3,
			Frame_InvalidFC 			= 1 << 4,
			Frame_InvalidRequest 		= 1 << 5,
			Frame_TruncatedData 		= 1 << 6,
			TX_Timeout					= 1 << 7,
			RX_Timeout					= 1 << 8,
			TX_TransmitError			= 1 << 9,
			RX_ReceiveError				= 1 << 10,
			Slave_Timeout 				= 1 << 11
		};

	public:
		ModbusDriver(UART_HandleTypeDef* huart);
		void setTimeout(uint16_t timeout);
		void setRetryCount(uint8_t retry);

		void run() override;
		bool readHoldingRegister(uint8_t slaveId, uint16_t startAddress, uint8_t numRegister, SemaphoreHandle_t *dataReady);
		bool beginMultipleWrite(uint8_t slaveId, uint16_t startAddress, SemaphoreHandle_t *dataReady);
		bool write(uint16_t data);
		bool endMultipleWrite();

		bool lastRequestStatus();
		bool lastRequestError();
		bool busy();
		uint8_t available();
		uint16_t read();

		inline void onIRQ()
		{
			// TX Buffer Empty
			if(__HAL_UART_GET_IT(mHuart, UART_IT_TXE) && __HAL_UART_GET_IT_SOURCE(mHuart, UART_IT_TXE))
			{
				if(TX_LoadPointer < TX_Pointer)
				{
					mHuart->Instance->TDR = TX_Buffer[TX_LoadPointer];
					TX_LoadPointer++;
				}

				else
				{
					BaseType_t xHigherPriorityTaskWoken = pdFALSE;
					xSemaphoreGiveFromISR(Sem_TxMessage, &xHigherPriorityTaskWoken);
					portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
					__HAL_UART_DISABLE_IT(mHuart, UART_IT_TXE);
				}

				__HAL_UART_CLEAR_FLAG(mHuart, UART_FLAG_TXE);
			}
			// Received character
			else if(__HAL_UART_GET_IT(mHuart, UART_IT_RXNE) && __HAL_UART_GET_IT_SOURCE(mHuart, UART_IT_RXNE))
			{
				if(RX_Pointer == 0)
				{
					__HAL_UART_CLEAR_IDLEFLAG(mHuart);
					__HAL_UART_ENABLE_IT(mHuart, UART_IT_IDLE);
				}

				if(RX_Pointer < MB_BUFFER_SIZE)
				{
					RX_Buffer[RX_Pointer] = (uint8_t)(mHuart->Instance->RDR);
					RX_Pointer++;
				}
				__HAL_UART_CLEAR_FLAG(mHuart, UART_FLAG_RXNE);
			}

			// Transfer complete by IDLE line
			else if(__HAL_UART_GET_IT(mHuart, UART_IT_IDLE) && __HAL_UART_GET_IT_SOURCE(mHuart, UART_IT_IDLE))
			{
				BaseType_t xHigherPriorityTaskWoken = pdFALSE;
				xSemaphoreGiveFromISR(Sem_RxResponse, &xHigherPriorityTaskWoken);
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
				__HAL_UART_DISABLE_IT(mHuart, UART_IT_RXNE);
				__HAL_UART_DISABLE_IT(mHuart, UART_IT_IDLE);
				__HAL_UART_CLEAR_FLAG(mHuart, UART_FLAG_IDLE);
			}
		}

	private:


		bool processFrame();
		void errorHandler();
		uint16_t calculateCRC(uint8_t *data, uint8_t size);
		void buildFrameHeader();
		void TX_BufferWrite(uint8_t b);
		bool sendRequest();
		bool readResponse();

	private:
		// state machine states
		enum State{
			Init,
		    Idle,
			WritePreparation,
		    SendRequest,
		    WaitResponse,
		    ProcessResponse,
			InterMessageDelay,
			DataReady,
		    Error
		};

		uint8_t mState;
		uint32_t mError;
		uint32_t mLastError;

		typedef struct {
			bool valid;
		    uint8_t slaveAddress;   	// Slave Address
		    uint8_t functionCode;   	// FunctionCode (1 byte)
		    uint16_t startAddress;  	// Start Address (2 bytes)
		    uint16_t numRegisters;  	// Number of registers (2 bytes)
		    std::deque<uint16_t> data;  //Data to send (FC 0x10 only)
		    uint16_t crc;           	// CRC (2 bytes)
		} ModbusRTUFrame;

		ModbusRTUFrame mFrame;

		uint16_t mTimeout;
		uint16_t mTimeoutCounter;
		uint8_t mMaxRetryCount;
		uint8_t mRetryCount;

		UART_HandleTypeDef *mHuart;

		SemaphoreHandle_t Sem_TxMessage;
		SemaphoreHandle_t Sem_TxChar;
		SemaphoreHandle_t Sem_TxTransmission;

		uint8_t TX_Buffer[128];
		uint8_t TX_Pointer;
		uint8_t TX_LoadPointer;

		SemaphoreHandle_t Sem_RxResponse;
		uint8_t RX_Buffer[128];
		uint8_t RX_Pointer;

		SemaphoreHandle_t* mDataReady;
};

extern ModbusDriver Serial1;
extern ModbusDriver Serial2;
extern ModbusDriver Serial4;
extern ModbusDriver Serial6;


/*

void ModbusDriver::IRQ_Handler()
{

	// Transfer Complete IT
	if(__HAL_UART_GET_FLAG(mHuart, UART_FLAG_TC))
	{
		if(mState == State::SendRequest)
		{
			mState = State::WaitResponse;
			__HAL_UART_ENABLE_IT(mHuart, UART_IT_RXNE);
		}
	}
	// Received character
	if(__HAL_UART_GET_FLAG(mHuart, UART_FLAG_RXNE))
	{
		if(mState == State::WaitResponse && RX_Pointer < MB_BUFFER_SIZE)
		{
			RX_Buffer[RX_Pointer] = (uint8_t)(mHuart->Instance->RDR);
		}
		__HAL_UART_ENABLE_IT(mHuart, UART_IT_IDLE);
		__HAL_UART_CLEAR_FLAG(mHuart, UART_FLAG_RXNE);
	}

	// Transfer complete by IDLE line
	if(__HAL_UART_GET_FLAG(mHuart, UART_FLAG_IDLE))
	{
		if(mState == State::WaitResponse)
			mState = State::ProcessResponse;

		__HAL_UART_DISABLE_IT(mHuart, UART_IT_RXNE);
		__HAL_UART_DISABLE_IT(mHuart, UART_IT_IDLE);
		__HAL_UART_CLEAR_FLAG(mHuart, UART_FLAG_IDLE);
	}

}

*/

#endif /* SRC_UTILITIES_MODBUSHANDLER_ModbusDriver_H_ */
