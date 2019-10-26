
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f3xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "opamp.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

#define true 1
#define false 0
#define MID_ADC_READING 2048

// Serial output buffer
char log_buffer[100];

// Flag
int8_t readings_ready = false;

// Calibration
double VCAL = 268.97;
double ICAL = 20.0;

// ISR accumulators
int64_t sum_P = 0;
uint64_t sum_V_sq = 0;
uint64_t sum_I_sq = 0;
int64_t sum_V = 0;
int64_t sum_I = 0;
uint64_t count = 0;

// Copy for main loop processing
int64_t sum_P_copy;
uint64_t sum_V_sq_copy;
uint64_t sum_I_sq_copy;
int64_t sum_V_copy;
int64_t sum_I_copy;
uint64_t count_copy;

uint8_t positive_V = 0;
uint8_t last_positive_V = 0;
uint8_t cycles = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void process_frame(uint16_t offset)
{
  int32_t sample_V, sample_I, signed_V, signed_I;
  
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
  for (int i=0; i<2000; i++) {
    // ----------------------------------------
    // Voltage
    sample_V = adc1_dma_buff[offset+i];
    signed_V = sample_V - MID_ADC_READING;
    sum_V += signed_V;
    sum_V_sq += signed_V * signed_V;
    // ----------------------------------------
    // Current
    sample_I = adc2_dma_buff[offset+i];
    signed_I = sample_I - MID_ADC_READING;
    sum_I += signed_I;
    sum_I_sq += signed_I * signed_I;
    // ----------------------------------------
    // Power
    sum_P += signed_V * signed_I;
    
    count ++;
    
    // Zero crossing detection
    last_positive_V = positive_V;
    if (signed_V > 0) positive_V = true; else positive_V = false;
    if (!last_positive_V && positive_V) cycles++;
    
    // 125 cycles or 2.5 seconds
    if (cycles>=125) {
      cycles = 0;
      
      sum_P_copy = sum_P;
      sum_V_sq_copy = sum_V_sq;
      sum_I_sq_copy = sum_I_sq;
      sum_V_copy = sum_V;
      sum_I_copy = sum_I;
      count_copy = count;
      
      sum_P = 0;
      sum_V_sq = 0;
      sum_I_sq = 0;
      sum_V = 0;
      sum_I = 0;
      count = 0;
            
      readings_ready = true;
    }
  }
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  double V_RATIO = VCAL * (3.3 / 4096.0);
  double I_RATIO = ICAL * (3.3 / 4096.0);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_TIM8_Init();
  MX_OPAMP2_Init();
  /* USER CODE BEGIN 2 */

  HAL_OPAMP_Start(&hopamp2);

  sprintf(log_buffer,"Vrms\tIrms\tRP\tAP\tPF\tCount\r\n");
  debug_printf(log_buffer);
  
  start_ADCs();
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
     if (readings_ready) {
       readings_ready = false;
       
       double Vmean = sum_V_copy * (1.0 / count_copy);
       double Imean = sum_I_copy * (1.0 / count_copy);
       
       sum_V_sq_copy *= (1.0 / count_copy);
       sum_V_sq_copy -= (Vmean*Vmean);
       double Vrms = V_RATIO * sqrt((double)sum_V_sq_copy);
       
       sum_I_sq_copy *= (1.0 / count_copy);
       sum_I_sq_copy -= (Imean*Imean);
       double Irms = I_RATIO * sqrt((double)sum_I_sq_copy);
       
       double mean_P = (sum_P_copy * (1.0 / count_copy)) - (Vmean*Imean);
       double realPower = V_RATIO * I_RATIO * mean_P;
       
       double apparentPower = Vrms * Irms;
       double powerFactor = realPower / apparentPower; 
     
       sprintf(log_buffer,"%.2f\t%.3f\t%.1f\t%.1f\t%.3f\t%d\r\n", Vrms, Irms, realPower, apparentPower, powerFactor, count_copy);
       debug_printf(log_buffer);
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

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_TIM8;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Tim8ClockSelection = RCC_TIM8CLK_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
