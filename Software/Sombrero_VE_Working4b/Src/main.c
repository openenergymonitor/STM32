
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
// 2 = ESP32
int mode = 1; // initialised as rPi mode for testing.


//------------------------------------------------
// timing, ADCs, power and frequency variables
//------------------------------------------------
#define MID_ADC_READING 2048
uint32_t posting_interval = 2500; // millis to post data at.
bool readings_ready = false;
bool readings_requested = false;
bool first_readings = false;
double V_RATIO;
double I_RATIO;
extern uint16_t const adc_buff_half_size;
const double adc_conversion_time = (194.0*(1.0/(72000000.0/4))); // time in seconds for an ADC conversion, for estimating mains AC frequency.
//const double adc_conversion_time = (614.0*(1.0/(72000000.0/4))); // time in seconds for an ADC conversion, for estimating mains AC frequency.
//const double phase_resolution = (adc_conversion_time / (1.0/50.0)) * 360.0);
double mains_frequency;
double Ws_accumulator[CTn];
//bool Vclipped; // unlikely?
//bool hunt_PF;


//------------------------
// Channel Accumulators
//------------------------
typedef struct channel_
{
  int64_t sum_P;
  uint64_t sum_V_sq;
  uint64_t sum_I_sq;
  int64_t sum_V;
  int64_t sum_I;
  uint32_t count;
  uint32_t cycles;
  uint32_t phase_shift;
  uint32_t positive_V;
  uint32_t last_positive_V;
  uint32_t Iclipped;
} channel_t; // data struct per CT channel, stm32 only does 32-bit struct items, anything else gets padded out to 32-bit automatically by

static channel_t channels[CTn];
static channel_t channels_ready[CTn];
bool channel_rdy_bools[CTn];


//----------------
// CALIBRATION
//----------------
const double VOLTS_PER_DIV = (3.3 / 4096.0);
//double VCAL = 266.1238757156; // note - single-phase proto board
//double VCAL = 224.4135906687; // note - 3-phase proto board
double VCAL = 236.660160908; // mascot ac-ac adaptor
double ICAL = 90.9;


//----------------
// PHASE CALIBRATION
//----------------
// Finite Impulse Response (FIR) filter-like Power Factor correction.
// based on setting the voltage phase per individual CT channel.
int hunt_PF[CTn]; // which channel are we hunting max power factor on?
// 0 = no power factor hunting.
// 1 = power factor hunt start.
// 2 = power factor hunt to the right. (increase VT lead).
// 3 = power factor hunt to the left. (increase VT lag).
// ?? 4 = hunt complete.
int phase_corrections[CTn];   // store of phase corrections per channel.
double last_powerFactor[CTn]; // PF from previous readings.
double powerFactor_now[CTn];  // PF from most recent readings.
bool pfhuntDone[CTn] = {false};


//------------------------
// UART SERIAL BUFFERS
//------------------------
extern char log_buffer[];
char rx_string[COMMAND_BUFFER_SIZE];
char string_buffer[200];
char readings_rdy_buffer[1000];


//--------
// RADIO
//--------
bool radio_send = 0; // set to 1 to send test data with RFM69.
uint16_t networkID = 210; // a.k.a. Network Group
uint8_t nodeID = 10; // this node's ID (address).
uint16_t freqBand = 433; // MHz
uint8_t toAddress = 1; // destination address.
bool requestACK = false; // untested.
char encryptkey[20] = {'\0'}; // twenty character encrypt key, or  '\0'  for nothing.
//char encryptkey[20] = "asdfasdfasdfasdf"; // twenty character encrypt key, or  '\0'  for nothing.
typedef struct { 
  uint32_t nodeId; 
  uint32_t uptime;
  //double temperature;   // other data
} Payload;
Payload radioData;


//----------------
// Time variables
//----------------
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
double Vrms;
double Irms;
double realPower;
double apparentPower;
double powerFactor;

