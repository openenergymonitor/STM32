
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
#include "i2c.h"
#include "opamp.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "jsmn.h"
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "RFM69.h"
#include "RFM69_ext.h"
//#include "ds18b20.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
// MODES
// 0 = Standalone
// 1 = rPi
int mode = 0;

#define MID_ADC_READING 2048
// const int crossings = 251;
uint32_t post_interval = 5000; // millis to post data at.
bool post_flag = false;

// Serial output buffer
extern char log_buffer[];

// RADIO
bool radio_send = false;
uint16_t networkID = 100; // a.k.a. Network Group
uint8_t nodeID = 1;
uint16_t freqBand = 433;
uint8_t toAddress = 10;
bool requestACK = false;
const char key[20] = "asdfasdfasdfasdf";

typedef struct
{
  uint32_t nodeId; // store this nodeId
  uint32_t uptime; // uptime in ms
  //float         temp;   //temperature maybe?
} Payload;
Payload theData;

// Flags
bool readings_ready = false;
bool CLEAR_TO_GO = true;
bool readings_pls = false;

extern const uint16_t adc_buff_half_size;

// Calibration
const float VOLTS_PER_DIV = (3.3 / 4096.0);
//float VCAL = 266.1238757156; // note - single-phase proto board
//float VCAL = 224.4135906687; // note - 3-phase proto board
const float VCAL = 236.660160908; // dan's custom ac-ac adaptor
const float ICAL = 90.9;

// ISR accumulators
typedef struct channel_
{
  int64_t sum_P;
  uint64_t sum_V_sq;
  uint64_t sum_I_sq;
  int32_t sum_V;
  int32_t sum_I;
  uint32_t count;
  bool Iclipped;
} channel_t;

uint32_t pulseCount = 0;

static channel_t channels[CTn];
static channel_t channels_ready[CTn];

int16_t sample_V, sample_I, signed_V, signed_I;
int16_t sample_V_Hz, signed_V_Hz, previous_V_Hz, crossings_Hz;

bool hz_ready;
uint16_t Hzcounter, Hzaccumulator, Hzacc_begin, Hz_value_count;
float Hz_value, Hz_value_accumulator, Hz_value_averaged;
const float Tconv = 614.0*(1.0/72000000.0);

uint32_t current_millis = 0;

bool Vclipped = false;

char rx_string[COMMAND_BUFFER_SIZE];

bool crossings_flip = false;


uint32_t time_now;
uint32_t previous_time;

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
    // zero-crossing detection sample //
    sample_V_Hz = adc1_dma_buff[offset + i];
    if (sample_V_Hz == 4095)
    {
      Vclipped = true;
    }
    signed_V_Hz = sample_V_Hz - MID_ADC_READING;

    // zero-crossing detection
    if (signed_V_Hz * previous_V_Hz < 0)
    {
      previous_V_Hz = signed_V_Hz;
      crossings_Hz++;
      if (Hzcounter == 0) {
        Hzacc_begin = i;
      }
      Hzcounter++;
      Hzaccumulator += i;
    }
    else
    {
      previous_V_Hz = signed_V_Hz;
    }
