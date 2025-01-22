/*
 * ModbusMaster.cpp
 *
 *  Created on: 2 janv. 2025
 *      Author: To
 */

#include "ModbusMaster.hpp"

#include <math.h>

ModbusMaster ModbusHandler;


ModbusMaster::ModbusMaster()
{
	// Define a specific HardwareSerial as interface
	//TODO Retrieve from FLASH or EEPROM values
	successRequest = 0;
	failedRequest = 0;
	requestPanicCounter = 0;
	answerPanicCounter = 0;
	successiveFailure = 0;

	// Initalizing and starting ModbusDriver tasks

	mInterfaces[0].modbus = &Serial1;
	mInterfaces[1].modbus = &Serial2;
	mInterfaces[2].modbus = &Serial4;
	mInterfaces[3].modbus = &Serial6;
}


/*
 * This function handle the drivers and process the request and retrieve the answer from the modbus slaves
 */

void ModbusMaster::setup()
{
	Serial6.start("Modbus_UART6", 128, osPriorityHigh);
}


void ModbusMaster::run()
{
	if(RequestFIFO.size() == 0)
	{
		suspend();
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

			Interface* interface = getInterface(packet->slave);

			// Begin transmission for first register
			packet->success &= interface->modbus->beginMultipleWrite(slaveID, startAddress, &interface->DataReadySemaphore);

			for(uint8_t i = 0; i < packet->registers.size(); i++)
			{
				uint16_t previousAddress = i == 0 ? startAddress : packet->registers[i-1].address;
				// Next address is not contiguous
				// Ending transmission, checking for error and starting new transmission
				if(abs(packet->registers[i].address - previousAddress) > 1)
				{
					xSemaphoreTake(interface->DataReadySemaphore, 0);
					interface->modbus->endMultipleWrite();
					if(xSemaphoreTake(interface->DataReadySemaphore, pdMS_TO_TICKS(1000)) != pdTRUE)
					{
						failedRequest++;
						successiveFailure++;
						packet->success = false;
						//TODO Warn about invalid read
						break;
					}

					packet->success = interface->modbus->lastRequestStatus();

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

					packet->success &= interface->modbus->beginMultipleWrite(slaveID, startAddress, &interface->DataReadySemaphore);

				}

				// Writing words to bus
				packet->success &= interface->modbus->write(packet->registers[i].value);
			}

			interface->modbus->endMultipleWrite();
			if(xSemaphoreTake(interface->DataReadySemaphore, pdMS_TO_TICKS(1000)) != pdTRUE)
			{
				failedRequest++;
				successiveFailure++;
				packet->success = false;
				//TODO Warn about invalid read
				break;
			}

			packet->success = interface->modbus->lastRequestStatus();

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

			Interface* interface = getInterface(packet->slave);

			xSemaphoreTake(interface->DataReadySemaphore, 0);
			interface->modbus->readHoldingRegister(slaveID, startAddress, blockSize, &interface->DataReadySemaphore);
			if(xSemaphoreTake(interface->DataReadySemaphore, pdMS_TO_TICKS(1000)) != pdTRUE)
			{
				failedRequest++;
				successiveFailure++;
				packet->success = false;
				//TODO Warn about invalid read
				break;
			}

			if(interface->modbus->available() == blockSize)
			{
				for(uint8_t i = 0; i < blockSize; i++)
				{
					uint16_t read = interface->modbus->read();
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

bool ModbusMaster::request(ModbusPacket *packet)
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
		resume();
	}

	else
	{
		requestPanicCounter++;
		//TODO Use logger to warn about a malformed packet
		return false;
	}

	return true;

}

ModbusPacket* ModbusMaster::response(uint8_t slaveID)
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

uint8_t ModbusMaster::available(uint8_t slaveID)
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

ModbusMaster::Interface* ModbusMaster::getInterface(uint8_t slaveID)
{
	return &mInterfaces[3];
}

uint16_t ModbusMaster::calculateBlockSize(const std::vector<Register>& registers, uint8_t startIndex, uint16_t startAddress)
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
