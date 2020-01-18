
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
  * COPYRIGHT(c) 2019 STMicroelectronics
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
#include "i2c.h"
#include "opamp.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <math.h>
#include <string.h>
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

#define true 1
#define false 0
#define MID_ADC_READING 2048

// Serial output buffer
char log_buffer[150];

// Flag
uint8_t readings_ready = false;

// Calibration
const float VOLTS_PER_DIV = (3.3 / 4096.0);
float VCAL = 268.97;
float ICAL = 90.9;

// ISR accumulators
typedef struct channel_ {
  int64_t sum_P;
  uint64_t sum_V_sq;
  uint64_t sum_I_sq;
  int32_t sum_V;
  int32_t sum_I;
  //uint32_t sum_offset;
  uint32_t count;

  uint32_t positive_V;
  uint32_t last_positive_V;
  uint32_t cycles;
} channel_t;

uint32_t pulseCount = 0;

//#define CTn 9 // number of CT channels
static channel_t channels[CTn];
static channel_t channels_copy[CTn];

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
  //int32_t offset_local;

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_SET);
  for (int i = 0; i < adc_buff_half_size; i += CTn)
  {
    // Cycle through channels
    for (int ch = 0; ch < CTn; ch++)
    {
      channel_t *channel = &channels[ch];
      
      //offset_local = adc4_dma_buff[offset + i + ch];
      //channel->sum_offset += offset_local;
      //sprintf(log_buffer, "here:%ld\r\n", offset_local);
      //debug_printf(log_buffer);
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

      // Zero crossing detection
      channel->last_positive_V = channel->positive_V;
      if (signed_V > 0)
        channel->positive_V = true;
      else
        channel->positive_V = false;
      if (!channel->last_positive_V && channel->positive_V)
        channel->cycles++;

      // 125 cycles or 2.5 seconds
      if (channel->cycles == 125)
      {
        channel->cycles = 0;

        channel_t *chn = &channels_copy[ch];
        // Copy accumulators for use in main loop
        memcpy((void *)chn, (void *)channel, sizeof(channel_t));
        // Reset accumulators to zero ready for next set of measurements
        memset((void *)channel, 0, sizeof(channel_t));

        if (ch == (CTn - 1))
        {
          readings_ready = true;
        }
      }
    }
  }
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_RESET);
}



//---------------------------------------
// RESET CAUSE
//---------------------------------------

/// @brief  Possible STM32 system reset causes
typedef enum reset_cause_e
{
    RESET_CAUSE_UNKNOWN = 0,
    RESET_CAUSE_LOW_POWER_RESET,
    RESET_CAUSE_WINDOW_WATCHDOG_RESET,
    RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET,
    RESET_CAUSE_SOFTWARE_RESET,
    RESET_CAUSE_POWER_ON_POWER_DOWN_RESET,
    RESET_CAUSE_EXTERNAL_RESET_PIN_RESET,
    RESET_CAUSE_EXTERNAL_OPTION_BYTE_LOAD_RESET,
} reset_cause_t;

