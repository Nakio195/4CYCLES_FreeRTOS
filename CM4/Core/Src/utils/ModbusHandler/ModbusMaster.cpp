/*
 * ModbusMaster.cpp
 *
 *  Created on: Jan 16, 2025
 *      Author: To
 */

#include "ModbusMaster.h"

ModbusMaster* Serial1;
ModbusMaster* Serial2;
ModbusMaster* Serial4;
ModbusMaster* Serial6;



ModbusMaster::ModbusMaster(UART_HandleTypeDef* huart)
{
	// TODO Auto-generated constructor stub
	mState = State::Init;
	mHuart = huart;
	mTimeout = MB_DEFAULT_TIMEOUT;

	__HAL_UART_CLEAR_FLAG(mHuart, UART_FLAG_TC);
	__HAL_UART_CLEAR_FLAG(mHuart, UART_FLAG_FE);
}

void ModbusMaster::process()
{
    switch (mState)
    {
        case State::Init:
            mState = State::Idle;
            break;

        case State::Idle:
            break;

        case State::WaitResponse:
        	mTimeoutCounter += HAL_GetTick() - mTimeoutCounter;

        	if(mTimeoutCounter > mTimeout)
        	{
        		mState = State::Error;
        		mError |= Timeout;
        		errorHandler();
        	}
            break;

        case State::ProcessResponse:
            processFrame();
            break;

        case State::InterMessageDelay:
        	break;

        case State::DataReady:
			break;

        case State::Error:
            errorHandler();
            mState = State::Idle;
            break;
    }
}


bool ModbusMaster::busy()
{
	if(mState != State::Idle && mState != State::DataReady)
		return true;

	return false;
}

uint8_t ModbusMaster::available()
{
	if(mState == State::DataReady)
		return mFrame.data.size();

	else
		return 0;
}

uint16_t ModbusMaster::read()
{
	if(mState == State::DataReady && mFrame.data.size() == 0)
		return -1;

	uint16_t data = mFrame.data.front();
	mFrame.data.pop_front();

	return data;
}

bool ModbusMaster::lastRequestStatus()
{
	if(mState != State::DataReady)
		return false;

	return mFrame.valid;
}

bool ModbusMaster::lastRequestError()
{
	return mLastError;
}

bool ModbusMaster::readHoldingRegister(uint8_t slaveId, uint16_t startAddress, uint8_t numRegister)
{
	if(mState != State::Idle)
		return false;

	mFrame.functionCode = READ_HOLDING_REGISTERS;
	mFrame.slaveAddress = slaveId;
	mFrame.startAddress = startAddress;
	mFrame.numRegisters = numRegister;

	buildFrameHeader();
	uint16_t crc = calculateCRC(TX_Buffer, TX_Pointer);

	TX_BufferWrite(crc << 8);
	TX_BufferWrite(crc & 0xFF);

	mState = State::SendRequest;
	__HAL_UART_CLEAR_FLAG(mHuart, UART_FLAG_TC);
	__HAL_UART_ENABLE_IT(mHuart, UART_FLAG_TC);
	HAL_UART_Transmit_IT(mHuart, TX_Buffer, TX_Pointer);


	return true;
}

bool ModbusMaster::beginMultipleWrite(uint8_t slaveId, uint16_t startAddress)
{
	if(mState == State::Idle)
	{
		mFrame.functionCode = WRITE_MULTIPLE_REGISTERS;
		mFrame.slaveAddress = slaveId;
		mFrame.startAddress = startAddress;
		mFrame.numRegisters = 0;
		mFrame.data.clear();

		mState = State::WritePreparation;
		return true;
	}

	return false;
}

bool ModbusMaster::write(uint16_t data)
{
	if(mState == State::WritePreparation)
	{
		mFrame.data.push_back(data);
		return true;
	}

	return false;
}

