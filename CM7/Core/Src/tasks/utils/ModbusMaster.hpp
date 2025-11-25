/*
 * ModbusMaster.h
 *
 *  Created on: 2 janv. 2025
 *      Author: To
 */

#ifndef ModbusMaster_H_
#define ModbusMaster_H_

#include "drivers/ModbusDriver.h"

#include <stdint.h>
#include <deque>
#include <vector>

#include "../RTOSTask.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "cmsis_os2.h"

#include "PhaserunnerRegisterMap.h"

#define PANIC_FIFO_SIZE 12
#define MODBUS_POOL_SIZE 250

class ModbusPacket
{
	public:
		explicit ModbusPacket(uint16_t slaveID = 5, bool dir = Read) : slave(slaveID), direction(dir)
		{
		}

		void reset(uint16_t slaveID, bool dir = Read)
		{
	        slave = slaveID;
	        direction = dir;
	        success = false;
	        registers.clear();  // Vide l’ancien contenu
		}

		void push(const Register& r)
		{
			registers.push_back(r);
		}

	public:
		enum {Read = 0, Write = 1};
		uint8_t slave; // ID of the recipient
		bool direction; // Read = false, Write = true

		std::vector<Register> registers = {};

		bool success = 0;
};

class ModbusPacketPoolHandler
{
	private:
		ModbusPacket pool[MODBUS_POOL_SIZE];
		bool used[MODBUS_POOL_SIZE];        // Indique si le slot est utilisé
		SemaphoreHandle_t mutex;

		uint16_t minPoolSizeEver;
		uint16_t currentPoolUse;

	public:
		ModbusPacketPoolHandler()
		{
			mutex = xSemaphoreCreateMutex();
			for (int i = 0; i < MODBUS_POOL_SIZE; i++)
				used[i] = false;
			minPoolSizeEver = MODBUS_POOL_SIZE; // Initialisation à la taille maximale
			currentPoolUse = 0;
		}

		ModbusPacket* allocate(uint16_t slaveID, bool dir = ModbusPacket::Read)
		{
			ModbusPacket* pkt = nullptr;
			if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
			{
				for (int i = 0; i < MODBUS_POOL_SIZE; i++)
				{
					if (!used[i])
					{
						used[i] = true;
						pkt = &pool[i];
						currentPoolUse++;
						if (MODBUS_POOL_SIZE - currentPoolUse < minPoolSizeEver)
							minPoolSizeEver = MODBUS_POOL_SIZE - currentPoolUse;
						break;
					}
				}
				xSemaphoreGive(mutex);
			}
			if(pkt != nullptr)
				pkt->reset(slaveID, dir);

			assert(pkt != nullptr); // Ensure that the request was allocated successfully

			return pkt; // nullptr si pool plein
		}

		void free(ModbusPacket* pkt)
		{
			if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
			{
				uint32_t index = pkt - pool; // calcul index
				if (index >= 0 && index < MODBUS_POOL_SIZE && used[index])
				{
					used[index] = false;
					currentPoolUse--;
				}
				xSemaphoreGive(mutex);
			}
		}
};

class ModbusMaster : public RTOS_Task
{

	public:
		struct Interface
		{
			ModbusDriver* modbus = nullptr;
			SemaphoreHandle_t DataReadySemaphore = xSemaphoreCreateBinary();
		};

	public:
		ModbusMaster();

		void setup() override;
		void run() override;

		bool request(ModbusPacket *packet);
		ModbusPacket* response(uint8_t slaveID);
		uint8_t available(uint8_t slaveID);

		uint32_t successRequest;
		uint32_t failedRequest;

		uint16_t requestPanicCounter;
		uint16_t answerPanicCounter;

		uint8_t successiveFailure;

		QueueHandle_t RequestFIFO;
		QueueHandle_t AnswerFIFO;

	private:
		SemaphoreHandle_t responseMutex;
		SemaphoreHandle_t answerMutex;

		Interface mInterfaces[4];
		Interface* getInterface(uint8_t slaveID);
		uint16_t calculateBlockSize(const std::vector<Register>& registers, uint8_t startIndex, uint16_t startAddress);
};

extern ModbusMaster ModbusHandler;
extern ModbusPacketPoolHandler ModbusPacketPool;

#endif /* ModbusMaster_H_ */
