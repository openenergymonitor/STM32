/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2020 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define SPI4_CS_RFM_Pin GPIO_PIN_3
#define SPI4_CS_RFM_GPIO_Port GPIOE
#define SINGLEPHASE_Pin GPIO_PIN_0
#define SINGLEPHASE_GPIO_Port GPIOA
#define THREEPHASE1_Pin GPIO_PIN_5
#define THREEPHASE1_GPIO_Port GPIOA
#define THREEPHASE2_Pin GPIO_PIN_6
#define THREEPHASE2_GPIO_Port GPIOA
#define THREEPHASE3_Pin GPIO_PIN_7
#define THREEPHASE3_GPIO_Port GPIOA
#define BUTTON1_Pin GPIO_PIN_14
#define BUTTON1_GPIO_Port GPIOB
#define BUTTON1_EXTI_IRQn EXTI15_10_IRQn
#define ADC4_5_RJ45_1_Pin GPIO_PIN_15
#define ADC4_5_RJ45_1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_12
#define LED2_GPIO_Port GPIOD
#define LED3_Pin GPIO_PIN_13
#define LED3_GPIO_Port GPIOD
#define ADC4_11_RJ45_2_Pin GPIO_PIN_14
#define ADC4_11_RJ45_2_GPIO_Port GPIOD
#define rPi_PWR_Pin GPIO_PIN_15
#define rPi_PWR_GPIO_Port GPIOD
#define IO_EXT1_Pin GPIO_PIN_6
#define IO_EXT1_GPIO_Port GPIOC
#define IO_EXT2_Pin GPIO_PIN_7
#define IO_EXT2_GPIO_Port GPIOC
#define LED1_Pin GPIO_PIN_8
#define LED1_GPIO_Port GPIOC
#define PULSE1_Pin GPIO_PIN_13
#define PULSE1_GPIO_Port GPIOA
#define PULSE1_EXTI_IRQn EXTI15_10_IRQn
#define PULSE2_Pin GPIO_PIN_6
#define PULSE2_GPIO_Port GPIOF
#define PULSE2_EXTI_IRQn EXTI9_5_IRQn
#define ADC_TRIG_Pin GPIO_PIN_14
#define ADC_TRIG_GPIO_Port GPIOA
#define SPI1_NSS_SLAVE_Pin GPIO_PIN_15
#define SPI1_NSS_SLAVE_GPIO_Port GPIOA
#define DS18B20_Pin GPIO_PIN_10
#define DS18B20_GPIO_Port GPIOC
#define ESP_PWR_Pin GPIO_PIN_11
#define ESP_PWR_GPIO_Port GPIOC
#define uart5_tx_esp_Pin GPIO_PIN_12
#define uart5_tx_esp_GPIO_Port GPIOC
#define uart5_rx_esp_Pin GPIO_PIN_2
#define uart5_rx_esp_GPIO_Port GPIOD
#define USB_RST_Pin GPIO_PIN_3
#define USB_RST_GPIO_Port GPIOD
#define RPI_GPIO16_Pin GPIO_PIN_4
#define RPI_GPIO16_GPIO_Port GPIOD
#define RPI_GPIO20_Pin GPIO_PIN_5
#define RPI_GPIO20_GPIO_Port GPIOD
#define IO_EXT3_Pin GPIO_PIN_6
#define IO_EXT3_GPIO_Port GPIOD
#define SPI4_CS_SDCARD_Pin GPIO_PIN_7
#define SPI4_CS_SDCARD_GPIO_Port GPIOD
#define USART3_RX_MBUS_Pin GPIO_PIN_8
#define USART3_RX_MBUS_GPIO_Port GPIOB
#define USART3_TX_MBUS_Pin GPIO_PIN_9
#define USART3_TX_MBUS_GPIO_Port GPIOB
#define RFM_RST_Pin GPIO_PIN_0
#define RFM_RST_GPIO_Port GPIOE
#define RFM_DIO0_Pin GPIO_PIN_1
#define RFM_DIO0_GPIO_Port GPIOE

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */
extern int _mode;
/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
