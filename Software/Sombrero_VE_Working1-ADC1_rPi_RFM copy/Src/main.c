
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
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "jsmn.h"
#include "RFM69.h"
#include "RFM69_ext.h"
//#include "ds18b20.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

//--------
// MODES
//--------
// 0 = Standalone
// 1 = rPi
int mode = 1; // for testing.


//--------
// ADC
//--------
#define MID_ADC_READING 2048
const int crossings = 251;
uint32_t post_interval = 5000; // millis to post data at.
bool post_flag;
bool readings_ready;
extern const uint16_t adc_buff_half_size;
int16_t sample_V_Hz, signed_V_Hz, previous_V_Hz, crossings_Hz;
bool hz_ready;
uint16_t Hzcounter, Hzaccumulator, Hzacc_begin, Hz_value_count;
float Hz_value, Hz_value_accumulator, Hz_value_averaged;
const float Tconv = 614.0*(1.0/72000000.0); // time in seconds for an ADC conversion
bool Vclipped;
bool crossings_flip;
bool first_readings;
int freq_index;
int last_freq_index;


//--------
// ISR accumulators
//--------
typedef struct channel_
{
  int64_t sum_P;
  uint64_t sum_V_sq;
  uint64_t sum_I_sq;
  int32_t sum_V;
  int32_t sum_I;
  uint32_t count;
  bool Iclipped;
  uint8_t positive_V;
  uint8_t last_positive_V;
  uint8_t cycles;
} channel_t;

static channel_t channels[CTn];
static channel_t channels_ready[CTn];
bool channel_ready[CTn] = {0}; // 


//--------
// CALIBRATION
//--------
const float VOLTS_PER_DIV = (3.3 / 4096.0);
//float VCAL = 266.1238757156; // note - single-phase proto board
//float VCAL = 224.4135906687; // note - 3-phase proto board
float VCAL = 236.660160908; // dan's 2nd hand ac-ac adaptor
float ICAL = 90.9;


//--------
// UART SERIAL BUFFERS
//--------
extern char log_buffer[];
bool log_buffer_flag;
char rx_string[COMMAND_BUFFER_SIZE];
char string_buffer[150];


//--------
// RADIO
//--------
bool radio_send;
uint16_t networkID = 210; // a.k.a. Network Group
uint8_t nodeID = 1;
uint16_t freqBand = 433;
uint8_t toAddress = 10;
bool requestACK = false;
char encryptkey[20] = "asdfasdfasdfasdf";
typedef struct
{
  uint32_t nodeId; // store this nodeId
  uint32_t uptime; // uptime in ms
  //float         temp;   // other data
} Payload;
Payload theData;


//--------
// Time variables
//--------
uint32_t current_millis;
uint32_t previous_millis;

