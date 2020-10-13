
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
#define POSITIVE 1
#define NEGATIVE 0

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

uint8_t cycles = 0;

uint8_t polarity_count = 0;
uint8_t polarityUnconfirmed = 0;
uint8_t polarityConfirmed = 0;
uint8_t polarityConfirmedOfLastSampleV = 0;

// uint32_t process_frame_start = 0;
// uint32_t process_frame_elapsed = 0;
// uint32_t process_frame_elapsed_copy = 0;

int32_t Vsamples[2000];
int32_t Isamples[2000];
uint32_t pos = 0;
int8_t samples_ready = false;
int8_t take_readings = false;
uint8_t complete_cycle = 0;

double phase_shift_sum = 0;
int8_t phase_shift_num = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void process_frame(uint16_t offset)
{
  // process_frame_start = HAL_GetTick();

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
    
    // -----------------------------------------------
    // Zero crossing detection from Robert Wall
    // -----------------------------------------------
    polarityConfirmedOfLastSampleV = polarityConfirmed;
    
    if (signed_V > 0) {
        polarityUnconfirmed = POSITIVE;
    } else {
        polarityUnconfirmed = NEGATIVE;
    }
    
    if (polarityUnconfirmed != polarityConfirmedOfLastSampleV) { 
        polarity_count++; 
    } else {
        polarity_count = 0; 
    }

    if (polarity_count >= 3) {
        polarity_count = 0;
        polarityConfirmed = polarityUnconfirmed;
    }
    
    
    if (polarityConfirmed == POSITIVE && polarityConfirmedOfLastSampleV != POSITIVE) {
        cycles++;
        
        if (pos==0 && cycles<122) {
            take_readings = true;
            complete_cycle = cycles + 2;
            
        }
        
        if (cycles==complete_cycle && take_readings==true) {
            take_readings = false;
            samples_ready = true;
        }
        
    }
    
    if (take_readings) {
        Vsamples[pos] = signed_V;
        Isamples[pos] = signed_I;
        pos ++;
    }
    
    // -----------------------------------------------
    
    // 125 cycles or 2.5 seconds
    if (cycles>=125) {
      cycles = 0;
      
      sum_P_copy = sum_P;
      sum_V_sq_copy = sum_V_sq;
      sum_I_sq_copy = sum_I_sq;
      sum_V_copy = sum_V;
      sum_I_copy = sum_I;
      count_copy = count;
      
      // process_frame_elapsed_copy = process_frame_elapsed;
      // process_frame_elapsed = 0;
      
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
  
  // process_frame_elapsed += HAL_GetTick() - process_frame_start;
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
  
  start_ADCs();

  sprintf(log_buffer,"Vrms\tIrms\tRP\tPF\tPFA\tAngle\tCount\r\n");
  debug_printf(log_buffer);  

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {     
     if (samples_ready) {
       samples_ready = false;

       double sinval,cosval;
       double sum_sin_V = 0;
       double sum_cos_V = 0;
       double sum_sin_I = 0;
       double sum_cos_I = 0;
       
       double step = -2 * 2 * 3.14159265 / pos;
       
       // sprintf(log_buffer,"%d\r\n", pos);
       // debug_printf(log_buffer);
       
       for (int x=0; x<pos; x++) {
       
           sinval = sin(x*step);
           cosval = cos(x*step);
           
           sum_sin_V += Vsamples[x] * sinval;
           sum_cos_V += Vsamples[x] * cosval;
           sum_sin_I += Isamples[x] * sinval;
           sum_cos_I += Isamples[x] * cosval;
           
           // sprintf(log_buffer,"%d,%d\r\n", Vsamples[x], Isamples[x]);
           // debug_printf(log_buffer);
       }

       // sprintf(log_buffer,"pos: %d, step: %.5f\r\n",pos,step);
       // debug_printf(log_buffer);
       
       double angleV = 360*atan2(sum_cos_V,sum_sin_V)/(2*3.14159265);
       double angleI = 360*atan2(sum_cos_I,sum_sin_I)/(2*3.14159265);

       // sprintf(log_buffer,"%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\r\n",sum_sin_V,sum_cos_V,angleV,sum_sin_I,sum_cos_I,angleI);
       // debug_printf(log_buffer);
       
       double phase_shift = (angleV-angleI+180) - (360*floor((angleV-angleI+180) / 360.0)) - 180;
       
       // sprintf(log_buffer,"%.3f\r\n",phase_shift);
       // debug_printf(log_buffer);
       
       phase_shift_sum += phase_shift;
       phase_shift_num ++;
       
       pos = 0;
     }
     
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
       double powerFactorDegrees = 360*(acos(powerFactor)/(2*3.14159265));
     
       double phase_shift_average = phase_shift_sum / phase_shift_num;
       phase_shift_sum = 0;
       phase_shift_num = 0;

       //sprintf(log_buffer,"Vrms\tIrms\tRP\tPF\tPFA\tAngle\tCount\r\n");
       //debug_printf(log_buffer);     
       sprintf(log_buffer,"%.2f\t%.3f\t%.1f\t%.3f\t%.3f\t%.3f\t%Ld\r\n", Vrms, Irms, realPower, powerFactor,powerFactorDegrees,phase_shift_average,count_copy);
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
