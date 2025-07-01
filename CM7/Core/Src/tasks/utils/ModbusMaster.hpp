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
#include "cmsis_os2.h"

#include "PhaserunnerRegisterMap.h"

#define PANIC_FIFO_SIZE 12

class ModbusPacket
{
	public:
		explicit ModbusPacket(uint16_t slaveID, bool dir = Read) : slave(slaveID), direction(dir)
		{
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

		std::deque<ModbusPacket*> RequestFIFO;
		std::deque<ModbusPacket*> AnswerFIFO;

	private:
		Interface mInterfaces[4];
		Interface* getInterface(uint8_t slaveID);
		uint16_t calculateBlockSize(const std::vector<Register>& registers, uint8_t startIndex, uint16_t startAddress);
};

extern ModbusMaster ModbusHandler;

#endif /* ModbusMaster_H_ */