/*
    if (crossings_Hz == crossings)
    {
      crossings_Hz = 0;
      //hz_ready = true;
    }
*/
    if (post_flag && ((crossings_Hz % 2) == crossings_flip))
    {
      crossings_flip ^= true;
      post_flag = false;
      //hz_ready = false;

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
      Hz_value_averaged = Hz_value_accumulator / Hz_value_count;
      Hz_value_count = 0;
      Hz_value_accumulator = 0.0;
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
      if (sample_I >= 4090)
      {
        channel->Iclipped = true;
      }
      signed_I = sample_I - MID_ADC_READING;
      channel->sum_I += signed_I;
      channel->sum_I_sq += signed_I * signed_I;
      // ----------------------------------------
      // Power
      channel->sum_P += signed_V * signed_I;

      channel->count++;
    }
    
  }
  
  uint16_t conversion_counts = Hzaccumulator - Hzacc_begin;
  Hzcounter--;
  float temp_Hzvalue = conversion_counts / Hzcounter;
  Hz_value = 1.0000/((float)temp_Hzvalue * Tconv);
  Hz_value_count++;
  Hz_value_accumulator += Hz_value;
  Hzcounter = 0;
  Hzaccumulator = 0;
  Hzacc_begin = 0;

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
  MX_SPI3_Init();
  MX_TIM16_Init();
  MX_TIM17_Init();
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  debug_printf("\r\n\r\nstart, connect VT\r\n");

  //json_parser();

  // READ GPIO STATE FOR MODE, is the rPi Connected?
  // HAL_Delay(500);
  if (HAL_GPIO_ReadPin(RPI_GPIO16_GPIO_Port, RPI_GPIO16_Pin) == 1 && HAL_GPIO_ReadPin(RPI_GPIO20_GPIO_Port, RPI_GPIO20_Pin) == 1)
  {
    debug_printf("rPi connected!\r\n");
    mode = 1;
  }

  // RADIO BEGIN
  RFM69_RST();
  HAL_Delay(10);
  if (RFM69_initialize(freqBand, nodeID, networkID))
  {
    sprintf(log_buffer, "RFM69 Initialized. Freq %dMHz. Node %d. Group %d.\r\n", freqBand, nodeID, networkID);
    debug_printf(log_buffer);
    //RFM69_readAllRegs();
  }
  else
  {
    debug_printf("RFM69 not connected.\r\n");
  }
  RFM69_encrypt(key);

  // ADC BEGIN
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED);
  HAL_OPAMP_Start(&hopamp4);
  HAL_Delay(2);
  start_ADCs();

  //init_ds18b20s();

  // UART DMA RX BEGIN
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  HAL_UART_Receive_DMA(&huart1, rx_buff, sizeof(rx_buff));
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
  HAL_UART_Receive_DMA(&huart2, rx_buff, sizeof(rx_buff));

  char temp_buffer[150];

  //uint16_t pulse_debounce = 200; // milliseconds
  //uint16_t previous_pulse_time;
  //bool pulse_goahead1 = true;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    //process_ds18b20s();

    time_now = HAL_GetTick();
    if (time_now - previous_time >= post_interval) {
      uint32_t correction = time_now - previous_time - post_interval;
      previous_time = time_now - correction;
      post_flag = true;
    }
