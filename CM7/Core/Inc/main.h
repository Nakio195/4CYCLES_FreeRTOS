/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EN_BRK_AR_Pin GPIO_PIN_10
#define EN_BRK_AR_GPIO_Port GPIOG
#define DIR_BRK_AV_Pin GPIO_PIN_15
#define DIR_BRK_AV_GPIO_Port GPIOH
#define PULSE_DIR_AV_Pin GPIO_PIN_5
#define PULSE_DIR_AV_GPIO_Port GPIOE
#define DIR_DIR_AR_Pin GPIO_PIN_4
#define DIR_DIR_AR_GPIO_Port GPIOE
#define LED_BLUE_Pin GPIO_PIN_3
#define LED_BLUE_GPIO_Port GPIOE
#define ALARM_DIR_AV_Pin GPIO_PIN_7
#define ALARM_DIR_AV_GPIO_Port GPIOK
#define LED_GREEN_Pin GPIO_PIN_13
#define LED_GREEN_GPIO_Port GPIOJ
#define DIR_BRK_AR_Pin GPIO_PIN_10
#define DIR_BRK_AR_GPIO_Port GPIOI
#define EN_DIR_AR_Pin GPIO_PIN_11
#define EN_DIR_AR_GPIO_Port GPIOI
#define EN_DIR_AV_Pin GPIO_PIN_7
#define EN_DIR_AV_GPIO_Port GPIOG
#define PULSE_BRK_AV_Pin GPIO_PIN_13
#define PULSE_BRK_AV_GPIO_Port GPIOI
#define ALARM_BRK_AV_Pin GPIO_PIN_14
#define ALARM_BRK_AV_GPIO_Port GPIOI
#define DIR_DIR_AV_Pin GPIO_PIN_2
#define DIR_DIR_AV_GPIO_Port GPIOK
#define PULSE_DIR_AR_Pin GPIO_PIN_0
#define PULSE_DIR_AR_GPIO_Port GPIOK
#define NSS_AV_Pin GPIO_PIN_1
#define NSS_AV_GPIO_Port GPIOK
#define DIR_PIN_Pin GPIO_PIN_2
#define DIR_PIN_GPIO_Port GPIOA
#define ALARM_BRK_AR_Pin GPIO_PIN_7
#define ALARM_BRK_AR_GPIO_Port GPIOJ
#define PULSE_BRK_AR_Pin GPIO_PIN_15
#define PULSE_BRK_AR_GPIO_Port GPIOI
#define EN_BRK_AV_Pin GPIO_PIN_2
#define EN_BRK_AV_GPIO_Port GPIOB
#define NSS_AR_Pin GPIO_PIN_12
#define NSS_AR_GPIO_Port GPIOH

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