bool ModbusMaster::endMultipleWrite()
{
	mFrame.numRegisters = mFrame.data.size();
	buildFrameHeader();

	uint16_t crc = calculateCRC(TX_Buffer, TX_Pointer);

	TX_BufferWrite(crc << 8);
	TX_BufferWrite(crc & 0xFF);

	mState = State::SendRequest;
	__HAL_UART_CLEAR_FLAG(mHuart, UART_FLAG_TC);
	__HAL_UART_ENABLE_IT(mHuart, UART_FLAG_TC);
	HAL_UART_Transmit_IT(mHuart, TX_Buffer, TX_Pointer);

	return true;
}

void ModbusMaster::processFrame()
{
	// Default valid Frame to false
	mFrame.valid = false;

	// Checking Minimum size Frame
	if(RX_Pointer < 5)
	{
		mState = State::Error;
		mError |= Frame_tooShort;
		return;
	}

	// Checking CRC
	uint16_t receivedCRC = (RX_Buffer[RX_Pointer - 2] | (RX_Buffer[RX_Pointer - 1] << 8));
	uint16_t calculatedCRC = calculateCRC(RX_Buffer, RX_Pointer - 2);
	if(receivedCRC != calculatedCRC)
	{
		mState = State::Error;
		mError |= Frame_tooShort;
		return;
	}

	mFrame.slaveAddress = RX_Buffer[0];
	mFrame.functionCode = RX_Buffer[1];

	// Checking supported FunctionCode
	if(mFrame.functionCode != READ_HOLDING_REGISTERS && mFrame.functionCode != WRITE_MULTIPLE_REGISTERS)
	{
		mState = State::Error;
		mError |= Frame_InvalidFC;
		mFrame.valid = false;
		return;
	}

	// Checking Invalid request
	if(mFrame.functionCode && 0x80)
	{
		mState = State::Error;
		mError |= Frame_InvalidRequest;
		mFrame.valid = false;
		return;
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
			return;
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
			return;
		}

		mFrame.data.clear();
		mFrame.startAddress = (RX_Buffer[2] << 8) | RX_Buffer[3];
		mFrame.numRegisters = (RX_Buffer[4] << 8) | RX_Buffer[5];
		mFrame.valid = true;
	}
}

void ModbusMaster::errorHandler()
{
	mLastError = mError;
	mError = 0;

	HAL_UART_AbortTransmit_IT(mHuart);

	__HAL_UART_DISABLE_IT(mHuart, UART_IT_TC);
	__HAL_UART_DISABLE_IT(mHuart, UART_IT_RXNE);
	__HAL_UART_DISABLE_IT(mHuart, UART_IT_IDLE);
}

void ModbusMaster::TX_BufferWrite(uint8_t b)
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

void ModbusMaster::buildFrameHeader()
{
	TX_Pointer = 0;

	TX_BufferWrite(mFrame.slaveAddress);
	TX_BufferWrite(mFrame.functionCode);

	if(mFrame.functionCode == READ_HOLDING_REGISTERS)
	{
		TX_BufferWrite(mFrame.startAddress << 8);
		TX_BufferWrite(mFrame.startAddress & 0xFF);
		TX_BufferWrite(mFrame.numRegisters << 8);
		TX_BufferWrite(mFrame.numRegisters & 0xFF);
	}

	else if(mFrame.functionCode == WRITE_MULTIPLE_REGISTERS)
	{
		TX_BufferWrite(mFrame.startAddress << 8);
		TX_BufferWrite(mFrame.startAddress & 0xFF);
		TX_BufferWrite(mFrame.numRegisters << 8);
		TX_BufferWrite(mFrame.numRegisters & 0xFF);
		TX_BufferWrite(mFrame.numRegisters*2);
	}
}

uint16_t ModbusMaster::calculateCRC(uint8_t *data, uint8_t size)
{
    uint16_t crc = 0xFFFF;
    uint16_t i, j;

    for (i = 0; i < size; i++) {
        crc ^= data[i];
        for (j = 8; j; j--) {
            if (crc & 0x01)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }

    return crc;
}