/// @brief      Obtain the STM32 system reset cause
/// @param      None
/// @return     The system reset cause
reset_cause_t reset_cause_get(void)
{
    reset_cause_t reset_cause;
    /*
  *            @arg @ref RCC_FLAG_OBLRST Option Byte Load reset
  *            @arg @ref RCC_FLAG_PINRST  Pin reset.
  *            @arg @ref RCC_FLAG_PORRST  POR/PDR reset.
  *            @arg @ref RCC_FLAG_SFTRST  Software reset.
  *            @arg @ref RCC_FLAG_IWDGRST Independent Watchdog reset.
  *            @arg @ref RCC_FLAG_WWDGRST Window Watchdog reset.
  *            @arg @ref RCC_FLAG_LPWRRST Low Power reset.
  * */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))
    {
        reset_cause = RESET_CAUSE_LOW_POWER_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
    {
        reset_cause = RESET_CAUSE_WINDOW_WATCHDOG_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        reset_cause = RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
    {
        reset_cause = RESET_CAUSE_SOFTWARE_RESET; // This reset is induced by calling the ARM CMSIS `NVIC_SystemReset()` function!
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
    {
        reset_cause = RESET_CAUSE_POWER_ON_POWER_DOWN_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
    {
        reset_cause = RESET_CAUSE_EXTERNAL_RESET_PIN_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST))
    {
        reset_cause = RESET_CAUSE_EXTERNAL_OPTION_BYTE_LOAD_RESET;
    }
    else
    {
        reset_cause = RESET_CAUSE_UNKNOWN;
    }

    // Clear all the reset flags or else they will remain set during future resets until system power is fully removed.
    __HAL_RCC_CLEAR_RESET_FLAGS();

    return reset_cause; 
}

// Note: any of the STM32 Hardware Abstraction Layer (HAL) Reset and Clock Controller (RCC) header
// files, such as "STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_rcc.h",
// "STM32Cube_FW_F2_V1.7.0/Drivers/STM32F2xx_HAL_Driver/Inc/stm32f2xx_hal_rcc.h", etc., indicate that the 
// brownout flag, `RCC_FLAG_BORRST`, will be set in the event of a "POR/PDR or BOR reset". This means that a 
// Power-On Reset (POR), Power-Down Reset (PDR), OR Brownout Reset (BOR) will trip this flag. See the 
// doxygen just above their definition for the `__HAL_RCC_GET_FLAG()` macro to see this:
// "@arg RCC_FLAG_BORRST: POR/PDR or BOR reset." <== indicates the Brownout Reset flag will *also* be set in 
// the event of a POR/PDR. 
// Therefore, you must check the Brownout Reset flag, `RCC_FLAG_BORRST`, *after* first checking the 
// `RCC_FLAG_PORRST` flag in order to ensure first that the reset cause is NOT a POR/PDR reset.


/// @brief      Obtain the system reset cause as an ASCII-printable name string from a reset cause type
/// @param[in]  reset_cause     The previously-obtained system reset cause
/// @return     A null-terminated ASCII name string describing the system reset cause
const char * reset_cause_get_name(reset_cause_t reset_cause)
{
    const char * reset_cause_name = "TBD";

    switch (reset_cause)
    {
        case RESET_CAUSE_UNKNOWN:
            reset_cause_name = "UNKNOWN";
            break;
        case RESET_CAUSE_LOW_POWER_RESET:
            reset_cause_name = "LOW_POWER_RESET";
            break;
        case RESET_CAUSE_WINDOW_WATCHDOG_RESET:
            reset_cause_name = "WINDOW_WATCHDOG_RESET";
            break;
        case RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET:
            reset_cause_name = "INDEPENDENT_WATCHDOG_RESET";
            break;
        case RESET_CAUSE_SOFTWARE_RESET:
            reset_cause_name = "SOFTWARE_RESET";
            break;
        case RESET_CAUSE_POWER_ON_POWER_DOWN_RESET:
            reset_cause_name = "POWER-ON_RESET (POR) / POWER-DOWN_RESET (PDR)";
            break;
        case RESET_CAUSE_EXTERNAL_RESET_PIN_RESET:
            reset_cause_name = "EXTERNAL_RESET_PIN_RESET";
            break;
        case RESET_CAUSE_EXTERNAL_OPTION_BYTE_LOAD_RESET:
            reset_cause_name = "OPTION_BYTE_LOAD_RESET";
            break;
    }

    return reset_cause_name;
}
/*
Example usage:
reset_cause_t reset_cause = reset_cause_get();
printf("The system reset cause is \"%s\"\n", reset_cause_get_name(reset_cause));
*/
//---------------------------------------
// RESET CAUSE END
//---------------------------------------


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
  MX_USART1_UART_Init();
  MX_USB_PCD_Init();
  MX_ADC3_Init();
  MX_ADC1_Init();
  MX_TIM8_Init();
  MX_ADC4_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  debug_printf("start, conn. VT\r\n");

  //HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  //HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED);

  HAL_OPAMP_Start(&hopamp4);

  start_ADCs();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (readings_ready)
    {
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_SET);
      readings_ready = false;
      //process_ds18b20s();

      for (int ch = 0; ch < CTn; ch++)
      {
        channel_t *chn = &channels_copy[ch];

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

        //float* sqrtV_temp;
        //float* sqrtI_temp;
        //arm_sqrt_f32(f32sum_V_sq_avg, sqrtV_temp);
        //arm_sqrt_f32(f32sum_I_sq_avg, sqrtI_temp);
        //float Vrms = V_RATIO * *sqrtV_temp;
        //float Irms = I_RATIO * *sqrtI_temp;

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

        //float offset_avg = (float)chn->sum_offset / (float)chn->count;

        int _ch = ch + 1;
        //sprintf(log_buffer, "V%d:%.2f,I%d:%.3f,RP%d:%.1f,AP%d:%.1f,PF%d:%.3f,Off%d:%.2f,C%d:%ld,", _ch, Vrms, _ch, Irms, _ch, realPower, _ch, apparentPower, _ch, powerFactor, _ch, offset_avg, _ch, chn->count);
        sprintf(log_buffer, "V%d:%.2f,I%d:%.3f,RP%d:%.1f,AP%d:%.1f,PF%d:%.3f,C%d:%ld,", _ch, Vrms, _ch, Irms, _ch, realPower, _ch, apparentPower, _ch, powerFactor, _ch, chn->count);
        debug_printf(log_buffer);

        uint32_t current_millis = HAL_GetTick();
        sprintf(log_buffer, "millis:%ld\r\n", current_millis);
        debug_printf(log_buffer);
      }
      sprintf(log_buffer, "PC:%ld\r\n", pulseCount);
        debug_printf(log_buffer);
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_RESET);
    }
    /*
      HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_8);
      HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_9);
      HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_10);
      sprintf(log_buffer, "millis:%d\r\n", HAL_GetTick());
      debug_printf(log_buffer);
      debug_printf("test\r\n");
      HAL_Delay(200);
      */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
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

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_TIM8;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
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
