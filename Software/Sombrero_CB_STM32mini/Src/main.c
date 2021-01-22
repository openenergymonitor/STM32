
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
  * COPYRIGHT(c) 2021 STMicroelectronics
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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <string.h>
//#include "ds18b20.h"
#include <math.h>
#include <stdbool.h>
#include "power.h"
#include "reset.h"
#include "phase.h"
#include "RFM69.h"
#include "RFM69_ext.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

// version information
char hwVersion[] = "0.2";
char fwVersion[] = "0.1";


//--------------------------------
// Radio Settings
//--------------------------------
// See RFM69.c for networkID, band, 
// and other settings.
bool radioSender = false; // set 'true' to send test data with RFM69.
bool radioReceiver = false; // set 'true' to enable rfm69cw receiving. 
static char encryptkey[20] = {'\0'}; // twenty character encrypt key, or  '\0'  for nothing.
//char encryptkey[20] = "asdfasdfasdfasdf"; // twenty character encrypt key, or  '\0'  for nothing.

//------------------------------------------------
// timing variables
//------------------------------------------------
uint32_t readings_interval = 500; // millis to read waveform at.
uint32_t posting_interval = 5000; // millis to post data at.

//------------------------------------------------
// misc. flags
//------------------------------------------------
bool first_readings = false;
bool no_volts_flag = false;
// bool readings_ready = false;
bool rpi_connected;
// extern bool conv_halfcplt_flag;
// extern bool conv_cplt_flag;
bool ledBlink = true; // enable or disable LED blinking.



//------------------------
// Channel Results
//------------------------
typedef struct channel_results_
{
  double Vrms;
  double Irms;
  double ApparentPower;
  double RealPower;
  double PowerFactor;
  bool Clipped;
  int Mains_AC_Cycles; // counting complete waveforms
  long SampleCount;
  int Count;
} channel_results_t;

channel_results_t channel_results[CTn] = {0}; //  init the channel results.



//------------------------
// UART SERIAL BUFFERS
//------------------------
#define COMMAND_BUFFER_SIZE 30
char rx_string[COMMAND_BUFFER_SIZE];
char string_buffer[200];


//--------------------
// Time variables
//--------------------
uint32_t current_millis;
uint32_t previous_millis_fine;
uint32_t previous_millis_course;
uint32_t time2;
uint32_t time1;
uint32_t time_diff;


//----------------
// MISC
//----------------
static uint32_t pulseCount = 0;
extern int boot_number;

