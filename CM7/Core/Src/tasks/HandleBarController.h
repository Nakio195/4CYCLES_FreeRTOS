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


class HandleBarController: public CanPeripheral, public RTOS_Task, public Controller
{

	public:
		HandleBarController();

		void setup() override;
		void run() override;
		void cleanup() override;

		void init() override;
		void reInit();
		void recovery();
		void absent();
		void recovered();
		void lost();

	private:
		void ControllerStatus(CanPacket* packet);
		void ControllerData(CanPacket* packet);

	private:
		SemaphoreHandle_t Mut_Data;
		//Peripheral control
		uint32_t mPreviousTick = 0;

		uint8_t mThrottle;
		uint8_t mBrake;
		int32_t mSteering;

		Switch mBrakeSwitch;
		Switch mLightsSwitch;
		Switch mParkBrakeSwitch;
		Switch mWarningSwitch;
		Switch mHornSwitch;
		Switch mTurnLSwitch;
		Switch mTurnRSwitch;
		Switch mReverseSwitch;

};

extern HandleBarController HandleBarTask;

#endif /* SRC_TASKS_PS3CONTROLLER_H_ */