//------------------------------------------
// Calculate Power Data for one Channel
//------------------------------------------
void calcPower (int ch)
{
  channel_t *chn = &channels_ready[ch];

  double Vmean = (float)chn->sum_V / (float)chn->count;
  double Imean = (float)chn->sum_I / (float)chn->count;

  double f32sum_V_sq_avg = (float)chn->sum_V_sq / (float)chn->count;
  f32sum_V_sq_avg -= (Vmean * Vmean); // offset removal

  double f32sum_I_sq_avg = (float)chn->sum_I_sq / (float)chn->count;
  f32sum_I_sq_avg -= (Imean * Imean); // offset removal

  // assuming result of offset removal always positive.
  // small chance a negative result would cause a nan at sqrt.

  Vrms = V_RATIO * sqrt(f32sum_V_sq_avg);
  Irms = I_RATIO * sqrt(f32sum_I_sq_avg);

  double f32_sum_P_avg = (float)chn->sum_P / (float)chn->count;
  double mean_P = f32_sum_P_avg - (Vmean * Imean); // offset removal
  
  realPower = V_RATIO * I_RATIO * mean_P;
  apparentPower = Vrms * Irms;

  // calculate PF, is it necessary to prevent dividing by zero error?
  if (apparentPower != 0) { powerFactor = realPower / apparentPower; }
  else powerFactor = 0;

  Ws_accumulator[ch] += (realPower * (posting_interval / 1000.0));
}