//--------
// MISC
//--------
uint32_t pulseCount = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void process_frame (uint16_t offset)
{
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_SET); // blink the led

  int16_t sample_V, sample_I, signed_V, signed_I;

  for (int i = 0; i < adc_buff_half_size; i += CTn) // CTn = CT channel quanity.
  {
    // Cycle through channels, accumulating
    for (int ch = 0; ch < CTn; ch++)
    {
      channel_t *channel = &channels[ch];
      // ----------------------------------------
      // Voltage
      sample_V = adc1_dma_buff[offset + i + ch];
      // if (sample_V == 4095) Vclipped = true; // unlikely
      signed_V = sample_V - MID_ADC_READING;
      channel->sum_V += signed_V;
      channel->sum_V_sq += signed_V * signed_V;
      // ----------------------------------------
      // Current
      sample_I = adc3_dma_buff[offset + i + ch];
      if (sample_I == 4095) channel->Iclipped = true; // much more likely, useful safety information.
      signed_I = sample_I - MID_ADC_READING; // mid-rail removal possible through ADCs, option for future.
      channel->sum_I += signed_I;
      channel->sum_I_sq += signed_I * signed_I;
      // ----------------------------------------
      // Power
      channel->sum_P += signed_V * signed_I;

      channel->count++;

      // upwards-zero-crossing detection, whole waveforms.
      channel->last_positive_V = channel->positive_V; // retrieve the previous value.
      if (signed_V >= 0) channel->positive_V = true; // changed > to >= . offset correction via midrail removal is option.
        else { channel->positive_V = false; }
      // --------------------------------------------------
      // --------------------------------------------------
      if (!channel->last_positive_V && channel->positive_V) { // looking out for a upwards-zero crossing.
      
        // ----------------------------------------
        // Frequency, UNTESTED
        // ----------------------------------------
        if (ch == 0) { // calculate mains frequency on one channel to save cpu time. UNTESTED
          last_freq_index = freq_index;
          freq_index = i;
          uint16_t index_difference = freq_index - last_freq_index;
          Hz_value = 1.0 / ((float)index_difference * Tconv);
        }
        
        if (post_flag) { // and if readings are needed.
          // channel->cycles++; // rather than count cycles for readings_ready, better to have the main loop ask for them.
          // copy and clear accummulator if main() loop demands readings.
          channel_t *channels_ready = &channels_ready[ch];
          // Copy accumulators for use in main loop.
          memcpy ((void*)channels_ready, (void*)channel, sizeof(channel_t));
          // Reset accumulators to zero.
          memset((void*)channel, 0, sizeof(channel_t));
          // set channel_ready for the channel.
          channel_ready[ch] = true;
          // are all the channels ready?
          uint8_t ready_count = 0;
          for (uint8_t i = 0; i < CTn; i++) 
          {
            if (channel_ready[i]) ready_count++;
          }
          // if all the channels are ready
          if (ready_count == CTn) { readings_ready = true;  memset(channel_ready, 0, CTn); }
        }
      }
      // --------------------------------------------------
      // --------------------------------------------------
    }
  }

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

  // is the rPi Connected?
  if (HAL_GPIO_ReadPin(RPI_GPIO16_GPIO_Port, RPI_GPIO16_Pin) == 1 && HAL_GPIO_ReadPin(RPI_GPIO20_GPIO_Port, RPI_GPIO20_Pin) == 1)
  {
    debug_printf("rPi connected!\r\n");
    mode = 1;
  }
  //------------------------
  // RADIO begin
  //------------------------
  RFM69_RST();
  HAL_Delay(10);
  if (RFM69_initialize(freqBand, nodeID, networkID))
  {
    sprintf(log_buffer, "RFM69 Initialized. Freq %dMHz. Node %d. Group %d.\r\n", freqBand, nodeID, networkID);
    debug_printf(log_buffer);
    //RFM69_readAllRegs(); // debugging
  }
  else
  {
    debug_printf("RFM69 not connected.\r\n");
  }
  if (encryptkey[0] != '\0') // if we have a encryption key, radio encryption will be set.
  {
    RFM69_encrypt(encryptkey);
  }

  //------------------------
  // ADC BEGIN
  //------------------------
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED);
  HAL_OPAMP_Start(&hopamp4);
  HAL_Delay(2);
  start_ADCs();

  //init_ds18b20s();

  //------------------------
  // UART DMA RX BEGIN
  //------------------------
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  HAL_UART_Receive_DMA(&huart1, rx_buff, sizeof(rx_buff));
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
  HAL_UART_Receive_DMA(&huart2, rx_buff, sizeof(rx_buff));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    //process_ds18b20s();

    current_millis = HAL_GetTick();
    //------------------------
    // check time for posting primary data.
    //------------------------
    if (current_millis - previous_millis >= post_interval) {
      uint32_t correction = current_millis - previous_millis - post_interval;
      previous_millis = current_millis - correction;
      post_flag = true;
    }
    
    //------------------------
    // USART RX check
    //------------------------
    if (usart1_rx_flag)
    {
      usart1_rx_flag = 0;
      memcpy(rx_string, rx_buff, sizeof(rx_buff));
      memset(rx_buff, 0, sizeof(rx_buff));
      huart1.hdmarx->Instance->CCR &= ~DMA_CCR_EN;
      huart1.hdmarx->Instance->CNDTR = sizeof(rx_buff);
      huart1.hdmarx->Instance->CCR |= DMA_CCR_EN;
      rPi_printf("STM32:"); // respond to command with "STM32:" prefix.
      json_parser(rx_string); // the actual response is posted from json_parser() result;
    }
    if (usart2_rx_flag)
    {
      usart2_rx_flag = 0;
      memcpy(rx_string, rx_buff, sizeof(rx_buff));
      memset(rx_buff, 0, sizeof(rx_buff));
      huart2.hdmarx->Instance->CCR &= ~DMA_CCR_EN;
      huart2.hdmarx->Instance->CNDTR = sizeof(rx_buff);
      huart2.hdmarx->Instance->CCR |= DMA_CCR_EN;
      rPi_printf("STM32:"); // respond to command with "STM32:" prefix.
      json_parser(rx_string); // the actual response is posted from json_parser() result;
      
      //(!strcmp(select, "boot_reason"))
      //sprintf(log_buffer, "%s\r\n", rx_string);
      //rPi_printf(log_buffer);
      //rPi_printf("baker_street:100:D:doodoodooo:80085\r\n");
    }

    
    //------------------------
    // ADC DMA buffer flags, adc conversions complete?
    //------------------------
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


    //------------------------------------------------
    // Check to post primary data.
    //------------------------------------------------
    if (readings_ready)
    {
      readings_ready = false;
      if (!first_readings) { first_readings = true; goto EndJump; } // discard the first set to ensure channel sync.
      
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_SET); // blink the led
      
      sprintf(log_buffer, "VCLIP:%d,", Vclipped); // first sprint, next ones are strcat().

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
        // calculate PF, preventing dividing by zero error. Necessary?
        if (apparentPower != 0)
        {
          powerFactor = realPower / apparentPower;
        }
        else
          powerFactor = 0;

        int _ch = ch + 1;
        if (mode == 1) {
          sprintf(string_buffer, "V%d:%.2f,I%d:%.3f,RP%d:%.1f,PF%d:%.3f,ICLIP%d:%d,", _ch, Vrms, _ch, Irms, _ch, realPower, _ch, powerFactor, _ch, chn->Iclipped);
          strcat(log_buffer, string_buffer);
        }
        Vclipped = false;
      }

      //current_millis = HAL_GetTick(); no point doing it twice in the main loop?
      sprintf(string_buffer, "millis:%ld,", current_millis);
      if (mode == 1) strcat(log_buffer, string_buffer);

      sprintf(string_buffer, "PC:%ld", pulseCount);
      if (mode == 1) strcat(log_buffer, string_buffer);

      // mains AC freuency, approximate due to CPU clock inaccuracy.
      // Hz_value_averaged
      sprintf(string_buffer, ",Hz_estimate:%.2f", Hz_value_averaged);
      if (mode == 1) strcat(log_buffer, string_buffer);

      // has the adc buffer overrun?
      sprintf(string_buffer, ",buffOverrun:%d", overrun_adc_buffer);
      if (mode == 1) strcat(log_buffer, string_buffer);
      overrun_adc_buffer = 0; // reset

      

      // close the string
      if (mode == 1) strcat(log_buffer, "\r\n");
      log_buffer_flag = 1;

      // RFM69 send.
      if (radio_send)
      {
        theData.nodeId = 1;
        theData.uptime = HAL_GetTick();
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, 1);
        HAL_TIM_Base_Start_IT(&htim16); // LED blink
        RFM69_send(toAddress, (const void *)(&theData), sizeof(theData), requestACK);
        //RFM69_sendWithRetry(toAddress, (const void *)(&theData), sizeof(theData), 3,20);
      }

      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_RESET);
      
      

    } EndJump: // end main readings_ready output.

    // RFM69 receive
    if (RFM69_ReadDIO0Pin()) {
      debug_printf("RFM69 DIO0 high.\r\n");
      RFM69_interruptHandler();
    }
    if (RFM69_receiveDone()) {
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, 1); // LED blink
      HAL_TIM_Base_Start_IT(&htim16); // LED blink, interrupt based.
      // debug output below.
      debug_printf("RFM69 payload received.\r\n");
      PrintRawBytes();
      PrintStruct();
      PrintByteByByte();
    }


    // Print anything in the log buffer to serial.
    if (log_buffer[0] != '\0') { // null terminated strings!
      rPi_printf(log_buffer);
      log_buffer[0] = '\0';
      // https://stackoverflow.com/questions/632846/clearing-a-char-array-c
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
    sprintf(log_buffer, "sys_error:%s,line:%d\r\n", file, line);
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
