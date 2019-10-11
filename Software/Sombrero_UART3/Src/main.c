/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "opamp.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
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

/* USER CODE BEGIN PV */
volatile char log_buffer[150];
char *buff_tx = "Hello!\n\r";

volatile uint8_t counter = 0;
uint32_t interval = 2000;
volatile uint32_t previousMillis = 0;
volatile uint32_t currentMillis = 0;
uint8_t flag = 0;

volatile uint8_t rx_index = 0;
volatile uint8_t rx_data;
volatile uint8_t buff_rx[50];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{ 
  sprintf(log_buffer, "heya!");
  debug_printf(log_buffer);


  //HAL_UART_Receive_DMA(&huart2, rx_data, 1);
  //sprintf(log_buffer, rx_data);
  //debug_printf(log_buffer);
}
/*
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

  
  if (huart->Instance == USART2)
  //if (huart == &huart2)
  
  {
  
        HAL_UART_Receive_IT(&huart2, rx_data, 1);
      sprintf(log_buffer, rx_data);
      debug_printf(log_buffer);
  }
  
  {
    // if data is not being received clear the buffer
    if (rx_index == 0)
    {
      for (int i = 0; i < sizeof(buff_rx); i++)
      {
        buff_rx[i] = 0;
      }
    }

    // if the character is not ASCII 'enter' then load it into buffer
    else if (rx_data != 13)
    {
      buff_rx[rx_index++] = rx_data;
    }

    else
    {
      HAL_UART_Transmit(&huart2, (uint8_t *)buff_rx, sizeof(buff_rx), 1);
      //sprintf(log_buffer, "%s\n\r", buff_rx);
      //debug_printf(log_buffer);
    }

    HAL_UART_Receive_IT(&huart2, &rx_data, 1);
    HAL_UART_Transmit(&huart2, (uint8_t *)rx_data, 1, 1);
  }
}
*/

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void clear_buff_rx(void)
{
  int i;
  for (i = 0; i < sizeof(buff_rx); i++)
  {
    buff_rx[i] = 0;
  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_ADC4_Init();
  MX_I2C1_Init();
  MX_OPAMP4_Init();
  MX_USART2_UART_Init();
  MX_SPI3_Init();
  /* USER CODE BEGIN 2 */
  // ---
  // PVs

  // ---

  //__HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);   // enable Tx UART interrupt.
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE); // enable Rx UART interrupt.

  //HAL_UART_Receive_DMA(&huart2, rx_data, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (flag)
    {
      HAL_Delay(1);
      HAL_UART_Receive_IT(&huart2, buff_rx, sizeof buff_rx);
      HAL_Delay(1);
      sprintf(log_buffer, "buffrx: %s\r\n", buff_rx);
      debug_printf(log_buffer);
      flag = 0;
      clear_buff_rx();
      __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
    }
    
    //HAL_Delay(10);
    currentMillis = HAL_GetTick();
    if (currentMillis - previousMillis >= interval)
    {
      counter++;
      previousMillis = currentMillis;
      sprintf(log_buffer, "millis: %ld\r\n", currentMillis);
      debug_printf(log_buffer);

      sprintf(log_buffer, "count: %d\r\n", counter);
      debug_printf(log_buffer);

      //sprintf(log_buffer, "buffrx: %s\r\n", buff_rx);
      //debug_printf(log_buffer);

      //clear_buff_rx();
      //USART_ClearFlag(&huart2, USART_FLAG_RXNE);
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_ADC12|RCC_PERIPHCLK_ADC34;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV4;
  PeriphClkInit.Adc34ClockSelection = RCC_ADC34PLLCLK_DIV4;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