//------------------------------------------
// Process Buffer into Accumulators.
//------------------------------------------
void process_frame (uint16_t offset)
{
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_SET); // blink the led

  uint16_t sample_V, sample_I;
  int16_t signed_V, signed_I;
   // to hunt one adc_buffer_index per readings_ready, because it's only in readings_ready where PF is calc'd.
  /*
  sprintf(log_buffer, "adc_buff_half_size:%d\r\n", adc_buff_half_size);
  debug_printf(log_buffer);
  log_buffer[0] = '\0';
  */

  for (uint16_t i = 0; i < adc_buff_half_size; i += CTn) // CTn = CT channel quanity.
  {
    
    /* // debug buffer
    sprintf(log_buffer, "signed_v:%d\r\n", signed_V);
    debug_printf(log_buffer);
    log_buffer[0] = '\0';
    */

    //----------------------------------------
    // Power
    //----------------------------------------
    // Cycle through channels, accumulating
    for (int ch = 0; ch < CTn; ch++)
    {
      channel_t *channel = &channels[ch];
      //----------------------------------------
      // Voltage
      //sample_V = adc1_dma_buff[offset + i + ch];
      sample_V = adc1_dma_buff[offset + i + ch + phase_corrections[ch]];  // phase correction could happen here to have higher resolution. 
                                                                          // to shift voltage gives finer grained phase resolution in single-phase mode
                                                                          // because the voltage channel is used by all CTs.
      //if (sample_V == 4095) Vclipped = true; // unlikely
      signed_V = sample_V - MID_ADC_READING;
      channel->sum_V += signed_V;
      channel->sum_V_sq += signed_V * signed_V; // for Vrms
      //----------------------------------------
      // Current
      sample_I = adc3_dma_buff[offset + i + ch];
      if (sample_I == 4095) channel->Iclipped = true; // much more likely, useful safety information.
      signed_I = sample_I - MID_ADC_READING; // mid-rail removal possible through ADC4, option for future perhaps.
      channel->sum_I += signed_I; // 
      channel->sum_I_sq += signed_I * signed_I; // for Irms
      //----------------------------------------
      // Power, instantaneous.
      channel->sum_P += signed_V * signed_I;
      
      channel->count++; // number of adc samples.
      
      //----------------------------------------
      // Upwards-zero-crossing detection, whole AC cycles.
      channel->last_positive_V = channel->positive_V; // retrieve the previous value.
      if (signed_V >= 0) { channel->positive_V = true; } // changed > to >= . not important as MID_ADC_READING 2048 not accurate anyway.
      else { channel->positive_V = false; }
      //--------------------------------------------------
      //--------------------------------------------------
      if (!channel->last_positive_V && channel->positive_V) { // looking out for a upwards-zero crossing.
        channel->cycles++; // rather than count cycles for readings_ready, better to have the main loop ask for them.
        // debug cycle count 
        /*
        sprintf(log_buffer, "cycles%d:%ld\r\n", ch, channel->cycles);
        debug_printf(log_buffer);
        log_buffer[0] = '\0';
        */
        //----------------------------------------
        
        if (hunt_PF[ch] == 0 || hunt_PF[ch] == 4 || pfhuntDone[ch]) { 
          goto pfSkip; // skip if channel done.
        }
        else if (hunt_PF[ch] == 1) {
          phase_corrections[ch]++; hunt_PF[ch] = 2; pfhuntDone[ch] = true;
        }
        else if (hunt_PF[ch] == 2) {
          if (last_powerFactor[ch] > powerFactor_now[ch]) { hunt_PF[ch] = 3; phase_corrections[ch]--; }
          else phase_corrections[ch]++;
          pfhuntDone[ch] = true;
        }
        else if (hunt_PF[ch] == 3) {
          if (last_powerFactor[ch] > powerFactor_now[ch]) { hunt_PF[ch] = 4; phase_corrections[ch]++; } //
          else phase_corrections[ch]--;
          pfhuntDone[ch] = true;
        }
        // print phase corrections.
        sprintf(log_buffer, "pfCorrection%d:%d\r\n", ch, phase_corrections[ch]);
        debug_printf(log_buffer);
        log_buffer[0] = '\0';
        
        pfSkip: // skip to here and continue.
        
        //----------------------------------------
        if (readings_requested && !channel_rdy_bools[ch]) { // if readings are needed by the main loop and channel is not copied/ready.
          channel_t *channel_ready = &channels_ready[ch];
          // copy accumulators for use in main loop.
          memcpy((void*)channel_ready, (void*)channel, sizeof(channel_t));
          // reset accumulators to zero.
          memset((void*)channel, 0, sizeof(channel_t));
          // follow through the state of positive_v so no extra AC cycle is erroneously counted.
          channel->positive_V = true;
          // set 'channel ready' for this channel.
          channel_rdy_bools[ch] = true;
          // are all the channels ready?
          int chn_ready_count = 0;
          for (int j = 0; j < CTn; j++) {
            if (channel_rdy_bools[j]) chn_ready_count++;
          }
          if (chn_ready_count == CTn) { readings_ready = true; readings_requested = false; }
          
          // test waveform sync. printed result should go from 1 to 9.
          /*
          sprintf(log_buffer, "channelsRdy:%d\r\n", chn_ready_count);
          debug_printf(log_buffer);
          log_buffer[0] = '\0';
          */
        }
      }
      //-------------------------------------------------- 
      //-------------------------------------------------- end zero-crossing.
    } // end per channel routine
  } // end buffer routine
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_RESET);
} // end process_frame().




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  V_RATIO = VCAL * VOLTS_PER_DIV;
  I_RATIO = ICAL * VOLTS_PER_DIV;
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
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_ADC4_Init();
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
    log_buffer[0] = '\0';
    //RFM69_readAllRegs(); // debug output
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
  HAL_ADCEx_Calibration_Start(&hadc4, ADC_SINGLE_ENDED);
  HAL_Delay(2);
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

  debug_printf("\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    current_millis = HAL_GetTick();

    //process_ds18b20s();
    
    //---------------------------------------
    // Interval for posting power readings.
    //---------------------------------------
    if (current_millis - previous_millis >= posting_interval) {
      uint32_t correction = current_millis - previous_millis - posting_interval;
      previous_millis = current_millis - correction;
      if (readings_requested)
      {
        debug_printf("No voltage waveform present\r\n");
      }
      readings_requested = true;
    }
    

    //------------------------
    // ADC DMA buffer flags. Process Frames.
    //------------------------
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


    //------------------------------------------------
    // Check to post primary data.
    //------------------------------------------------
    if (readings_ready)
    {
      readings_ready = false; memset(channel_rdy_bools, 0, sizeof(channel_rdy_bools)); // clear flags.      
      if (!first_readings) { first_readings = true; goto EndJump; } // discard the first set as beginning of 1st waveform not tracked.
      
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_SET); // blink the led
      // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, 1); HAL_TIM_Base_Start_IT(&htim16); // LED blink
      
      sprintf(readings_rdy_buffer, "STM:1.0,"); // initital write to buffer.
      pfhuntDone[0] = false;
      for (int ch = 0; ch < CTn; ch++)
      {
        channel_t *chn = &channels_ready[ch];


        last_powerFactor[ch] = powerFactor_now[ch];
        calcPower(ch);
        powerFactor_now[ch] = powerFactor;
        

        if (ch == 0) { // estimate mains_frequency on a single channel, no need for more.
          mains_frequency = 1.0/(((chn->count * CTn) / chn->cycles) * adc_conversion_time);
        }

        int _ch = ch + 1; // nicer looking channel numbers. 1 starts at 1 instead of 0.
        if (mode == 1) {
          sprintf(string_buffer, "V%d:%.2f,I%d:%.3f,RP%d:%.1f,PF%d:%.3f,Joules%d:%.3f,Clip%d:%ld,cycles%d:%ld,samples%d:%ld,", _ch, Vrms, _ch, Irms, _ch, realPower, _ch, powerFactor, _ch, Ws_accumulator[ch], _ch, chn->Iclipped, _ch, chn->cycles, _ch, chn->count);
          strcat(readings_rdy_buffer, string_buffer);
        }
      }

      // Main frequency estimate.
      sprintf(string_buffer, "Hz_estimate:%.1f,", mains_frequency);
      if (mode == 1) strcat(readings_rdy_buffer, string_buffer);
      // Millis
      sprintf(string_buffer, "millis:%ld,", current_millis);
      if (mode == 1) strcat(readings_rdy_buffer, string_buffer);
      // Pulsecount
      sprintf(string_buffer, "PC:%ld,", pulseCount);
      if (mode == 1) strcat(readings_rdy_buffer, string_buffer);
      
      
      /*
      // get an average of the OPAMP4 ADC4 buffer.
      uint32_t ADC4_buff_accumulator = 0;
      for (int i = 0; i < sizeof(adc4_dma_buff); i++) {
        ADC4_buff_accumulator += adc4_dma_buff[i];
        //sprintf(log_buffer, "%d\r\n", adc4_dma_buff[i]);
        //debug_printf(log_buffer);
      }
      //log_buffer[0] = '\0';
      double ADC4_buff_avg = ADC4_buff_accumulator / sizeof(adc4_dma_buff);
      sprintf(string_buffer, "ADC4_avg:%.2f,", ADC4_buff_avg);
      if (mode == 1) strcat(readings_rdy_buffer, string_buffer);
      */

      // has the adc buffer overrun?
      sprintf(string_buffer, "buffOverrun:%d", overrun_adc_buffer);
      overrun_adc_buffer = 0; // reset
      if (mode == 1) strcat(readings_rdy_buffer, string_buffer);

      // close the string
      if (mode == 1) strcat(readings_rdy_buffer, "\r\n");


      // RFM69 send.
      if (radio_send) // sending data, test data only.
      {
        radioData.nodeId = nodeID;
        radioData.uptime = HAL_GetTick();
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, 1); HAL_TIM_Base_Start_IT(&htim16); // LED blink
        RFM69_send(toAddress, (const void *)(&radioData), sizeof(radioData), requestACK);
        //RFM69_sendWithRetry(toAddress, (const void *)(&radioData), sizeof(radioData), 3,20);
      }

      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_RESET);
    } EndJump: // end main readings_ready function.


    //-------------------------------
    // RFM69 Rx
    //-------------------------------
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


    //------------------------
    // UART Rx check
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
      //sprintf(log_buffer, "uartRxDebug:%s\r\n", rx_string);
    }
    if (usart2_rx_flag)
    {
      hunt_PF[0] = true; // test powerfactor hunting.
      usart2_rx_flag = 0;
      memcpy(rx_string, rx_buff, sizeof(rx_buff));
      memset(rx_buff, 0, sizeof(rx_buff));
      huart2.hdmarx->Instance->CCR &= ~DMA_CCR_EN;
      huart2.hdmarx->Instance->CNDTR = sizeof(rx_buff);
      huart2.hdmarx->Instance->CCR |= DMA_CCR_EN;
      rPi_printf("STM32:"); // respond to command with "STM32:" prefix.
      json_parser(rx_string); // the actual response is posted from json_parser() result;
      debug_printf("test\r\n");
      //(!strcmp(select, "boot_reason"))
      //sprintf(log_buffer, "uartRxDebug:%s\r\n", rx_string);
      //rPi_printf(log_buffer);
      //rPi_printf("baker_street:100:D:doodoodooo:80085\r\n");
    }


    //-------------------------------
    // Print char buffers to serial.
    //-------------------------------
    if (log_buffer[0] != '\0') { // null terminated strings!
      rPi_printf(log_buffer);
      log_buffer[0] = '\0'; // set index of string to null to effectively delete the string.
      // https://stackoverflow.com/questions/632846/clearing-a-char-array-c
    }
    if (readings_rdy_buffer[0] != '\0') {
      rPi_printf(readings_rdy_buffer);
      readings_rdy_buffer[0] = '\0';
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
                              |RCC_PERIPHCLK_TIM8;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_SYSCLK;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.Tim16ClockSelection = RCC_TIM16CLK_HCLK;
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
    //debug_printf(log_buffer);
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
