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
#include <math.h>
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

#define true 1
#define false 0
#define MID_ADC_READING 2048


char log_buffer[100];



// Flag
int8_t readings_ready = false;

// Calibration
double VCAL = 268.97;
double ICAL = 90.9;

// ISR accumulators
typedef struct channel_ {
  int64_t sum_P;
  uint64_t sum_V_sq;
  uint64_t sum_I_sq;
  int64_t sum_V;
  int64_t sum_I;
  uint64_t count;

  uint8_t positive_V;
  uint8_t last_positive_V;
  uint8_t cycles;
} channel_t;

uint64_t pulseCount = 0;

static channel_t channels[3];
static channel_t channels_copy[3];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void process_frame(uint16_t offset)
{
  int32_t sample_V, sample_I, signed_V, signed_I;
  
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_SET);

  for (int i=0; i<3000; i+=3) {
    // Cycle through channels
    for (int n=0; n<3; n++) {
      channel_t* channel = &channels[n];
      
      // ----------------------------------------
      // Voltage
      sample_V = adc1_dma_buff[offset+i+n];
      signed_V = sample_V - MID_ADC_READING;
      channel->sum_V += signed_V;
      channel->sum_V_sq += signed_V * signed_V;
      // ----------------------------------------
      // Current
      sample_I = adc3_dma_buff[offset+i+n];
      signed_I = sample_I - MID_ADC_READING;
      channel->sum_I += signed_I;
      channel->sum_I_sq += signed_I * signed_I;
      // ----------------------------------------
      // Power
      channel->sum_P += signed_V * signed_I;
      
      channel->count ++;
    
    
      // Zero crossing detection
      channel->last_positive_V = channel->positive_V;
      if (signed_V > 0) channel->positive_V = true; else channel->positive_V = false;
      if (!channel->last_positive_V && channel->positive_V) channel->cycles++;
      
      // 125 cycles or 2.5 seconds
      if (channel->cycles>=125) {
        channel->cycles = 0;
        
        channel_t* channel_copy = &channels_copy[n];
        // Copy accumulators for use in main loop 
        memcpy ((void*)channel_copy, (void*)channel, sizeof(channel_t));
        // Reset accumulators to zero ready for next set of measurements
        memset((void*)channel, 0, sizeof(channel_t));
        
        if (n==2) {
          readings_ready = true;
        }
      }
    }
  }
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_RESET);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  float V_RATIO = VCAL * (3.3 / 4096.0);
  float I_RATIO = ICAL * (3.3 / 4096.0);
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

  debug_printf("start\n");
  HAL_Delay(10);
  HAL_OPAMP_Start(&hopamp4);
  start_ADCs();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (readings_ready) {
       readings_ready = false;
       //process_ds18b20s();
       
       for (int n=0; n<3; n++) {
         channel_t* chn = &channels_copy[n];
       
         double Vmean = chn->sum_V * (1.0 / chn->count);
         double Imean = chn->sum_I * (1.0 / chn->count);
         
         chn->sum_V_sq *= (1.0 / chn->count);
         chn->sum_V_sq -= (Vmean*Vmean);
         double Vrms = V_RATIO * sqrt((double)chn->sum_V_sq);
         
         chn->sum_I_sq *= (1.0 / chn->count);
         chn->sum_I_sq -= (Imean*Imean);
         double Irms = I_RATIO * sqrt((double)chn->sum_I_sq);
         
         double mean_P = (chn->sum_P * (1.0 / chn->count)) - (Vmean*Imean);
         double realPower = V_RATIO * I_RATIO * mean_P;
         
         double apparentPower = Vrms * Irms;
         double powerFactor = realPower / apparentPower; 
       
         sprintf(log_buffer,"V%d:%.2f,I%d:%.3f,RP%d:%.1f,AP%d:%.1f,PF%d:%.3f,C%d:%d,", n,Vrms,n,Irms,n,realPower,n,apparentPower,n,powerFactor,n,chn->count);
         debug_printf(log_buffer);
       
       }
       
       //sprintf(log_buffer,"\r\n");
       //debug_printf(log_buffer);
       
       sprintf(log_buffer,"PC:%d\r\n",pulseCount);
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
