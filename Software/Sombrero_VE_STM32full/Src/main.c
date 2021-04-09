
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
#include "usb.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "jsmn.h"
#include "reset.h"
#include "json.h"
#include "power.h"
#include "phase.h"
#include "RFM69.h"
#include "RFM69_ext.h"
// #include "ds18b20.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
// test variables
bool testing = false; // for testing
bool dontGiveAMonkeys = false; // don't wait for a VT adaptor to calculate Irms, AP, RP etc.
bool ledBlink = true; // enable or disable LED blinking.

// version information
char hwVersion[] = "1.0";
char fwVersion[] = "0.1";

//-------------
// MODES
//-------------
// 0 = Standalone
// 1 = rPi
// 2 = ESP32
// 3 = testingtestingtestingtestingtestingtestingtesting
int _mode = 0; // init mode
bool rpi_connected = false;




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
bool debugFlag_pulse1 = false;
bool debugFlag_pulse2 = false;
bool debugFlag_Button1 = false;



//----------------------------
// CT Settings : Common
//----------------------------



//----------------------------
// CT Settings : Per Channel
//----------------------------



//----------------------------
// VT Settings : Common
//----------------------------





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

char rx_string[COMMAND_BUFFER_SIZE];
char string_buffer[200];
//char log_buffer[1000];


//----------------
// RADIO
//----------------
bool radioSender = false; // set 'true' to send test data with RFM69.
bool radioReceiver = false; // set 'true' to enable rfm69cw receiving. 
static uint16_t networkID = 210; // a.k.a. Network Group
static uint8_t nodeID = 10; // this node's ID (address).
static uint16_t freqBand = 433; // MHz
static uint8_t toAddress = 1; // destination address.
bool requestACK = false; // untested.
static char encryptkey[20] = {'\0'}; // twenty character encrypt key, or  '\0'  for nothing.
//char encryptkey[20] = "asdfasdfasdfasdf"; // twenty character encrypt key, or  '\0'  for nothing.
typedef struct {
  uint32_t nodeId; 
  uint32_t uptime;
  //double temperature;   // other data
} Payload;
Payload radioData;


//--------------------
// Time variables
//--------------------
uint32_t current_millis;
uint32_t previous_millis_fine;
uint32_t previous_millis_course;


//----------------
// MISC
//----------------
static uint32_t pulseCount1 = 0;
static uint32_t pulseCount2 = 0;
extern char json_response[40];
extern int boot_number;



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/



/* USER CODE END PFP */

/* USER CODE BEGIN 0 */





//------------------------
// EEPROM Emulation
//------------------------
// Credit to Viktor Vano for the Flash read/write code
// https://github.com/viktorvano/STM32F3Discovery_internal_FLASH  

#define FLASH_STORAGE 0x08030000 // Maximum value for STM32F303xE is defined at line 111 of stm32f3xx_hal_flash_ex.h (0x0807FFFFU)
#define page_size 0x800

void save_to_flash(uint8_t *data)
{
	volatile uint32_t data_to_FLASH[(strlen((char*)data)/4)	+ (int)((strlen((char*)data) % 4) != 0)];
	memset((uint8_t*)data_to_FLASH, 0, strlen((char*)data_to_FLASH));
	strcpy((char*)data_to_FLASH, (char*)data);

	volatile uint32_t data_length = (strlen((char*)data_to_FLASH) / 4)
									+ (int)((strlen((char*)data_to_FLASH) % 4) != 0);
	volatile uint16_t pages = (strlen((char*)data)/page_size)
									+ (int)((strlen((char*)data)%page_size) != 0);
	  /* Unlock the Flash to enable the flash control register access *************/
	  HAL_FLASH_Unlock();

	  /* Allow Access to option bytes sector */
	  HAL_FLASH_OB_Unlock();

	  /* Fill EraseInit structure*/
	  FLASH_EraseInitTypeDef EraseInitStruct;
	  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	  EraseInitStruct.PageAddress = FLASH_STORAGE;
	  EraseInitStruct.NbPages = pages;
	  uint32_t PageError;

	  volatile uint32_t write_cnt=0, index=0;

	  volatile HAL_StatusTypeDef status;
	  status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
	  while(index < data_length)
	  {
		  if (status == HAL_OK)
		  {
			  status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_STORAGE+write_cnt, data_to_FLASH[index]);
			  if(status == HAL_OK)
			  {
				  write_cnt += 4;
				  index++;
			  }
		  }
	  }

	  HAL_FLASH_OB_Lock();
	  HAL_FLASH_Lock();
}

