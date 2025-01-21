/*
 * ModbusDriver.cpp
 *
 *  Created on: 2 janv. 2025
 *      Author: To
 */

#include "ModbusDriver.hpp"
#include <math.h>

ModbusDriver *ModbusHandler;

ModbusDriver::ModbusDriver()
{
	// Define a specific HardwareSerial as interface

	Serial1 = new ModbusMaster(&huart1);
	Serial2 = new ModbusMaster(&huart2);
	Serial4 = new ModbusMaster(&huart4);
	Serial6 = new ModbusMaster(&huart6);

	//TODO Retrieve from FLASH or EEPROM values
	successRequest = 0;
	failedRequest = 0;
	requestPanicCounter = 0;
	answerPanicCounter = 0;
	successiveFailure = 0;
}


/*
 * This function handle the hadware and process the request and retrieve the answer from the modbus slaves
 */


void ModbusDriver::process()
{
	Serial1->process();
	Serial2->process();
	Serial4->process();
	Serial6->process();

	if(RequestFIFO.size() == 0)
	{
		return;
	}

	while(RequestFIFO.size() > 0)
	{
		ModbusPacket* packet = RequestFIFO.front();
		RequestFIFO.pop_front();

		// Getting packet informations
		uint16_t startAddress = packet->registers[0].address;
		uint16_t blockSize = packet->registers.size();
		bool direction = packet->direction;
		uint8_t slaveID = packet->slave;

		/*
		 * Writing multiple registers and handling error information
		 */
		if(direction == ModbusPacket::Write)
		{
			packet->success = true;

			ModbusMaster* client = getInterface(packet->slave);

			// Begin transmission for first register
			blockSize = calculateBlockSize(packet->registers, 0, startAddress);
			packet->success &= client->beginMultipleWrite(slaveID, startAddress);

			for(uint8_t i = 0; i < packet->registers.size(); i++)
			{
				uint16_t previousAddress = i == 0 ? startAddress : packet->registers[i-1].address;
				// Next address is not contiguous
				// Ending transmission, checking for error and starting new transmission
				if(abs(packet->registers[i].address - previousAddress) > 1)
				{
					client->endMultipleWrite();
					while(client->busy());

					packet->success = client->lastRequestStatus();

					if(!packet->success)
					{
						failedRequest++;
						successiveFailure++;
						//TODO Warn about a failed transmission
					}

					else
					{
						successRequest++;
						successiveFailure = 0;
					}

					packet->success &= client->beginMultipleWrite(slaveID, startAddress);

				}

				// Writing words to bus
				packet->success &= client->write(packet->registers[i].value);
			}

			client->endMultipleWrite();
			while(client->busy());

			packet->success = client->lastRequestStatus();

			if(!packet->success)
			{
				failedRequest++;
				successiveFailure++;
				//TODO Warn about a failed transmission
			}

			else
			{
				successRequest++;
				successiveFailure = 0;
			}
		}

		if(direction == ModbusPacket::Read)
		{
			packet->success = true;

			ModbusMaster* client = getInterface(packet->slave);
			client->readHoldingRegister(slaveID, startAddress, blockSize);
			while(client->busy());

			if(client->available() == blockSize)
			{
				for(uint8_t i = 0; i < blockSize; i++)
				{
					uint16_t read = client->read();
					if(read == -1)
					{
						failedRequest++;
						successiveFailure++;
						packet->success = false;
						//TODO Warn about invalid read
						break;
					}
					else
						packet->registers[i].value = read;
				}

				successRequest++;
				successiveFailure = 0;
			}

			else
			{
				packet->success = false;
				failedRequest++;
				successiveFailure++;

				//TODO Warn about invalid read
			}

		}

		/*
		 * Transferring packet to Answer FIFO with results
		 */
		if(AnswerFIFO.size() >= PANIC_FIFO_SIZE)
		{
			answerPanicCounter++;
			//TODO Use logger to warn user about a Panic situation
		}

		AnswerFIFO.push_back(packet);
	}
}

bool ModbusDriver::request(ModbusPacket *packet)
{
	if(RequestFIFO.size() >= PANIC_FIFO_SIZE)
	{
		requestPanicCounter++;
		//TODO Use logger to warn user about a Panic situation
		return false;
	}

	if(packet->registers.size() > 0)
	{
		RequestFIFO.push_back(packet);
	}

	else
	{
		requestPanicCounter++;
		//TODO Use logger to warn about a malformed packet
		return false;
	}

	return true;

}

ModbusPacket* ModbusDriver::getAnswer(uint8_t slaveID)
{
	if(AnswerFIFO.size() == 0)
		return nullptr;

	if(AnswerFIFO.front()->slave == slaveID)
	{
		ModbusPacket* packet = AnswerFIFO.front();
		AnswerFIFO.pop_front();
		return packet;
	}

	else
		return nullptr;
}

uint8_t ModbusDriver::available(uint8_t slaveID)
{
	uint8_t count = 0;

	if(AnswerFIFO.size() == 0)
		return count;

	for(uint8_t i = 0; AnswerFIFO.size(); i++)
	{
		if(AnswerFIFO[i]->slave == slaveID)
			count++;
	}

	return count;
}


ModbusMaster* ModbusDriver::getInterface(uint8_t slaveID)
{
	return Serial6;
}

uint16_t ModbusDriver::calculateBlockSize(const std::vector<Register>& registers, uint8_t startIndex, uint16_t startAddress)
{
    uint16_t size = 0;

    for (uint8_t i = startIndex; i < registers.size(); i++)
    {
        if (abs(registers[i].address - startAddress) > 1)
        {
            break;
        }
        size++;
        startAddress = registers[i].address;
    }
    return size;
}
