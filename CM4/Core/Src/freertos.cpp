/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for initTask */
osThreadId_t initTaskHandle;
const osThreadAttr_t initTask_attributes = {
  .name = "initTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ModbusTask */
osThreadId_t ModbusTaskHandle;
const osThreadAttr_t ModbusTask_attributes = {
  .name = "ModbusTask",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for MotorTask */
osThreadId_t MotorTaskHandle;
const osThreadAttr_t MotorTask_attributes = {
  .name = "MotorTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MonitoringTask */
osThreadId_t MonitoringTaskHandle;
const osThreadAttr_t MonitoringTask_attributes = {
  .name = "MonitoringTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal7,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartInit(void *argument);
void StartModbus(void *argument);
void StartMotorControl(void *argument);
void StartMonitoring(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of initTask */
  initTaskHandle = osThreadNew(StartInit, NULL, &initTask_attributes);

  /* creation of ModbusTask */
  ModbusTaskHandle = osThreadNew(StartModbus, NULL, &ModbusTask_attributes);

  /* creation of MotorTask */
  MotorTaskHandle = osThreadNew(StartMotorControl, NULL, &MotorTask_attributes);

  /* creation of MonitoringTask */
  MonitoringTaskHandle = osThreadNew(StartMonitoring, NULL, &MonitoringTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartInit */
/**
  * @brief  Function implementing the initTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartInit */
void StartInit(void *argument)
{
  /* USER CODE BEGIN StartInit */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartInit */
}

/* USER CODE BEGIN Header_StartModbus */
/**
* @brief Function implementing the ModbusDriverTas thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartModbus */
void StartModbus(void *argument)
{
  /* USER CODE BEGIN StartModbus */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartModbus */
}

/* USER CODE BEGIN Header_StartMotorControl */
/**
* @brief Function implementing the MotorControlTas thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMotorControl */
void StartMotorControl(void *argument)
{
  /* USER CODE BEGIN StartMotorControl */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartMotorControl */
}

/* USER CODE BEGIN Header_StartMonitoring */
/**
* @brief Function implementing the MonitoringTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMonitoring */
void StartMonitoring(void *argument)
{
  /* USER CODE BEGIN StartMonitoring */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartMonitoring */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

