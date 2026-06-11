/*
 * VehicleTask.h
 *
 *  Created on: Feb 3, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_VEHICLETASK_H_
#define SRC_TASKS_VEHICLETASK_H_

#include "RTOSTask.h"
#include "LoggerTask.h"
#include "HandleBarController.h"
#include "PS3Controller.h"
#include "DirectionTask.h"
#include "BatteryTask.h"
#include "usbd_cdc_if.h"
#include "usart.h"

#include "FreeRTOS.h"
#include "timers.h"

#include "utils/filters/FilterChain.h"
#include "utils/filters/LowPassFilter.h"
#include "utils/filters/SCurveFilter.h"
#include "utils/filters/ThresholdFilter.h"
#include "utils/filters/LUTs.h"
#include "utils/Phaserunner.hpp"
#include "utils/tools/Timer.hpp"
#include "utils/Event.h"

#include <string>

#define VEHICLE_TICK_MS 10

class VehicleTask : public RTOS_Task
{
	public:
		VehicleTask();

		void setup() override;
		void run() override;
		void cleanup() override;

		void handleLightsAction(Action* action);

		void engageMotor();
		void disengageMotor();

		void engageParking();
		void disengageParking();

		MotorInfo getMotorInfo(Phaserunner &motor)
		{
			return motor.getMotorInfo();
		}

		void setMotorSpeed(float speed, bool reverse = false);
		void setMotorEBrake(float brake);

	private:
		void updateControllerSelection();
		void changeController(CanPeripheral::PeripheralId);

		void enterControllerError();
		float getMeanSpeed();
		float getMaxSpeed();

	    void updateTimers();
	    void processEvents();
	    void processActions();
	    void compute();
	    void updateVehicle();

		int32_t normalizeSterring(int32_t);

	private:
		QueueHandle_t mControllerQueue;

		//Controllers
		uint8_t mControllersAvailables;
		bool mRemoteControllerAvailable;
		bool mLocalControllerAvailable;
		CanPeripheral::PeripheralId mCurrentController;

		//Motors
		bool mMotorEngaged; // true if motor is engaged
		bool mParkingEngaged;

		//Direction values
		int32_t mSteeringCommand_AV;
		int32_t mSteeringCommand_AR;

		//Motor Reverse
		bool mMotorReversePending; // true if reverse gear is pending engagement
		bool mMotorReversePendingValue; // true if reverse gear is pending value change
		bool mMotorReverseEngaged; // true if reverse gear is engaged
		// Filters
		FilterChain mThrottle;
		FilterChain mBrake;
		uint8_t mRawThottle;
		uint8_t mRawBrake;

		//Motor Zero crossing detection
		bool mZeroCrossing;

		// Thread Control
		Timer mLogDynamicsTimer;
		Timer mMotorUpdateTimer;
		uint32_t mLastWakeTime;
		Timer mHeapStatsTimer;

		size_t freeHeap;
		size_t minEver;
		HeapStats_t stats;

};

extern VehicleTask Vehicle;

#endif /* SRC_TASKS_VEHICLETASK_H_ */
