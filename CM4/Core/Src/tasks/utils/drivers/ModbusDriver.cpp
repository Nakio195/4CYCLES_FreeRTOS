/*
 * ModbusDriver.cpp
 *
 *  Created on: Jan 16, 2025
 *      Author: To
 */

#include "ModbusDriver.h"

ModbusDriver Serial1(&huart1);
ModbusDriver Serial2(&huart2);
ModbusDriver Serial4(&huart4);
ModbusDriver Serial6(&huart6);

#define SLAVE_ID_1	Serial1
#define SLAVE_ID_2	Serial2
#define SLAVE_ID_3	Serial4
#define SLAVE_ID_4	Serial6

ModbusDriver::ModbusDriver(UART_HandleTypeDef* huart)
{
	// TODO Auto-generated constructor stub
	mState = State::Init;
	mHuart = huart;
	mTimeout = MB_DEFAULT_TIMEOUT;

	Sem_TxMessage = xSemaphoreCreateBinary();
	Sem_TxChar = xSemaphoreCreateBinary();
	Sem_TxTransmission = xSemaphoreCreateBinary();
	Sem_RxResponse = xSemaphoreCreateBinary();

	__HAL_UART_CLEAR_FLAG(mHuart, UART_FLAG_TC);
	__HAL_UART_CLEAR_FLAG(mHuart, UART_FLAG_FE);
}

void ModbusDriver::run()
{
    switch (mState)
    {
        case State::Init:
            mState = State::Idle;
            break;

        case State::Idle:
            suspend();
            break;

        case State::SendRequest:
        	if(sendRequest())
        	{
        		mState = State::WaitResponse;
        		mTimeoutCounter = 0;
        	}
			break;

        case State::WaitResponse:
        	if(readResponse())
        		mState = State::ProcessResponse;
            break;

        case State::ProcessResponse:
            if(processFrame())
            	mState = State::DataReady;
            break;

        case State::DataReady:
        	xSemaphoreGive(*mDataReady);
        	mState = State::InterMessageDelay;
			break;


        case State::InterMessageDelay:
        	//vTaskDelay(pdMS_TO_TICKS(1));
        	mState = State::Idle;
        	break;

        case State::Error:
            errorHandler();
            mState = State::Idle;
            break;
    }
}


bool ModbusDriver::busy()
{
	if(mState != State::Idle && mState != State::DataReady)
		return true;

	return false;
}

uint8_t ModbusDriver::available()
{
	return mFrame.data.size();
}

uint16_t ModbusDriver::read()
{
	if(mFrame.data.size() == 0)
		return -1;

	uint16_t data = mFrame.data.front();
	mFrame.data.pop_front();

	return data;
}

bool ModbusDriver::lastRequestStatus()
{
	return mFrame.valid;
}

bool ModbusDriver::lastRequestError()
{
	return mLastError;
}

bool ModbusDriver::readHoldingRegister(uint8_t slaveId, uint16_t startAddress, uint8_t numRegister, SemaphoreHandle_t *dataReady)
{
	if(mState != State::Idle)
		return false;
	mDataReady = dataReady;

	mFrame.functionCode = READ_HOLDING_REGISTERS;
	mFrame.slaveAddress = slaveId;
	mFrame.startAddress = startAddress;
	mFrame.numRegisters = numRegister;

	buildFrameHeader();
	uint16_t crc = calculateCRC(TX_Buffer, TX_Pointer);

	TX_BufferWrite(crc & 0xFF);
	TX_BufferWrite(crc >> 8);

	mState = State::SendRequest;

	resume();

	return true;
}

bool ModbusDriver::beginMultipleWrite(uint8_t slaveId, uint16_t startAddress, SemaphoreHandle_t *dataReady)
{
	if(mState == State::Idle || mState == State::InterMessageDelay)
	{
		mFrame.functionCode = WRITE_MULTIPLE_REGISTERS;
		mFrame.slaveAddress = slaveId;
		mFrame.startAddress = startAddress;
		mFrame.numRegisters = 0;
		mFrame.data.clear();

		mDataReady = dataReady;
		mState = State::WritePreparation;
		return true;
	}

	return false;
}

