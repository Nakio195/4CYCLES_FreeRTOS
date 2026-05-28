/*
 * BatteryTask.cpp
 *
 *  Created on: Dec 8, 2025
 *      Author: To
 */

#include "BatteryTask.h"

BatteryTask BatteryHandler;

BatteryTask::BatteryTask()
{
	// TODO Auto-generated constructor stub
	setRangeFilter(0x18FF0009, 0x18FF1591);
	setCommunicationTimeout(200);
	setRecoveryMode(5, 100);

	Mut_Data = xSemaphoreCreateMutex();
	xSemaphoreGive(Mut_Data);

	mPeripheralType = Battery;

	CanHandler.attach(this);
}

// ############ RTOS Task Methods ############

void BatteryTask::setup()
{

}

void BatteryTask::run()
{
	CanPacket* packet = nullptr;
	if(xQueueReceive(mPacketsQueue, &packet, 0) == pdTRUE)
	{
		if(packet != nullptr)
		{
			switch(packet->Identifier)
			{
				case 0x18FF0080: processBatteryVoltage(packet); break;
				case 0x18FF0880: processBatteryCurrent(packet); break;
				case 0x18FF0980: processBatteryState(packet); break;
				case 0x18FF0B80: processBatteryTemperature(packet); break;
				case 0x18FF1180: processBatteryErrors(packet); break;
				case 0x18FF1480: processBatteryWarnings(packet); break;
				case 0x18FF1380: processBatteryChargeState(packet); break;
				default: break;
			}
			CanPacketPool.free(packet);
		}
	}

	tick(xTaskGetTickCount());
	osDelay(10);
}

void BatteryTask::cleanup()
{

}

// ############ CAN packets Methods ############

void BatteryTask::processBatteryState(CanPacket* packet)
{

}

void BatteryTask::processBatteryWarnings(CanPacket* packet)
{
	mWarnings.WarningDataLogger = packet->data[1] & 0x40;
	mWarnings.WarningBluetooth = packet->data[1] & 0x20;
	mWarnings.WarningNTC = packet->data[1] & 0x10;
	mWarnings.WarningHighVoltage = packet->data[1] & 0x08;
	mWarnings.WarningLowVoltage = packet->data[1] & 0x04;
	mWarnings.WarningCurrentCalibration = packet->data[1] & 0x02;
	mWarnings.WarningShortCircuitOnPrecharge = packet->data[1] & 0x01;
	mWarnings.WarningAFEIssues = packet->data[2] & 0x80;
	mWarnings.WarningSlaveSilent = packet->data[2] & 0x40;
	mWarnings.WarningShortCircuit = packet->data[2] & 0x20;
	mWarnings.WarningCellIssues = packet->data[2] & 0x10;
	mWarnings.WarningChargerIssue = packet->data[2] & 0x08;
	mWarnings.WarningBmsOverTemperature = packet->data[2] & 0x04;
	mWarnings.WarningPduOverTemperature = packet->data[2] & 0x02;
	mWarnings.WarningPduFailure = packet->data[2] & 0x01;
	mWarnings.WarningPDULock = packet->data[3] & 0x80;
	mWarnings.WarningOverCurrent = packet->data[3] & 0x40;
	mWarnings.WarningOverVoltage = packet->data[3] & 0x20;
	mWarnings.WarningUnderVoltage = packet->data[3] & 0x10;
	mWarnings.WarningOverTemperatureDischarge = packet->data[3] & 0x08;
	mWarnings.WarningUnderTemperatureDischarge = packet->data[3] & 0x04;
	mWarnings.WarningOverTemperatureCharge = packet->data[3] & 0x02;
	mWarnings.WarningUnderTemperatureCharge = packet->data[3] & 0x01;
}

void BatteryTask::processBatteryErrors(CanPacket* packet)
{
	mErrors.ErrorDataLogger = packet->data[1] & 0x40;
	mErrors.ErrorBluetooth = packet->data[1] & 0x20;
	mErrors.ErrorNTC = packet->data[1] & 0x10;
	mErrors.ErrorHighVoltage = packet->data[1] & 0x08;
	mErrors.ErrorLowVoltage = packet->data[1] & 0x04;
	mErrors.ErrorCurrentCalibration = packet->data[1] & 0x02;
	mErrors.ErrorShortCircuitOnPrecharge = packet->data[1] & 0x01;
	mErrors.ErrorAFEIssues = packet->data[2] & 0x80;
	mErrors.ErrorSlaveSilent = packet->data[2] & 0x40;
	mErrors.ErrorShortCircuit = packet->data[2] & 0x20;
	mErrors.ErrorCellIssues = packet->data[2] & 0x10;
	mErrors.ErrorChargerIssue = packet->data[2] & 0x08;
	mErrors.ErrorBmsOverTemperature = packet->data[2] & 0x04;
	mErrors.ErrorPduOverTemperature = packet->data[2] & 0x02;
	mErrors.ErrorPduFailure = packet->data[2] & 0x01;
	mErrors.ErrorPDULock = packet->data[3] & 0x80;
	mErrors.ErrorOverCurrent = packet->data[3] & 0x40;
	mErrors.ErrorOverVoltage = packet->data[3] & 0x20;
	mErrors.ErrorUnderVoltage = packet->data[3] & 0x10;
	mErrors.ErrorOverTemperatureDischarge = packet->data[3] & 0x08;
	mErrors.ErrorUnderTemperatureDischarge = packet->data[3] & 0x04;
	mErrors.ErrorOverTemperatureCharge = packet->data[3] & 0x02;
	mErrors.ErrorUnderTemperatureCharge = packet->data[3] & 0x01;
}

void BatteryTask::processBatteryChargeState(CanPacket* packet)
{

}

void BatteryTask::processBatteryTemperature(CanPacket* packet)
{

}

void BatteryTask::processBatteryVoltage(CanPacket* packet)
{
	mStatus.voltage = 0.001 * float(uint32_t(packet->data[4] | (packet->data[5] << 8) | (packet->data[6] << 16) | (packet->data[7] << 24)));
	mStatus.minCellVoltage = 0.001 * float(uint16_t(packet->data[0] | (packet->data[1] << 8)));
	mStatus.maxCellVoltage = 0.001 * float(uint16_t(packet->data[2] | (packet->data[3] << 8)));
}

void BatteryTask::processBatteryCurrent(CanPacket* packet)
{

}


// ############ CAN Peripheral Methods ############

void BatteryTask::init()
{

}

void BatteryTask::reInit()
{

}

void BatteryTask::discovered()
{

}

void BatteryTask::recovery()
{

}

void BatteryTask::absent()
{

}

void BatteryTask::recovered()
{

}

void BatteryTask::lost()
{

}

BatteryStatus BatteryTask::status()
{
	return mStatus;
}