/*
    // PULSE READING ON RJ45#1
    if ((HAL_GPIO_ReadPin(RJ1_PULSE_GPIO_Port, RJ1_PULSE_Pin) == true) && pulse_goahead1) {
      pulse_goahead1 = false;
      if (time_now - previous_pulse_time >= pulse_debounce) {
        debug_printf("PULSE!\r\n");
        pulseCount++;
      }
      previous_pulse_time = time_now;
    }
    else {
      pulse_goahead1 = true;
    }
*/
    // USART RX CHECKING
    if (usart1_rx_flag)
    {
      usart1_rx_flag = 0;
      memcpy(rx_string, rx_buff, sizeof(rx_buff));
      // stringready_flag = 1;
      memset(rx_buff, 0, sizeof(rx_buff));
      huart1.hdmarx->Instance->CCR &= ~DMA_CCR_EN;
      huart1.hdmarx->Instance->CNDTR = sizeof(rx_buff);
      huart1.hdmarx->Instance->CCR |= DMA_CCR_EN;
      // sprintf(log_buffer, "STM:");
      //strcat(log_buffer, "\r\n");
      //sprintf(log_buffer, "%s:123.1", rx_string);
      //strcat(log_buffer, "\r\n");
      rPi_printf("STM32:");
      json_parser(rx_string);
      //rPi_printf(log_buffer);
      // rPi_printf("baker_street:100:D:doodoodooo:80085\r\n");
    }
    if (usart2_rx_flag)
    {
      usart2_rx_flag = 0;
      memcpy(rx_string, rx_buff, sizeof(rx_buff));
      // stringready_flag = 1;
      memset(rx_buff, 0, sizeof(rx_buff));
      huart2.hdmarx->Instance->CCR &= ~DMA_CCR_EN;
      huart2.hdmarx->Instance->CNDTR = sizeof(rx_buff);
      huart2.hdmarx->Instance->CCR |= DMA_CCR_EN;
      /*if (!strcmp(rx_string, "data_pls")) {
        readings_pls =  true;
      }*/
      rPi_printf("STM32:");
      json_parser(rx_string);
//      strcat
      
      //(!strcmp(select, "boot_reason"))
      //sprintf(log_buffer, "%s\r\n", rx_string);
      //rPi_printf(log_buffer);
      //rPi_printf("baker_street:100:D:doodoodooo:80085\r\n");
    }

    

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
    //if (readings_pls)
    {
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_SET);
      readings_ready = false;
      readings_pls = false;
      
      memset(log_buffer, 0, sizeof(log_buffer));
      
      sprintf(temp_buffer, "VCLIP:%d,", Vclipped);
      strcat(log_buffer, temp_buffer);

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
        //sprintf(log_buffer, "V%d:%.2f,I%d:%.3f,RP%d:%.1f,AP%d:%.1f,PF%d:%.3f,C%d:%ld,ICLIP%d:%d", _ch, Vrms, _ch, Irms, _ch, realPower, _ch, apparentPower, _ch, powerFactor, _ch, chn->count, _ch, chn->Iclipped);
        //debug_printf(log_buffer);
        if (mode == 1) {
          sprintf(temp_buffer, "V%d:%.2f,I%d:%.3f,RP%d:%.1f,PF%d:%.3f,ICLIP%d:%d,", _ch, Vrms, _ch, Irms, _ch, realPower, _ch, powerFactor, _ch, chn->Iclipped);
          //rPi_printf(log_buffer);
          strcat(log_buffer, temp_buffer);
        }
        Vclipped = false; // reset

        //debug_printf(log_buffer);
        //debug_printf("\r\n");
      }

      

      current_millis = HAL_GetTick();
      sprintf(temp_buffer, "millis:%ld,", current_millis);
      if (mode == 1) strcat(log_buffer, temp_buffer);
      //strcat(log_buffer, "\r\n");
      //debug_printf(log_buffer);

      sprintf(temp_buffer, "PC:%ld", pulseCount);
      if (mode == 1) strcat(log_buffer, temp_buffer);

      // MAINS Hz CALCULATION, approximate due to CPU clock inaccuracy.
      //Hz_value_averaged
      sprintf(temp_buffer, ",Hz_estimate:%.2f", Hz_value_averaged);
      if (mode == 1) strcat(log_buffer, temp_buffer);
      /*
      else { strcat(log_buffer, "\r\n");
      debug_printf(log_buffer);
      }
      */

       if (mode == 1) strcat(log_buffer, "\r\n");

      rPi_printf(log_buffer);

      // RADIO test.
      if (radio_send)
      {
        theData.nodeId = 1;
        theData.uptime = HAL_GetTick();
        toAddress = 10;
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, 1);
        HAL_TIM_Base_Start_IT(&htim16); // LED blink
        RFM69_send(toAddress, (const void *)(&theData), sizeof(theData), requestACK);
        //RFM69_sendWithRetry(toAddress, (const void *)(&theData), sizeof(theData), 3,20);
      }
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_RESET);
    }

    
    // SAMPLE RECEIVE CODE
    if (RFM69_ReadDIO0Pin()) {
      debug_printf("DIO-\r\n");
      RFM69_interruptHandler();
    }
    if (RFM69_receiveDone()) {
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, 1); // LED blink
      HAL_TIM_Base_Start_IT(&htim16); // LED blink
      debug_printf("Payload Received!\r\n");
      PrintRawBytes();
      PrintStruct();
      PrintByteByByte();
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

    /**Configure LSE Drive Capability 
    */
  HAL_PWR_EnableBkUpAccess();

  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
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

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_TIM16
                              |RCC_PERIPHCLK_TIM17|RCC_PERIPHCLK_TIM8;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_SYSCLK;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.Tim16ClockSelection = RCC_TIM16CLK_HCLK;
  PeriphClkInit.Tim17ClockSelection = RCC_TIM17CLK_HCLK;
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
void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin) {
  if (GPIO_Pin == RJ1_PULSE_Pin) {
    debug_printf("pulse!\r\n");
    pulseCount++;
  }
}
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