bool ModbusDriver::write(uint16_t data)
{
	if(mState == State::WritePreparation)
	{
		mFrame.data.push_back(data);
		return true;
	}

	return false;
}

bool ModbusDriver::endMultipleWrite()
{
	mFrame.numRegisters = mFrame.data.size();
	buildFrameHeader();

	for(uint8_t i = 0; i < mFrame.data.size(); i++)
	{
		TX_BufferWrite(mFrame.data[i] >> 8);
		TX_BufferWrite(mFrame.data[i] & 0xFF);
	}

	uint16_t crc = calculateCRC(TX_Buffer, TX_Pointer);

	TX_BufferWrite(crc & 0xFF);
	TX_BufferWrite(crc >> 8);

	mState = State::SendRequest;

	resume();

	return true;
}


bool ModbusDriver::sendRequest()
{

	xSemaphoreTake(Sem_TxMessage, 0); //Reset semaphores

	TX_LoadPointer = 0;
	__HAL_UART_ENABLE_IT(mHuart, UART_IT_TXE);

	mHuart->Instance->TDR = TX_Buffer[0];
	if (xSemaphoreTake(Sem_TxMessage, pdMS_TO_TICKS(MB_DEFAULT_TIMEOUT)) != pdTRUE)
	{
		mState = State::Error;
		mError |= TX_Timeout;
		__HAL_UART_DISABLE_IT(mHuart, UART_IT_TXE);
		return false;
	}

	__HAL_UART_DISABLE_IT(mHuart, UART_IT_TXE);

	return true;
}

bool ModbusDriver::readResponse()
{
	RX_Pointer = 0;

	// Clearing Error bits
	__HAL_UART_CLEAR_FLAG(mHuart, UART_FLAG_ORE);
	__HAL_UART_CLEAR_FLAG(mHuart, UART_FLAG_FE);
	__HAL_UART_CLEAR_FLAG(mHuart, UART_FLAG_NE);
	//Enabling IT
	__HAL_UART_ENABLE_IT(mHuart, UART_IT_RXNE);
	xSemaphoreTake(Sem_RxResponse, 0);
	if (xSemaphoreTake(Sem_RxResponse, pdMS_TO_TICKS(MB_DEFAULT_TIMEOUT)) != pdTRUE)
	{
		mState = State::Error;
		mError |= Slave_Timeout;

		__HAL_UART_DISABLE_IT(mHuart, UART_IT_RXNE);
		__HAL_UART_DISABLE_IT(mHuart, UART_IT_IDLE);
		return false;
	}

	__HAL_UART_DISABLE_IT(mHuart, UART_IT_RXNE);
	__HAL_UART_DISABLE_IT(mHuart, UART_IT_IDLE);

	return true;
}

