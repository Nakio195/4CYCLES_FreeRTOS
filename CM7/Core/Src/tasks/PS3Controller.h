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
#include "utils/Controller.h"
#include "utils/Switch.h"

namespace PS3
{
		struct Status
		{
			uint8_t battery = 0;
			Switch connected;
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
			Switch L2;
			Switch R2;

		};

		struct Pad
		{
			Switch up;
			Switch down;
			Switch left;
			Switch right;
		};

		struct Buttons
		{
			Switch circle;
			Switch cross;
			Switch square;
			Switch triangle;
			Switch R1;
			Switch L1;
			Switch L3;
			Switch R3;
			Switch select;
			Switch start;
			Switch ps;
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

class PS3Controller: public CanPeripheral, public RTOS_Task, public Controller
{
	public:
		enum BatteryLevels{Undefined, Shutdown, Dying, Low, High, Full, Charging};

	public:
		PS3Controller();

		void setup() override;
		void run() override;
		void cleanup() override;

	private:
		void onInit() override;
		void onDiscovered() override;
		void onReady() override;
		void onRecovery() override;
		void onAbsent() override;
		void onRecovered() override;
		void onLost() override;
		void onDisabled() override;

	private:
		void ControllerStatus(CanPacket* packet);
		void ControllerData(CanPacket* packet);

	private:
		SemaphoreHandle_t Mut_Data;
		//Peripheral control
		PS3::Data controller;
		uint32_t mPreviousTick	 = 0;

};

extern PS3Controller PS3Task;

#endif /* SRC_TASKS_PS3CONTROLLER_H_ */