#define MID_ADC_READING 2048

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

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
  V_SCALE = VCAL * VOLTS_PER_DIV;
  I_SCALE = ICAL * VOLTS_PER_DIV;
  
  int _readings_interval = readings_interval; // temporarily store the usable posting period.
  readings_interval = 100; // speed up first discarded reading.

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
  MX_USART1_UART_Init();
  MX_TIM8_Init();
  MX_ADC2_Init();
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  MX_ADC4_Init();
  MX_SPI3_Init();
  MX_OPAMP2_Init();
  /* USER CODE BEGIN 2 */


  HAL_Delay(2000); // time necessary to catch first serial output on firmware launch after flash.
  
  sprintf(log_buffer, "{HardwareVersion:%s,FirmwareVersion:%s}\r\n", hwVersion, fwVersion); // initital write to buffer.
  debug_printf(log_buffer);
  debug_printf("\r\n\r\nStart, connect VT.\r\n");

  //------------------------
  // RESET CAUSE
  //------------------------
  reset_cause_store = reset_cause_get();
  // while (!usart_tx_ready); // force wait while usart Tx finishes.
  sprintf(log_buffer, "Reset Cause:%s\r\n", reset_cause_get_name(reset_cause_store));
  debug_printf(log_buffer);
  

  //------------------------
  // is the rPi Connected?
  //------------------------
  if (HAL_GPIO_ReadPin(RPI_CONNECTED_GPIO_Port, RPI_CONNECTED_Pin) == 1)
  {
    debug_printf("rPi connected!\r\n");
    // _mode = 1; // rpi mode.
    rpi_connected = true;
  }
  else {
    debug_printf("rPi not connected.\r\n");
    // _mode = 0;
  }


  //--------------------------------
  // Radio Init
  //--------------------------------
  RFM69_RST();
  HAL_Delay(20);
  if (RFM69_initialize(freqBand, nodeID, networkID))
  {
    // while (!usart_tx_ready); // force wait while usart Tx finishes.
    sprintf(log_buffer, "RFM69 Initialized. Freq %dMHz. Node %d. Group %d.\r\n", freqBand, nodeID, networkID);
    debug_printf(log_buffer);
    //RFM69_readAllRegs(); // debug output
  }
  else {
    debug_printf("RFM69 not connected.\r\n");
    radioSender = false; radioSender = false;
  }
  
  // if we have a encryption key, radio encryption will be set.
  if (encryptkey[0]) RFM69_encrypt(encryptkey);

  if (!radioReceiver && !radioSender) {
    HAL_SPI_MspDeInit(&hspi3); // disable SPI if radio is not wanted.
    debug_printf("Radio SPI Deinitialised.\r\n");
  }


  //------------------------
  // ADC & OPAMP Start
  //------------------------
  HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc4, ADC_SINGLE_ENDED);
  HAL_Delay(2);
  HAL_OPAMP_SelfCalibrate(&hopamp2); 
  HAL_OPAMP_Start(&hopamp2);
  HAL_Delay(2);
  start_ADCs(usec_lag);


  //------------------------
  // Temp Sensor Start
  //------------------------
  //init_ds18b20s(); // temperature sensor
  
  // sensible start times..
  current_millis = HAL_GetTick();
  previous_millis_course = current_millis;
  previous_millis_fine = current_millis;


  // Done Init indcator.
  debug_printf("\r\n.\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    current_millis = HAL_GetTick();

    //------------------------------------------------
    // Short interval for accumulating raw readings.
    //------------------------------------------------
    if (current_millis - previous_millis_fine >= readings_interval) 
    {
      uint16_t correction = current_millis - previous_millis_fine - readings_interval;
      previous_millis_fine = current_millis - correction;

      if (readings_requested) no_volts_flag = true;

      readings_requested = true;

      // RTC debug
      // RTC_CalendarShow(aShowTime, aShowDate);
      // debug_printf((char*)aShowDate); debug_printf("\r\n");
      // debug_printf((char*)aShowTime); debug_printf("\r\n");
    }
    

    //---------------------------------------------------
    // ADC DMA buffer flags. Process ADC Buffer Frames.
    //---------------------------------------------------
    if (conv_hfcplt_flag)
    {
      conv_hfcplt_flag = false;
      process_frame(0); // process 1st half of buffer.
    }
    if (conv_cplt_flag)
    {
      conv_cplt_flag = false;
      process_frame(adc_buff_half_size); // process 2nd half of buffer.
    }
    // HAL_Delay(200); // ADC buffer overrun flag test.


    
    //------------------------------------------------------
    // Calculate volts, amps etc. and store in accumulator.
    //------------------------------------------------------
    if (readings_ready)
    {
      readings_ready = false;
      memset(channel_rdy_bools, 0, sizeof(channel_rdy_bools)); // clear flags.

      if (!first_readings) { 
        readings_interval = _readings_interval;
        first_readings = true; 
        
        previous_millis_course = current_millis;
        
        // while (!usart_tx_ready); // force wait while usart Tx finishes.
        sprintf(log_buffer, "Start Sampling Millis: %ld\r\n", current_millis); // initital write to buffer.
        debug_printf(log_buffer);

        goto EndJump;
      } // discard the first set as beginning of 1st waveform not tracked.
      
      // initital write to buffer. sprintf or similar is crucial to prepare for strcat further along.
      sprintf(log_buffer, "HW:%s,FW:%s\r\n", hwVersion, fwVersion); 
      //debug_printf(log_buffer);

      // CALCULATE POWER
      for (int ch = 0; ch < CTn; ch++)
      {
        channel_t *chn = &channels_ready[ch];
        channel_results_t *chn_result = &channel_results[ch];

        last_powerFactor[ch] = powerFactor_now[ch];
        calcPower(ch);
        powerFactor_now[ch] = powerFactor;
        pfHunt(ch);

        if (chn->Iclipped) { chn_result->Clipped = true; chn->Iclipped = false; }

        if (ch == 0) { // estimate mains_frequency on a single channel, no need for more.
          mains_frequency = 1.0/(((chn->samplecount * CTn) / chn->cycles) * adc_conversion_time);
        }

        chn_result->Vrms += Vrms;
        chn_result->Irms += Irms;
        chn_result->ApparentPower += apparentPower;
        chn_result->RealPower += realPower;
        chn_result->PowerFactor += powerFactor;
        chn_result->Mains_AC_Cycles += chn->cycles;
        chn_result->SampleCount += chn->samplecount;
        chn_result->Count++;
      }
    }

    //---------------------------------------------
    // POST DATA.. Serial Output, Radio, etc.
    //---------------------------------------------
    current_millis = HAL_GetTick();
    // now to take the channel results, average them and post them.
    if (current_millis - previous_millis_course >= posting_interval)
      { 
        uint16_t correction = current_millis - previous_millis_course - posting_interval;
        previous_millis_course = current_millis - correction;
        
        if (no_volts_flag) {
          // while (!usart_tx_ready); // force wait while usart Tx finishes.
          sprintf(log_buffer, "No voltage waveform present.\r\n"); // initital write to buffer.
          debug_printf(log_buffer);
          no_volts_flag = false; // reset.
          goto SkipPost;
        }

        for (int ch = 0; ch < CTn; ch++) {
          channel_results_t *chn_result = &channel_results[ch];
          
          // Calculate human-readable values.
          chn_result->Vrms /= chn_result->Count;
          chn_result->Irms /= chn_result->Count;
          chn_result->ApparentPower /= chn_result->Count;
          chn_result->RealPower /= chn_result->Count;
          chn_result->PowerFactor /= chn_result->Count;

          int _ch = ch + 1; // nicer looking channel numbers.
          //if (_ch == 1) { //---uncomment for single channel debug output---
          sprintf(string_buffer, "V%d:%.2lf,I%d:%.3lf,RP%d:%.1lf,AP%d:%.1lf,PF%d:%.6lf,Joules%d:%.3lf,Clip%d:%d,cycles%d:%d,samples%d:%ld,\r\n",
                                _ch, chn_result->Vrms, _ch, chn_result->Irms, _ch, chn_result->RealPower, 
                                _ch, chn_result->ApparentPower, _ch, chn_result->PowerFactor, 
                                _ch, Ws_accumulator[ch], _ch, chn_result->Clipped, 
                                _ch, chn_result->Mains_AC_Cycles, _ch, chn_result->SampleCount);
          strcat(log_buffer, string_buffer);
          //} //---uncomment for single channel debug output---
          chn_result->Clipped = false;
        }

        // Main frequency estimate.
        sprintf(string_buffer, "Hz:%.2f,", mains_frequency);
        strcat(log_buffer, string_buffer);
        
        // Millis
        sprintf(string_buffer, "millis:%ld,", current_millis);
        strcat(log_buffer, string_buffer);
        
        // Pulsecounters
        sprintf(string_buffer, "PC:%ld,", pulseCount);
        strcat(log_buffer, string_buffer);

        // has the adc buffer overrun?
        sprintf(string_buffer, "buffOverrun:%d", adc_buffer_overflow);
        adc_buffer_overflow = false; // reset
        strcat(log_buffer, string_buffer);
        
        //------------------------------------
        // Process Temperature Sesnor Data
        //------------------------------------
        // process_ds18b20s();
        // todo

        // close the string and add some whitespace for clarity.
        // if (!rpi_connected) { strcat(log_buffer, "}"); }
        strcat(log_buffer, "\r\n");
        debug_printf(log_buffer);

        // // RFM69 send.
        // if (radioSender) // sending data, test data only.
        // {
        //   radioData.nodeId = nodeID;
        //   radioData.uptime = HAL_GetTick();
        //   if(ledBlink) { HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, 1); HAL_TIM_Base_Start_IT(&htim16); }// LED blink
        //   RFM69_send(toAddress, (const void *)(&radioData), sizeof(radioData), requestACK);
        //   while (!usart_tx_ready); // force wait while usart Tx finishes.
        //   debug_printf("radio Tx\r\n");
        //   //RFM69_sendWithRetry(toAddress, (const void *)(&radioData), sizeof(radioData), 3,20);
        // }

        SkipPost:
        // reset accumulators.
        for (int ch = 0; ch < CTn; ch++) {
                channel_results_t *chn_result = &channel_results[ch];
                memset((void*)chn_result, 0, sizeof(channel_results_t));
        }
        
        
        // if(ledBlink) {HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); }

      } EndJump:; // end main readings_ready function.
    
    // end main readings_ready functions.
    // end main readings_ready functions.
    // end main readings_ready functions.

    // if (readings_ready)
    // {
    //   readings_ready = false;
    //   time2 = time1;
    //   time1 = HAL_GetTick();
    //   time_diff = time1 - time2;
    //   for (int n = 0; n < NUMBER_OF_CHANNELS; n++)
    //   {
    //     channel_t *chn = &channels_copy[n];

    //     float Vmean = chn->sum_V / (float)chn->count;
    //     float Imean = chn->sum_I / (float)chn->count;

    //     chn->sum_V_sq /= (float)chn->count;
    //     chn->sum_V_sq -= (Vmean * Vmean); // offset subtraction

    //     if (chn->sum_V_sq < 0) // if offset removal cause a negative number,
    //       chn->sum_V_sq = 0;   // make it 0 to avoid a nan at sqrt.

    //     float Vrms = V_RATIO * sqrtf((float)chn->sum_V_sq);

    //     chn->sum_I_sq /= (float)chn->count;
    //     chn->sum_I_sq -= (Imean * Imean);

    //     if (chn->sum_I_sq < 0)
    //       chn->sum_I_sq = 0;

    //     float Irms = I_RATIO * sqrtf((float)chn->sum_I_sq);

    //     float mean_P = (chn->sum_P / (float)chn->count) - (Vmean * Imean);
    //     float realPower = V_RATIO * I_RATIO * mean_P;

    //     float apparentPower = Vrms * Irms;

    //     float powerFactor;
    //     if (apparentPower != 0) // prevents 'inf' at division
    //     {
    //       powerFactor = realPower / apparentPower;
    //     }
    //     else
    //       powerFactor = 0;

    //     Ws_acc[n] += ((float)(time_diff / 1000.0)) * realPower; // Watt second accumulator.
    //     float Wh_acc = Ws_acc[n] / 3600.0;                      // Wh_acc
    //     float frequency = 250.0 / (float)(time_diff / 1000.0);    // Hz

    //     int _n = n + 1; // for nicer serial print out
    //     sprintf(log_buffer, "V%d:%.2f,I%d:%.3f,RP%d:%.1f,AP%d:%.1f,PF%d:%.3f,Wh%d:%.3f,Hz%d:%.2f,C%d:%ld", _n, Vrms, _n, Irms, _n, realPower, _n, apparentPower, _n, powerFactor, _n, Wh_acc, _n, frequency, _n, chn->count);
    //     debug_printf(log_buffer);
    //   }

    //   debug_printf("\r\n");

      /*
      CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
      ITM->LAR = 0xC5ACCE55;
      DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
      DWT->CYCCNT = 0;
      uint32_t DWTcounter = DWT->CYCCNT;
      sprintf(log_buffer, "here:%ld\r\n", DWTcounter);
      debug_printf(log_buffer);
      */
    // }
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART3
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_TIM8;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_SYSCLK;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
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

// void onPulse()
// {
//   pulseCount++;
// }

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin) {
  if (GPIO_Pin == PULSE_INPUT_Pin) {
    // debugFlag_pulse1 = true;    
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
