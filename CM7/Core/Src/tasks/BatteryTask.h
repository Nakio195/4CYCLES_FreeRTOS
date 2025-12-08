/*
 * BatteryTask.h
 *
 *  Created on: Dec 8, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_BATTERYTASK_H_
#define SRC_TASKS_BATTERYTASK_H_

#include "RTOSTask.h"
#include "utils/CanPeripheral.h"
#include "queue.h"
#include "semphr.h"
#include "utils/CanPeripheral.h"
#include "CANTask.h"

struct BatteryStatus
{
	uint8_t stateOfCharge; // Percentage 0-100%
	uint8_t stateOfHealth; // Percentage 0-100%
	uint16_t cycleCount;   // Number of charge/discharge cycles
	float voltage;         // Volts
	float minCellVoltage; // Volts
	float maxCellVoltage; // Volts
	float current;         // Amperes
	float meanTemp;     // Celsius
	float lowTemp;     // Celsius
	float highTemp;     // Celsius
	bool isCharging;
};

struct BatteryErrors
{
	bool ErrorDataLogger;
	bool ErrorBluetooth;
	bool ErrorNTC;
	bool ErrorHighVoltage;
	bool ErrorLowVoltage;
	bool ErrorCurrentCalibration;
	bool ErrorShortCircuitOnPrecharge;
	bool ErrorAFEIssues;
	bool ErrorSlaveSilent;
	bool ErrorShortCircuit;
	bool ErrorCellIssues;
	bool ErrorChargerIssue;
	bool ErrorBmsOverTemperature;
	bool ErrorPduOverTemperature;
	bool ErrorPduFailure;
	bool ErrorPDULock;
	bool ErrorOverCurrent;
	bool ErrorOverVoltage;
	bool ErrorUnderVoltage;
	bool ErrorOverTemperatureDischarge;
	bool ErrorUnderTemperatureDischarge;
	bool ErrorOverTemperatureCharge;
	bool ErrorUnderTemperatureCharge;

	bool fatal()
	{
		return ErrorHighVoltage || ErrorLowVoltage || ErrorShortCircuit
				|| ErrorOverCurrent || ErrorOverVoltage || ErrorUnderVoltage
				|| ErrorOverTemperatureDischarge || ErrorUnderTemperatureDischarge
				|| ErrorOverTemperatureCharge || ErrorUnderTemperatureCharge
				|| ErrorPduFailure;
	}
};


struct BatteryWarnings
{
	bool WarningDataLogger;
	bool WarningBluetooth;
	bool WarningNTC;
	bool WarningHighVoltage;
	bool WarningLowVoltage;
	bool WarningCurrentCalibration;
	bool WarningShortCircuitOnPrecharge;
	bool WarningAFEIssues;
	bool WarningSlaveSilent;
	bool WarningShortCircuit;
	bool WarningCellIssues;
	bool WarningChargerIssue;
	bool WarningBmsOverTemperature;
	bool WarningPduOverTemperature;
	bool WarningPduFailure;
	bool WarningPDULock;
	bool WarningOverCurrent;
	bool WarningOverVoltage;
	bool WarningUnderVoltage;
	bool WarningOverTemperatureDischarge;
	bool WarningUnderTemperatureDischarge;
	bool WarningOverTemperatureCharge;
	bool WarningUnderTemperatureCharge;
};

class BatteryTask: public CanPeripheral, public RTOS_Task
{
	public:
		BatteryTask();

		void setup() override;
		void run() override;
		void cleanup() override;

		void init() override;
		void reInit();
		void discovered();
		void recovery();
		void absent();
		void recovered();
		void lost();

		BatteryStatus status();

	private:
		void processBatteryState(CanPacket* packet);
		void processBatteryWarnings(CanPacket* packet);
		void processBatteryErrors(CanPacket* packet);
		void processBatteryChargeState(CanPacket* packet);
		void processBatteryTemperature(CanPacket* packet);
		void processBatteryVoltage(CanPacket* packet);
		void processBatteryCurrent(CanPacket* packet);


		SemaphoreHandle_t Mut_Data;
		BatteryStatus mStatus;
		BatteryWarnings mWarnings;
		BatteryErrors mErrors;
};

extern BatteryTask BatteryHandler;

#endif /* SRC_TASKS_BATTERYTASK_H_ */
