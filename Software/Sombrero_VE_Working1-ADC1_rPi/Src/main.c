
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
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

//#define true 1
//#define false 0
#define MID_ADC_READING 2048
int cycles = 125;
// Serial output buffer
extern char log_buffer[150];

// Flag
bool readings_ready = false;
bool CLEAR_TO_GO = true;

extern const uint16_t adc_buff_half_size;

// Calibration
const float VOLTS_PER_DIV = (3.3 / 4096.0);
//float VCAL = 266.1238757156; // note - single-phase board
//float VCAL = 224.4135906687; // note - 3-phase board
float VCAL = 236.660160908; // dan's custom ac-ac adaptor
float ICAL = 90.9;

// ISR accumulators
typedef struct channel_
{
  int64_t sum_P;
  uint64_t sum_V_sq;
  uint64_t sum_I_sq;
  int32_t sum_V;
  int32_t sum_I;
  uint32_t count;

} channel_t;

uint32_t pulseCount = 0;

static channel_t channels[CTn];
static channel_t channels_ready[CTn];

int16_t sample_V, sample_I, signed_V, signed_I;
int16_t sample_V_Hz, signed_V_Hz, previous_V_Hz, cycles_Hz;
bool hz_ready = 0;
int32_t Hz_t1 = 0;

#define Hz_ARRAY_SIZE 20
uint32_t Hz_array[Hz_ARRAY_SIZE] = {0};
int Hz_array_counter = 0;
uint32_t Hz_sum = 0;
bool Hz_value_ready = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void process_frame(uint16_t offset)
{
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_SET);
  CLEAR_TO_GO = false;

  for (int i = 0; i < adc_buff_half_size; i += CTn)
  {

    // zero-crossing detection //
    sample_V_Hz = adc1_dma_buff[offset + i];
    signed_V_Hz = sample_V_Hz - MID_ADC_READING;

    if (signed_V_Hz > 0 && previous_V_Hz < 0)
    {
      previous_V_Hz = signed_V_Hz;
      cycles_Hz++;
    }
    else
    {
      previous_V_Hz = signed_V_Hz;
    }

    // for example, 125 upwards-only-zero-crossings approx 2.5 seconds @ 50Hz.
    if (cycles_Hz == cycles)
    {
      cycles_Hz = 0;
      hz_ready = true;
    }

    if (hz_ready)
    {

      hz_ready = false;

      // copy and reset accu.
      for (int ch = 0; ch < CTn; ch++)
      {
        channel_t *channel = &channels[ch];
        channel_t *chn = &channels_ready[ch];
        // Copy accumulators for use in main loop
        memcpy((void *)chn, (void *)channel, sizeof(channel_t));
        // Reset accumulators to zero ready for next set of measurements
        memset((void *)channel, 0, sizeof(channel_t));
      }

      readings_ready = true;
    }

    // Cycle through channels, accumulating
    for (int ch = 0; ch < CTn; ch++)
    {

      channel_t *channel = &channels[ch];

      // ----------------------------------------
      // Voltage
      sample_V = adc1_dma_buff[offset + i + ch];
      signed_V = sample_V - MID_ADC_READING;
      channel->sum_V += signed_V;
      channel->sum_V_sq += signed_V * signed_V;
      // ----------------------------------------
      // Current
      sample_I = adc3_dma_buff[offset + i + ch];
      signed_I = sample_I - MID_ADC_READING;
      channel->sum_I += signed_I;
      channel->sum_I_sq += signed_I * signed_I;
      // ----------------------------------------
      // Power
      channel->sum_P += signed_V * signed_I;

      channel->count++;
    }
  }
  CLEAR_TO_GO = true;
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_RESET);
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
  float V_RATIO = VCAL * VOLTS_PER_DIV;
  float I_RATIO = ICAL * VOLTS_PER_DIV;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_USART2_UART_Init();
  MX_OPAMP4_Init();
  MX_ADC3_Init();
  MX_TIM8_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  debug_printf("start, conn. VT\r\n");
  sprintf(log_buffer, "%d\r\n", adc_buff_half_size);

  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED);

  HAL_OPAMP_Start(&hopamp4);

  HAL_Delay(2);

  start_ADCs();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    if (conv_cplt_flag)
    {
      conv_cplt_flag = false;
      process_frame(0);
    }
    if (conv_hfcplt_flag)
    {
      conv_hfcplt_flag = false;
      process_frame(adc_buff_half_size);
    }

    if (readings_ready && CLEAR_TO_GO)
    {
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_SET);
      readings_ready = false;
      //process_ds18b20s();

      for (int ch = 0; ch < CTn; ch++)
      {
        channel_t *chn = &channels_ready[ch];

        float Vmean = (float)chn->sum_V / (float)chn->count;
        float Imean = (float)chn->sum_I / (float)chn->count;

        float f32sum_V_sq_avg = (float)chn->sum_V_sq / (float)chn->count;
        f32sum_V_sq_avg -= (Vmean * Vmean); // offset removal

        float f32sum_I_sq_avg = (float)chn->sum_I_sq / (float)chn->count;
        f32sum_I_sq_avg -= (Imean * Imean); // offset removal

        if (f32sum_V_sq_avg < 0) // if offset removal cause a negative number,
          f32sum_V_sq_avg = 0;   // make it 0 to avoid a nan at sqrt.
        if (f32sum_I_sq_avg < 0)
          f32sum_I_sq_avg = 0;

        float Vrms = V_RATIO * sqrtf(f32sum_V_sq_avg);
        float Irms = I_RATIO * sqrtf(f32sum_I_sq_avg);

        float f32_sum_P_avg = (float)chn->sum_P / (float)chn->count;
        float mean_P = f32_sum_P_avg - (Vmean * Imean); // offset removal
        float realPower = V_RATIO * I_RATIO * mean_P;

        float apparentPower = Vrms * Irms;

        float powerFactor;
        // calculate PF, preventing dividing by zero error.
        if (apparentPower != 0)
        {
          powerFactor = realPower / apparentPower;
        }
        else
          powerFactor = 0;

        int _ch = ch + 1;
        sprintf(log_buffer, "V%d:%.2f,I%d:%.3f,RP%d:%.1f,AP%d:%.1f,PF%d:%.3f,C%d:%ld,", _ch, Vrms, _ch, Irms, _ch, realPower, _ch, apparentPower, _ch, powerFactor, _ch, chn->count);
        rPi_printf(log_buffer);
        debug_printf(log_buffer);

        uint32_t current_millis = HAL_GetTick();
        sprintf(log_buffer, "millis:%ld\r\n", current_millis);
        debug_printf(log_buffer);
      }
      uint32_t current_millis = HAL_GetTick();
      sprintf(log_buffer, "millis:%ld,", current_millis);
      rPi_printf(log_buffer);
      sprintf(log_buffer, "PC:%ld,", pulseCount);
      rPi_printf(log_buffer);

      uint32_t Hz_t2 = HAL_GetTick();
      uint32_t Hz_td = (Hz_t2 - Hz_t1);
      //sprintf(log_buffer, "Hz_td:%ld\r\n", Hz_td);
      //debug_printf(log_buffer);

      Hz_array[Hz_array_counter] = Hz_td;
      Hz_array_counter++;
      //if (Hz_array_counter
      //sprintf(log_buffer, "Hz_array_counter:%d\r\n", Hz_array_counter);
      //debug_printf(log_buffer);
      Hz_t1 = Hz_t2;
      if (Hz_array_counter >= Hz_ARRAY_SIZE)
      {
        Hz_value_ready = true;
        Hz_array_counter = 0;
      }
      for (int i = 0; i < Hz_ARRAY_SIZE; i++)
      {
        //sprintf(log_buffer, "Hz_ARRAY:%ld :%d\r\n", Hz_array[i], i);
        //debug_printf(log_buffer);
        Hz_sum += Hz_array[i];
      }
      //}
      float Hz_value = 1000.0/((float)Hz_sum / (cycles*Hz_ARRAY_SIZE));
      sprintf(log_buffer, "Hz_sum:%.2f,Ready:%d\r\n", Hz_value, Hz_value_ready);
      rPi_printf(log_buffer);
      Hz_sum = 0;

      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_RESET);
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

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_TIM8;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
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
  while (1)
  {
    sprintf(log_buffer, "sys_error:%s,line:%d", file, line);
    debug_printf(log_buffer);
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