void read_flash(uint8_t* data)
{
	volatile uint32_t read_data;
	volatile uint32_t read_cnt=0;
	do
	{
		read_data = *(uint32_t*)(FLASH_STORAGE + read_cnt);
		if(read_data != 0xFFFFFFFF)
		{
			data[read_cnt] = (uint8_t)read_data;
			data[read_cnt + 1] = (uint8_t)(read_data >> 8);
			data[read_cnt + 2] = (uint8_t)(read_data >> 16);
			data[read_cnt + 3] = (uint8_t)(read_data >> 24);
			read_cnt += 4;
		}
	} while(read_data != 0xFFFFFFFF);
}

typedef struct {
  int reset_counter; 
  //uint32_t uptime;
  //double temperature;   // other data
} FlashStruct; 

FlashStruct flash_struct;


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


  //FlashStruct *flashpt = &flash_struct;
  //union Un flashUn; 

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
  MX_TIM8_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_TIM16_Init();
  MX_SPI1_Init();
  MX_SPI4_Init();
  MX_UART4_Init();
  MX_USART3_UART_Init();
  MX_I2C3_Init();
  MX_UART5_Init();
  MX_RTC_Init();
  MX_ADC3_Init();
  MX_OPAMP4_Init();
  MX_ADC4_Init();
  MX_USB_PCD_Init();
  MX_ADC2_Init();
  /* USER CODE BEGIN 2 */

  HAL_Delay(1200); // time necessary to catch first serial output on firmware launch after flash.

  //------------------------
  // UART DMA RX ENABLE
  //------------------------
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  HAL_UART_Receive_DMA(&huart1, rx_buff, sizeof(rx_buff));
  // __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
  // HAL_UART_Receive_DMA(&huart2, rx_buff, sizeof(rx_buff));
  //------------------------------
  // UART Non-blocking TX ENABLE
  //------------------------------
  // __HAL_UART_ENABLE_IT(&huart1, UART_IT_TC); // necessary only to know if we're good to fire another Tx soon after a HAL_UART_Transmit_IT()
  // __HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);  
  // __HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE); // necessary only to know if we're good to fire another Tx soon after a HAL_UART_Transmit_IT()
  // __HAL_UART_ENABLE_IT(&huart2, UART_IT_TXE);  
  // usart1_rx_flag = 1; // debugging

  debug_printf("\r\n\r\nStart, connect VT.\r\n");
  // char startLine[] = "\r\n\r\nStart, connect VT.\r\n";
  // HAL_UART_Transmit_IT(&huart2, (uint8_t*)startLine, strlen(startLine));



  //-------------------------------------------------
  // Analog Input RNG for Radio Start Delay
  //-------------------------------------------------
  if (radioSender) {  
    int RadDelayCount = 0, RadioDelay = 0;
    while(RadDelayCount < 3) {
      static int reading;
      static int previousRadDelay;
      previousRadDelay = reading;
      
      // ADC read start
      HAL_ADC_Start(&hadc4);
      while (HAL_ADC_PollForConversion(&hadc4, 1000) != HAL_OK);
      reading = HAL_ADC_GetValue(&hadc4);
      HAL_ADC_Stop(&hadc4);
      // ADC read end.
      
      RadioDelay = abs((reading - previousRadDelay + 400) * 10); // scale the 'noise'.
      HAL_Delay(100);
      RadDelayCount++;
    }
    while (!usart_tx_ready); // force wait while usart Tx finishes.
    sprintf(log_buffer, "RadioDelay:%d\r\n", RadioDelay);
    debug_printf(log_buffer); 
    HAL_Delay(RadioDelay);

  } // end RadioDelay.


  //------------------------
  // RTC Backup Registers
  //------------------------
  // RTC backup register test, 16 x 32-bit addresses available.
  // see stm32f3xx_hal_rtc_ex.c  line 1118 onwards.
  //uint32_t bkp_write = 123456789;
  uint32_t bkp_ = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);
  while (!usart_tx_ready); // force wait while usart Tx finishes.
  sprintf(log_buffer, "Number of boots:%ld\r\n", bkp_);
  debug_printf(log_buffer);
  boot_number = bkp_;
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, bkp_+1);
  // works!
  
  // RESET CAUSE
  reset_cause_store = reset_cause_get();
  while (!usart_tx_ready); // force wait while usart Tx finishes.
  sprintf(log_buffer, "Reset Cause:%s\r\n", reset_cause_get_name(reset_cause_store));
  debug_printf(log_buffer);
  
  
  //------------------------
  // EEPROM Emulation test
  //------------------------
  /*
  char write_data_pgm[50];
  strcpy(write_data_pgm, "Hellow from Flash Memory!\r\n");
  save_to_flash((uint8_t*) write_data_pgm);
  char read_data_pgm[50];
  read_flash((uint8_t*)read_data_pgm);
  debug_printf(read_data_pgm); // print result
  
  float write_float_pgm = 123.321f;
  float *pointer_write_float = &write_float_pgm;
  save_to_flash((uint8_t*)pointer_write_float);
  float read_float_pgm = 0.0;
  float *pointer_read_float = &read_float_pgm;
  read_flash((uint8_t*)pointer_read_float);
  //char log_buffer[20];
  sprintf(log_buffer, "Float value also read from flash memory! %.3f\r\n", read_float_pgm);
  debug_printf(log_buffer); log_buffer[0] = 0; // print result
  */

  // int test_array_mode[5] = {2,1,3,4,5};
  // sprintf(log_buffer, "test mode-finding: %d\r\n", findmode(test_array_mode, 5));
  // debug_printf(log_buffer); log_buffer[0] = 0; // print result
  // seems findmode() will return the first value of the array if no mode value is found.


  //------------------------
  // is the rPi Connected?
  //------------------------
  if (HAL_GPIO_ReadPin(rPi_PWR_GPIO_Port, rPi_PWR_Pin) == 1)
  {
    debug_printf("rPi connected!\r\n");
    _mode = 1; // rpi mode.
    rpi_connected = true;
  }
  else {
    debug_printf("rPi not connected.\r\n");
    // _mode = 0;
  }


  //------------------------
  // RADIO Init
  //------------------------
  RFM69_RST();
  HAL_Delay(20);
  if (RFM69_initialize(freqBand, nodeID, networkID))
  {
    while (!usart_tx_ready); // force wait while usart Tx finishes.
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
    HAL_SPI_MspDeInit(&hspi4); // disable SPI if radio is not wanted.
    debug_printf("Radio SPI DeInit'd.\r\n");
  }
  //else {
  //RFM69_interruptHandler();
  //RFM69_receiveDone();
  //}


  //------------------------
  // ADC & OPAMP Start
  //------------------------
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED);
  HAL_Delay(2);
  HAL_OPAMP_SelfCalibrate(&hopamp4); 
  HAL_OPAMP_Start(&hopamp4);
  HAL_Delay(2);
  start_ADCs(usec_lag);

  //init_ds18b20s(); // temperature sensor

  debug_printf("\r\n");
  
  // sensible start times..
  current_millis = HAL_GetTick();
  previous_millis_course = current_millis;
  previous_millis_fine = current_millis;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    current_millis = HAL_GetTick();

    // process_ds18b20s();
    
    //------------------------------------------
    // Interval for accumulating power readings.
    //------------------------------------------
    if (current_millis - previous_millis_fine >= readings_interval) 
    {
      uint16_t correction = current_millis - previous_millis_fine - readings_interval;
      previous_millis_fine = current_millis - correction;

      if (readings_requested) no_volts_flag = true;

      readings_requested = true;

      // RTC debug
      //RTC_CalendarShow(aShowTime, aShowDate);
      //debug_printf((char*)aShowDate); debug_printf("\r\n");
      //debug_printf((char*)aShowTime); debug_printf("\r\n");
    }
    

    //------------------------------------------------
    // ADC DMA buffer flags. Process Frames.
    //------------------------------------------------
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
    // To post primary data.
    //------------------------------------------------
    if (readings_ready)
    {
        readings_ready = false;
        memset(channel_rdy_bools, 0, sizeof(channel_rdy_bools)); // clear flags.

        if (!first_readings) { 
          readings_interval = _readings_interval;
          first_readings = true; 
          previous_millis_course = current_millis;
          
          while (!usart_tx_ready); // force wait while usart Tx finishes.
          sprintf(log_buffer, "Start Sampling Millis: %ld\r\n", current_millis); // initital write to buffer.
          debug_printf(log_buffer);
          
          goto EndJump;
        } // discard the first set as beginning of 1st waveform not tracked.
        
        while (!usart_tx_ready); // force wait while usart Tx finishes.
        sprintf(log_buffer, "{STM_HW:%s,STM_FW:%s,\r\n", hwVersion, fwVersion); // initital write to buffer.
        if (rpi_connected) sprintf(log_buffer, "STM_HW:%s,STM_FW:%s,", hwVersion, fwVersion); // initital write to buffer.

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

    // -----------------
    // POSTING SECTION
    // -----------------
    current_millis = HAL_GetTick();
    if (current_millis - previous_millis_course >= posting_interval) { // now to take the channel results, average them and post them.
        uint16_t correction = current_millis - previous_millis_course - posting_interval;
        previous_millis_course = current_millis - correction;

        
        if(ledBlink) { HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET); } // blink the led
        
        if (no_volts_flag) {
          while (!usart_tx_ready); // force wait while usart Tx finishes.
          sprintf(log_buffer, "No voltage waveform present.\r\n"); // initital write to buffer.
          debug_printf(log_buffer);
          no_volts_flag = false; // reset.
          goto SkipPost;
        }

        for (int ch = 0; ch < CTn; ch++) {
          channel_results_t *chn_result = &channel_results[ch];

          chn_result->Vrms /= chn_result->Count;
          chn_result->Irms /= chn_result->Count;
          chn_result->ApparentPower /= chn_result->Count;
          chn_result->RealPower /= chn_result->Count;
          chn_result->PowerFactor /= chn_result->Count;

          int _ch = ch + 1; // nicer looking channel numbers. First channel starts at 1 instead of 0.
          //if (_ch == 1) { // single channel debug output.
          if (rpi_connected) { sprintf(string_buffer, "V%d:%.2lf,I%d:%.3lf,AP%d:%.1lf,RP%d:%.1lf,PF%d:%.6lf,Joules%d:%.3lf,Clip%d:%d,cycles%d:%d,samples%d:%ld,", _ch, chn_result->Vrms, _ch, chn_result->Irms, _ch, chn_result->ApparentPower, _ch, chn_result->RealPower, _ch, chn_result->PowerFactor, _ch, Ws_accumulator[ch], _ch, chn_result->Clipped, _ch, chn_result->Mains_AC_Cycles, _ch, chn_result->SampleCount); }
          else { sprintf(string_buffer, "V%d:%.2lf,I%d:%.3lf,AP%d:%.1lf,RP%d:%.1lf,PF%d:%.6lf,Joules%d:%.3lf,Clip%d:%d,cycles%d:%d,samples%d:%ld,\r\n", _ch, chn_result->Vrms, _ch, chn_result->Irms, _ch, chn_result->ApparentPower, _ch, chn_result->RealPower, _ch, chn_result->PowerFactor, _ch, Ws_accumulator[ch], _ch, chn_result->Clipped, _ch, chn_result->Mains_AC_Cycles, _ch, chn_result->SampleCount); }
          strcat(log_buffer, string_buffer);
          //} // single channel debug output
          chn_result->Clipped = false;
        }
        

        // Main frequency estimate.
        sprintf(string_buffer, "Hz:%.2f,", mains_frequency);
        strcat(log_buffer, string_buffer);
        
        // Millis
        sprintf(string_buffer, "millis:%ld,", current_millis);
        strcat(log_buffer, string_buffer);
        
        // Pulsecounters
        sprintf(string_buffer, "PC1:%ld,", pulseCount1);
        strcat(log_buffer, string_buffer);
        sprintf(string_buffer, "PC2:%ld,", pulseCount2);
        strcat(log_buffer, string_buffer);

        // has the adc buffer overrun?
        sprintf(string_buffer, "buffOverrun:%d", adc_buffer_overflow);
        adc_buffer_overflow = 0; // reset
        strcat(log_buffer, string_buffer);

        // close the string and add some whitespace for clarity.
        if (!rpi_connected) { strcat(log_buffer, "}"); }
        strcat(log_buffer, "\r\n");
        debug_printf(log_buffer);

        // RFM69 send.
        if (radioSender) // sending data, test data only.
        {
          radioData.nodeId = nodeID;
          radioData.uptime = HAL_GetTick();
          if(ledBlink) { HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, 1); HAL_TIM_Base_Start_IT(&htim16); }// LED blink
          RFM69_send(toAddress, (const void *)(&radioData), sizeof(radioData), requestACK);
          while (!usart_tx_ready); // force wait while usart Tx finishes.
          debug_printf("radio Tx\r\n");
          //RFM69_sendWithRetry(toAddress, (const void *)(&radioData), sizeof(radioData), 3,20);
        }

        SkipPost:
        // reset accumulators.
        for (int ch = 0; ch < CTn; ch++) {
                channel_results_t *chn_result = &channel_results[ch];
                memset((void*)chn_result, 0, sizeof(channel_results_t));
        }
        
        
        if(ledBlink) {HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); }

    } EndJump: // end main readings_ready function.
    // end main readings_ready functions.
    // end main readings_ready functions.
    // end main readings_ready functions.



    //-------------------------------
    // RFM69 Rx
    //-------------------------------
    // This needs checking, does a flag need generating from a DIO0 read = true?
    if(radioReceiver) {
      if (RFM69_ReadDIO0Pin()) {
        debug_printf("RFM69 DIO0 high.\r\n");
        //RFM69_interruptHandler();
        if (RFM69_receiveDone()) { // this maybe shouldn't be nested in the previous if(). If Rx doesn't work, could be the problem.
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, 1); // LED blink
          HAL_TIM_Base_Start_IT(&htim16); // LED blink, interrupt based.
          // debug output below.
          debug_printf("RFM69 payload received.\r\n");
          PrintRawBytes();
          //PrintStruct();
          //PrintByteByByte();
          //RFM69_interruptHandler();
          while (!usart_tx_ready); // force wait while usart Tx finishes.
          sprintf(log_buffer, "RSSI:%d\r\n", rssi);
          debug_printf(log_buffer);

          RFM69_receiveBegin();
          //RFM69_receiveDone(); // seems to be required.
        }
      }
    } // end radioReceiver


    
    /*  // below for testing radio Noise
    if (radioSender) // sending data, test data only.
      {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, 1); // LED blink
        HAL_TIM_Base_Start_IT(&htim16); // LED blink, interrupt based.
        radioData.nodeId = nodeID;
        radioData.uptime = HAL_GetTick();
        if(ledBlink){HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, 1); HAL_TIM_Base_Start_IT(&htim16); }// LED blink
        RFM69_send(toAddress, (const void *)(&radioData), sizeof(radioData), requestACK);
        //RFM69_sendWithRetry(toAddress, (const void *)(&radioData), sizeof(radioData), 3,20);
      }
    */



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
      huart1.hdmarx->Instance->CCR |= DMA_CCR_EN; // reset dma counter
      //json_parser("{G:RTC}"); // calling this loads json_response[] with a response.
      json_parser(rx_string); // calling this loads json_response[] with a response.
      while (!usart_tx_ready); // force wait while usart Tx finishes.
      sprintf(log_buffer, "{STM32:%s}\r\n", json_response);
      debug_printf(log_buffer);
    }
    // if (usart2_rx_flag)
    // {
    //   //-----
    //     //phase_corrections[0] = 13; // slap a value in there to test with.
    //     //hunt_PF[0] = true; // test powerfactor hunting on CT1.
    //   //-----
    //   usart2_rx_flag = 0;
    //   memcpy(rx_string, rx_buff, sizeof(rx_buff));
    //   memset(rx_buff, 0, sizeof(rx_buff));
    //   huart2.hdmarx->Instance->CCR &= ~DMA_CCR_EN;
    //   huart2.hdmarx->Instance->CNDTR = sizeof(rx_buff);
    //   huart2.hdmarx->Instance->CCR |= DMA_CCR_EN; // reset dma counter
    //   json_parser(rx_string); // calling this loads json_response[] with a response.
    //   while (!usart_tx_ready); // force wait while usart Tx finishes.
    //   sprintf(log_buffer, "{STM32:%s}\r\n", json_response);
    //   debug_printf(log_buffer);
    // }


    //-------------------------------
    // Print char buffers to serial.
    //-------------------------------
    // if (log_buffer[0] != '\0') { // null terminated strings!
    //   rPi_printf(log_buffer);
    //    // set index of string to null to effectively delete the string.
    //   // https://stackoverflow.com/questions/632846/clearing-a-char-array-c
    // }
    // if (log_buffer[0] != '\0') {
    //   rPi_printf(log_buffer);
    //   log_buffer[0] = '\0';
    // }
    

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
    if (debugFlag_pulse1) {
      debug_printf("Pulse Ch1 Detected.\r\n");
      debugFlag_pulse1 = false;
    }
    if (debugFlag_pulse2) {
      debug_printf("Pulse Ch2 Detected.\r\n");
      debugFlag_pulse2 = false;
    }
    if (debugFlag_Button1) {
      debug_printf("Button Pressed!\r\n");
      debugFlag_Button1 = false;
    }  
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

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_UART4
                              |RCC_PERIPHCLK_UART5|RCC_PERIPHCLK_I2C3
                              |RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_TIM16
                              |RCC_PERIPHCLK_TIM8;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_SYSCLK;
  PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_SYSCLK;
  PeriphClkInit.Uart5ClockSelection = RCC_UART5CLKSOURCE_SYSCLK;
  PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_SYSCLK;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
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
  if (GPIO_Pin == PULSE1_Pin) {
    debugFlag_pulse1 = true;    
    pulseCount1++;
  }
  if (GPIO_Pin == PULSE2_Pin) {
    debugFlag_pulse2 = true;    
    pulseCount2++;
  }
  if (GPIO_Pin == BUTTON1_Pin) {
    debugFlag_Button1 = true;    
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart1) {
    // HAL_UART_Transmit(&huart2, "test\r\n", 7, 1000);
    usart_tx_ready = true;
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
    while (!usart_tx_ready); // force wait while usart Tx finishes.
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
