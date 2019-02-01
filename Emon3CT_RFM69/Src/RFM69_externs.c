#include "RFM69_externs.h"
#include "gpio.h"
#include "usart.h"
#include <stdbool.h>
#include <stdint.h>

bool GO_AHEAD_READ_RFM69;
bool rststate;
char log_buffer[30];


void RFM69_RST(void) {
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, 1);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, 0);
  HAL_Delay(50);
}

// extern functions
bool noInterrupts()                // function to disable interrupts
{
GO_AHEAD_READ_RFM69 = 0;
return true;
}

bool interrupts()                  // function to enable interrupts
{
GO_AHEAD_READ_RFM69 = 1;
return true;
}

void RFM69_SetCSPin(bool SetCSValue)          // function to control the GPIO tied to RFM69 chip select (parameter HIGH or LOW)
{
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, SetCSValue); //GPIOA, GPIO_PIN_5 is LED on Nucleao, for testing only.
}

bool RFM69_ReadDIO0Pin(void)       // function to read GPIO connected to RFM69 DIO0 (RFM69 interrupt signalling)
{
  if (GO_AHEAD_READ_RFM69 == 1 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) == 1)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/*
//functions below provided elsewhere.
uint8_t SPI_transfer8(uint8_t)     // function to transfer 1byte on SPI with readback
{

}


void SerialPrint(char*)            // function to print to serial port a string
{

}
*/
static uint32_t thistimeout1;
static uint32_t thistimeout2;

bool Timeout_IsTimeout1(void)      // function for timeout handling, checks if previously set timeout expired
{
  if(HAL_GetTick() >= thistimeout1)
    {
      sprintf(log_buffer,"TimeOut1!\r\n");
      debug_printf(log_buffer);
      return true;
    }
  else
  {
    return false;
  }
}

void Timeout_SetTimeout1(uint16_t settimeoutvalue) // function for timeout handling, sets a timeout, parameter is in milliseconds (ms)
{
  thistimeout1 = HAL_GetTick() + settimeoutvalue;
  //sprintf(log_buffer,"TimeOutSet %d and Millis = %d\r\n",thistimeout1, HAL_GetTick());
  //debug_printf(log_buffer);
}



bool Timeout_IsTimeout2(void)      // function for timeout handling, checks if previously set timeout expired
{
  if(HAL_GetTick() >= thistimeout2)
    {
      sprintf(log_buffer,"TimeOut2!\r\n");
      debug_printf(log_buffer);
      return true;
    }
  else
  {
    return false;
  }
}

void Timeout_SetTimeout2(uint16_t settimeoutvalue) // function for timeout handling, sets a timeout, parameter is in milliseconds (ms)
{
  thistimeout2 = HAL_GetTick() + settimeoutvalue;
}

/*
uint32_t rfm_timeout1 = 0;
uint32_t gettick = 0;
uint8_t setthistimeout = 0;

bool Timeout_IsTimeout1(void)      // function for timeout handling, checks if previously set timeout expired
{
  gettick = HAL_GetTick();
  if (gettick >= rfm_timeout1) {
  return true;
  }
  else {
    return false;
  }
}

void Timeout_SetTimeout1(uint8_t setthistimeout) // function for timeout handling, sets a timeout, parameter is in milliseconds (ms)
{
rfm_timeout1 == HAL_GetTick() + setthistimeout;
}
*/
