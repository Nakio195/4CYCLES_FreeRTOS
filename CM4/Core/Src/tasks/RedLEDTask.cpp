/*
 * RedLEDTask.cpp
 *
 *  Created on: Jan 21, 2025
 *      Author: To
 */

#include "RedLEDTask.h"

RedLED_Task::RedLED_Task() : RTOS_Task()
{
	// TODO Auto-generated constructor stub

}


void RedLED_Task::setup()
{

}

void RedLED_Task::run()
{
	HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
	vTaskDelay(pdMS_TO_TICKS(1000));
}
