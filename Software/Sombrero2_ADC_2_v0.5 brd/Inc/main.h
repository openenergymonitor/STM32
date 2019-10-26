/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f3xx_hal.h"

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
void process_frame(uint16_t offset);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define VSIG1_Pin GPIO_PIN_0
#define VSIG1_GPIO_Port GPIOA
#define VSIG2_Pin GPIO_PIN_1
#define VSIG2_GPIO_Port GPIOA
#define VSIG3_Pin GPIO_PIN_2
#define VSIG3_GPIO_Port GPIOA
#define CT9_Pin GPIO_PIN_0
#define CT9_GPIO_Port GPIOB
#define CT8_Pin GPIO_PIN_1
#define CT8_GPIO_Port GPIOB
#define CT7_Pin GPIO_PIN_7
#define CT7_GPIO_Port GPIOE
#define CT6_Pin GPIO_PIN_8
#define CT6_GPIO_Port GPIOE
#define CT5_Pin GPIO_PIN_9
#define CT5_GPIO_Port GPIOE
#define CT4_Pin GPIO_PIN_10
#define CT4_GPIO_Port GPIOE
#define CT3_Pin GPIO_PIN_11
#define CT3_GPIO_Port GPIOE
#define CT2_Pin GPIO_PIN_12
#define CT2_GPIO_Port GPIOE
#define CT1_Pin GPIO_PIN_13
#define CT1_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
