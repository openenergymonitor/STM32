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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define IO3_EX_Pin GPIO_PIN_2
#define IO3_EX_GPIO_Port GPIOE
#define IO4_EX_Pin GPIO_PIN_3
#define IO4_EX_GPIO_Port GPIOE
#define IO5_EX_Pin GPIO_PIN_4
#define IO5_EX_GPIO_Port GPIOE
#define IO6_EX_Pin GPIO_PIN_5
#define IO6_EX_GPIO_Port GPIOE
#define IO7_EX_Pin GPIO_PIN_6
#define IO7_EX_GPIO_Port GPIOE
#define BUTTON1_Pin GPIO_PIN_14
#define BUTTON1_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_8
#define LED1_GPIO_Port GPIOD
#define LED2_Pin GPIO_PIN_9
#define LED2_GPIO_Port GPIOD
#define LED3_Pin GPIO_PIN_10
#define LED3_GPIO_Port GPIOD
#define rpi_gpio16_Pin GPIO_PIN_12
#define rpi_gpio16_GPIO_Port GPIOD
#define rpi_gpio20_Pin GPIO_PIN_13
#define rpi_gpio20_GPIO_Port GPIOD
#define DS18B20_2_Pin GPIO_PIN_14
#define DS18B20_2_GPIO_Port GPIOD
#define PUSHIN_2_Pin GPIO_PIN_15
#define PUSHIN_2_GPIO_Port GPIOD
#define PUSHIN_3_Pin GPIO_PIN_6
#define PUSHIN_3_GPIO_Port GPIOC
#define PUSHIN_4_Pin GPIO_PIN_7
#define PUSHIN_4_GPIO_Port GPIOC
#define PULSE2_Pin GPIO_PIN_8
#define PULSE2_GPIO_Port GPIOC
#define DS18B20_RJ45_Pin GPIO_PIN_9
#define DS18B20_RJ45_GPIO_Port GPIOC
#define PULSE1_RJ45_Pin GPIO_PIN_8
#define PULSE1_RJ45_GPIO_Port GPIOA
#define IO8_EX_Pin GPIO_PIN_5
#define IO8_EX_GPIO_Port GPIOB
#define IO11_EX_Pin GPIO_PIN_8
#define IO11_EX_GPIO_Port GPIOB
#define PUSHIN_1_Pin GPIO_PIN_9
#define PUSHIN_1_GPIO_Port GPIOB
#define IO1_EX_Pin GPIO_PIN_0
#define IO1_EX_GPIO_Port GPIOE
#define IO2_EX_Pin GPIO_PIN_1
#define IO2_EX_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