bool ModbusDriver::processFrame()
{
	// Default valid Frame to false
	mFrame.valid = false;

	// Checking Minimum size Frame
	if(RX_Pointer < 5)
	{
		mState = State::Error;
		mError |= Frame_tooShort;
		return false;
	}

	// Checking CRC
	uint16_t receivedCRC = (RX_Buffer[RX_Pointer - 1] << 8 | (RX_Buffer[RX_Pointer - 2]));
	uint16_t calculatedCRC = calculateCRC(RX_Buffer, RX_Pointer - 2);
	if(receivedCRC != calculatedCRC)
	{
		mState = State::Error;
		mError |= Frame_tooShort;
		return false;
	}

	mFrame.slaveAddress = RX_Buffer[0];
	mFrame.functionCode = RX_Buffer[1];

	// Checking supported FunctionCode
	if(mFrame.functionCode != READ_HOLDING_REGISTERS && mFrame.functionCode != WRITE_MULTIPLE_REGISTERS)
	{
		mState = State::Error;
		mError |= Frame_InvalidFC;
		mFrame.valid = false;
		return false;
	}

	// Checking Invalid request
	if(mFrame.functionCode & 0x80)
	{
		mState = State::Error;
		mError |= Frame_InvalidRequest;
		mFrame.valid = false;
		return false;
	}

	/*
	 *	FC = 0x03 Read Holding Register
	 */

	if(mFrame.functionCode == READ_HOLDING_REGISTERS)
	{
		uint8_t byteCount = RX_Buffer[2];
		// Checking data lenght
		if (RX_Pointer != (3 + byteCount + 2))
		{
			mState = State::Error;
			mError |= Frame_TruncatedData;
			mFrame.valid = false;
			return false;
		}

		// Populating data
		mFrame.data.clear();
		for (size_t i = 0; i < byteCount; i += 2)
		{
			uint16_t value = (RX_Buffer[3 + i] << 8) | RX_Buffer[4 + i];
			mFrame.data.push_back(value);
		}

		mFrame.valid = true;
		mState = State::DataReady;
	}

	/*
	 *	FC = 0x10 Write Multiple Register
	 */

	else if(mFrame.functionCode == WRITE_MULTIPLE_REGISTERS)
	{
		// Valid answer must be 8 octet long
		if(RX_Pointer != 8)
		{
			mState = State::Error;
			mError |= Frame_tooShort;
			mFrame.valid = false;
			return false;
		}

		mFrame.data.clear();
		mFrame.startAddress = (RX_Buffer[2] << 8) | RX_Buffer[3];
		mFrame.numRegisters = (RX_Buffer[4] << 8) | RX_Buffer[5];
		mFrame.valid = true;
	}

	return true;
}

void ModbusDriver::errorHandler()
{
	mLastError = mError;
	mError = 0;

	HAL_UART_AbortTransmit_IT(mHuart);

	__HAL_UART_DISABLE_IT(mHuart, UART_IT_TC);
	__HAL_UART_DISABLE_IT(mHuart, UART_IT_RXNE);
	__HAL_UART_DISABLE_IT(mHuart, UART_IT_IDLE);
}

void ModbusDriver::TX_BufferWrite(uint8_t b)
{
	if(TX_Pointer == MB_BUFFER_SIZE)
	{
		mState = State::Error;
		mError |= MB_Error::TX_Buffer_Ov;
		return;
	}

	TX_Buffer[TX_Pointer] = b;
	TX_Pointer++;
}

void ModbusDriver::buildFrameHeader()
{
	TX_Pointer = 0;

	TX_BufferWrite(mFrame.slaveAddress);
	TX_BufferWrite(mFrame.functionCode);

	if(mFrame.functionCode == READ_HOLDING_REGISTERS)
	{
		TX_BufferWrite(mFrame.startAddress >> 8);
		TX_BufferWrite(mFrame.startAddress & 0xFF);
		TX_BufferWrite(mFrame.numRegisters >> 8);
		TX_BufferWrite(mFrame.numRegisters & 0xFF);
	}

	else if(mFrame.functionCode == WRITE_MULTIPLE_REGISTERS)
	{
		TX_BufferWrite(mFrame.startAddress >> 8);
		TX_BufferWrite(mFrame.startAddress & 0xFF);
		TX_BufferWrite(mFrame.numRegisters >> 8);
		TX_BufferWrite(mFrame.numRegisters & 0xFF);
		TX_BufferWrite(mFrame.numRegisters*2);
	}
}

uint16_t ModbusDriver::calculateCRC(uint8_t *data, uint8_t size)
{
    uint16_t crc = 0xFFFF;
    uint16_t i, j;

    for (i = 0; i < size; i++) {
        crc ^= data[i];  // XOR the byte into the CRC

        for (j = 8; j; j--) {  // Process each bit in the byte
            if (crc & 0x01)  // Check if the LSB of CRC is 1
                crc = (crc >> 1) ^ 0xA001;  // Shift right and XOR with the polynomial
            else
                crc >>= 1;  // Just shift right
        }
    }

    return crc;  // Return the computed CRC value
}

