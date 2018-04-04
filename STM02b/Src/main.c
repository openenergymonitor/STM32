
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
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#define true 1
#define false 0

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
float VCAL = 268.97;
float ICAL = 60.606;

char log_buffer[100];

long shiftedFCL = 0;

int sampleA0 = 0;
int last_sampleA0 = 0;
long shifted_filterA0 = -10000;

int sampleA1 = 0;
int last_sampleA1 = 0;
long shifted_filterA1 = -10000;

long sumA0 = 0;
uint64_t sqsumA0 = 0;
long sumA1 = 0;
unsigned long sqsumA1 = 0;

unsigned long sumA0A1 = 0;

int checkVCross = 0;
int lastVCross = 0;
int crossCount = 0;
unsigned long sampleCount = 0;
float V_RATIO = 0;
float I_RATIO = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/


void process_frame(int offset)
{
    for (int i=0; i<2000; i++) {
        last_sampleA0 = sampleA0;
        sampleA0 = adc1_dma_buff[offset+i];
        
        shiftedFCL = shifted_filterA0 + (long)((sampleA0 - last_sampleA0)<<8);
        shifted_filterA0 = shiftedFCL - (shiftedFCL>>8);
        long filtered_A0 = (shifted_filterA0+128)>>8;
        
        sumA0 += filtered_A0;
        sqsumA0 += filtered_A0 * filtered_A0;
    
        
        last_sampleA1 = sampleA1;
        sampleA1 = adc2_dma_buff[offset+i];
        
        shiftedFCL = shifted_filterA1 + (long)((sampleA1 - last_sampleA1)<<8);
        shifted_filterA1 = shiftedFCL - (shiftedFCL>>8);
        long filtered_A1 = (shifted_filterA1+128)>>8;
        
        sumA1 += filtered_A1;
        sqsumA1 += filtered_A1 * filtered_A1;
        
        sumA0A1 += filtered_A0 * filtered_A1;
        
        sampleCount ++;
        
        lastVCross = checkVCross;
        if (filtered_A0 > 0) checkVCross = true; else checkVCross = false;
        if (lastVCross != checkVCross) crossCount++;
        
        
        if (crossCount>50) {
          crossCount = 0;
            
          int rmsA0 = sqrt(sqsumA0 / sampleCount);
          int rmsA1 = sqrt(sqsumA1 / sampleCount);
          int meanA0A1 = (sumA0A1 / sampleCount);
          
          float Vrms = V_RATIO * rmsA0;
          float Irms = I_RATIO * rmsA1;

          float realPower = V_RATIO * I_RATIO * meanA0A1;
          float apparentPower = Vrms * Irms;
          float powerFactor = realPower / apparentPower;
          
          sprintf(log_buffer,"%.2f\t%.2f\t%.0f\t%.0f\t%.3f\t%lu\r\n", Vrms, Irms, realPower, apparentPower, powerFactor, sampleCount);
          debug_printf(log_buffer);
          
          sumA0 = 0;
          sqsumA0 = 0;
          sumA1 = 0;
          sqsumA1 = 0;
          sumA0A1 = 0;
          sampleCount = 0;
        }  
        

    }
}

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

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
  V_RATIO = VCAL * (3.3 / 4096.0);
  I_RATIO = ICAL * (3.3 / 4096.0);


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
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
  
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);      // LED on
  
  snprintf(log_buffer, sizeof(log_buffer),
	   "\nOEM ADC Demo 1.0\r\n");
  debug_printf(log_buffer);
  
  sprintf(log_buffer,"Vrms\tIrms\tRP\tAP\tPF\r\n");
  debug_printf(log_buffer);

  calibrate_ADCS();

  //
  // Start it all running.  We do continuous sampling in the followin channel order:
  // 1,3,4,1,5,6,1,7,8,1,9,11,1,12,14  (15 readings per sequence)
  // The DMA buffer is set to 100x15 and will interrupt us when half full, and full.
  // Once full it wraps back to the beginning.
  //
  start_ADCS();
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    // Conversions take sample_time + 12.5 cycles  (all ADC cycles at 72MHz)
    // where sample_time is how long you want it to spend charging the S&H cap
    // In the GUI we configured sampling time to 1.5 cycles on every channel, so each sample takes 14 ADC cycles
    // There are 15 conversion in the sequence (also chosen in the GUI)
    // CAUTION:  a sampling time of 1.5 cycles with a 72MHz clock is a crazy-low sampling time (20.8 nsecs)
    // You'd need a seriously low source impedance for that to give stable results, but hey, this is just
    // example code to show what's possible.  Sampling time can be set as large as 601.5 cycles (8.3 usecs)
    // Or you can slow the ADC clock down (see Clock Configuration Tab in GUI).
    //
    if (adc_half_conv_complete && !adc_half_conv_overrun) {
      //
      // You've got ~150 usecs to process the bottom 50 x 15 readings
      // to be found in adc2_dma_buff[0..749], before they get overwritten
      // In this demo we don't even look at the data, but toggle the LED so
      // we can measure with the scope how often the data comes around.
      //
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);      // LED off
      adc_half_conv_complete = false;
      
      process_frame(0);  // 0 to 2000
    }

    if (adc_full_conv_complete && !adc_full_conv_overrun) {
      //
      // You've got ~150 usecs to process the top  50 x 15 readings
      // to be found in adc2_dma_buff[750..1449], before they get overwritten
      // In this demo we don't even look at the data, but toggle the LED so
      // we can measure with the scope how often the data comes around.
      //
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);      // LED on
      adc_full_conv_complete = false;
      
      process_frame(2000); // 2000 to 4000
    }
    
    //
    // See if we've overrun and lost our place.
    //
    if (adc_half_conv_overrun || adc_full_conv_overrun) {
      snprintf(log_buffer, sizeof(log_buffer), "Data overrun!!!\n");
      debug_printf(log_buffer);
      adc_full_conv_complete = adc_half_conv_complete = adc_full_conv_overrun = adc_half_conv_overrun = false;
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
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
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
