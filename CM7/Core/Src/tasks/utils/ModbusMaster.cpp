/*
 * ModbusMaster.cpp
 *
 *  Created on: 2 janv. 2025
 *      Author: To
 */

#include "ModbusMaster.hpp"

#include <math.h>

ModbusMaster ModbusHandler;
ModbusPacketPoolHandler ModbusPacketPool;

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

	mInterfaces[0].modbus = &Serial2;
	mInterfaces[1].modbus = &Serial6;
	mInterfaces[2].modbus = &Serial4;
	mInterfaces[3].modbus = &Serial1;

	RequestFIFO = xQueueCreate(200, sizeof(ModbusPacket*));
	vQueueAddToRegistry(RequestFIFO, "ModBusRequest");
	AnswerFIFO = xQueueCreate(200, sizeof(ModbusPacket*));
	vQueueAddToRegistry(AnswerFIFO, "ModBusAnswer");
	responseMutex = xSemaphoreCreateMutex();
}


/*
 * This function handle the drivers and process the request and retrieve the answer from the modbus slaves
 */

void ModbusMaster::setup()
{
	Serial2.start("Modbus_UART2", 128, osPriorityHigh7);
	Serial6.start("Modbus_UART6", 128, osPriorityHigh7);
	Serial4.start("Modbus_UART4", 128, osPriorityHigh7);
	Serial1.start("Modbus_UART1", 128, osPriorityHigh7);
}


void ModbusMaster::run()
{
	ModbusPacket* packet = nullptr;


	if(xQueueReceive(RequestFIFO, &packet, 100) == pdTRUE)
	{
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
				if(abs((int32_t)(packet->registers[i].address) - (int32_t)(previousAddress)) > 1)
				{
					xSemaphoreTake(interface->DataReadySemaphore, 0);
					interface->modbus->endMultipleWrite();
					if(xSemaphoreTake(interface->DataReadySemaphore, pdMS_TO_TICKS(100)) != pdTRUE)
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

					packet->success &= interface->modbus->beginMultipleWrite(slaveID, packet->registers[i].address, &interface->DataReadySemaphore);

				}

				// Writing words to bus
				packet->success &= interface->modbus->write(packet->registers[i].value);
			}

			if(packet->success)
			{
				xSemaphoreTake(interface->DataReadySemaphore, 0);
				interface->modbus->endMultipleWrite();
				if(xSemaphoreTake(interface->DataReadySemaphore, pdMS_TO_TICKS(5)) != pdTRUE)
				{
					failedRequest++;
					successiveFailure++;
					packet->success = false;
					//TODO Warn about invalid read
				}
				else
				{
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
			}
		}

		if(direction == ModbusPacket::Read)
		{
			packet->success = true;

			Interface* interface = getInterface(packet->slave);

			xSemaphoreTake(interface->DataReadySemaphore, 0);
			interface->modbus->readHoldingRegister(slaveID, startAddress, blockSize, &interface->DataReadySemaphore);
			if(xSemaphoreTake(interface->DataReadySemaphore, pdMS_TO_TICKS(5)) != pdTRUE)
			{
				failedRequest++;
				successiveFailure++;
				packet->success = false;
				//TODO Warn about invalid read

			}

			else
			{
				if(interface->modbus->available() == blockSize)
				{
					for(uint8_t i = 0; i < blockSize; i++)
					{
						int16_t read = interface->modbus->read();
						if(read == -1)
						{
							failedRequest++;
							successiveFailure++;
							packet->success = false;
							//TODO Warn about invalid read
							break;
						}
						else
							packet->registers[i].value = (uint16_t)read;
					}

					if(packet->success)
					{
						successRequest++;
						successiveFailure = 0;
					}
				}

				else
				{
					packet->success = false;
					failedRequest++;
					successiveFailure++;

					//TODO Warn about invalid read
				}
			}
		}

		/*
		 * Transferring packet to Answer FIFO with results
		 */

		if (packet->success)
		{
		    if (xSemaphoreTake(responseMutex, pdMS_TO_TICKS(10)) == pdTRUE)
		    {
		        if (xQueueSend(AnswerFIFO, &packet, 5) != pdTRUE)
		        {
		            answerPanicCounter++;
		            //TODO log warning
		            ModbusPacketPool.free(packet); // Libérer si FIFO pleine
		        }
		        xSemaphoreGive(responseMutex);
		    }
		    else
		    {
		        // Mutex non obtenu : gérer l’erreur (log, delete packet ?)
	            answerPanicCounter++;
		    	ModbusPacketPool.free(packet);
		    }
		}

		else
		{
			ModbusPacketPool.free(packet); // Free memory if request failed
		}
	}

	//osDelay(1);


}

bool ModbusMaster::request(ModbusPacket *packet)
{
    return (xQueueSend(RequestFIFO, &packet, 5) == pdTRUE);
}

ModbusPacket* ModbusMaster::response(uint8_t slaveID)
{
    ModbusPacket* foundPacket = nullptr;
    ModbusPacket* tempPacket = nullptr;
    std::vector<ModbusPacket*> tempBuffer;

    if(xSemaphoreTake(responseMutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        while(xQueueReceive(AnswerFIFO, &tempPacket, 0) == pdTRUE)
        {
            if(tempPacket->slave == slaveID && foundPacket == nullptr)
            {
                foundPacket = tempPacket;
            }
            else
            {
                tempBuffer.push_back(tempPacket);
            }
        }

        for(auto pkt : tempBuffer)
        {
            if(xQueueSend(AnswerFIFO, &pkt, 0) != pdTRUE)
            {
            	ModbusPacketPool.free(pkt);
            }
        }

        xSemaphoreGive(responseMutex);
    }
    return foundPacket;
}

uint8_t ModbusMaster::available(uint8_t slaveID)
{
    uint8_t count = 0;
    ModbusPacket* tempPacket = nullptr;
    std::vector<ModbusPacket*> tempBuffer;

    if(xSemaphoreTake(responseMutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        while(xQueueReceive(AnswerFIFO, &tempPacket, 0) == pdTRUE)
        {
            if(tempPacket->slave == slaveID)
            {
                count++;
            }
            tempBuffer.push_back(tempPacket);
        }

        for(auto pkt : tempBuffer)
        {
            if(xQueueSend(AnswerFIFO, &pkt, 0) != pdTRUE)
            	ModbusPacketPool.free(pkt);
        }

        xSemaphoreGive(responseMutex);
    }
    return count;
}

ModbusMaster::Interface* ModbusMaster::getInterface(uint8_t slaveID)
{
	return &mInterfaces[slaveID-1];
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
