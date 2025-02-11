/*
 * PS3Controller.h
 *
 *  Created on: Feb 3, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_PS3CONTROLLER_H_
#define SRC_TASKS_PS3CONTROLLER_H_

#include "RTOSTask.h"
#include "queue.h"
#include "semphr.h"
#include "utils/CanPeripheral.h"
#include "CANTask.h"

namespace PS3
{
		struct Status
		{
			uint8_t battery = 0;
			bool connected = 0;
			uint32_t timestamp = 0;
		};

		struct Stick
		{
			int8_t x = 0;
			int8_t y = 0;
		};

		struct Sticks
		{
			Stick L;
			Stick R;
		};

		struct Triggers
		{
			uint8_t L = 0;
			uint8_t R = 0;

		};

		struct Pad
		{
			bool up = 0;
			bool down = 0;
			bool left = 0;
			bool right = 0;
		};

		struct Buttons
		{
			bool circle = 0;
			bool cross = 0;
			bool square = 0;
			bool triangle = 0;
			bool R1 = 0;
			bool L1 = 0;
			bool L3 = 0;
			bool R3 = 0;
			bool select = 0;
			bool start = 0;
			bool ps = 0;
		};

		struct Data
		{
			Pad dpad;
			Buttons buttons;
			Sticks sticks;
			Triggers trig;
			Status status;
		};
}

class PS3Controller: public CanPeripheral, public RTOS_Task
{
	public:
		enum BatteryLevels{Undefined, Shutdown, Dying, Low, High, Full, Charging};

	public:
		PS3Controller();

		void setup() override;
		void run() override;
		void cleanup() override;

		void init() override;
		void reInit();
		void recovery();
		void absent();

		PS3::Data getData();

	private:
		void ControllerStatus(CanPacket* packet);
		void ControllerData(CanPacket* packet);

	private:
		SemaphoreHandle_t Mut_Data;
		//Peripheral control
		PS3::Data controller;

};

extern PS3Controller MainController;

#endif /* SRC_TASKS_PS3CONTROLLER_H_ */
