// module interface, platform specific functions

#include "RFM69_ext.h"
#include "gpio.h"
#include "usart.h"
#include <stdbool.h>
#include <stdint.h>

bool GO_AHEAD_READ_RFM69 = true;
bool rststate;
//char log_buffer[30];


void RFM69_RST(void) {
  HAL_GPIO_WritePin(RFM_RST_GPIO_Port, RFM_RST_Pin, 1);
  HAL_Delay(5);
  HAL_GPIO_WritePin(RFM_RST_GPIO_Port, RFM_RST_Pin, 0);
  HAL_Delay(10);
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

void RFM69_SetCSPin(bool value)          // function to control the GPIO tied to RFM69 chip select (parameter HIGH or LOW)
{
HAL_GPIO_WritePin(SPI4_CS_RFM_GPIO_Port, SPI4_CS_RFM_Pin, value); //GPIOA, GPIO_PIN_5 is LED on Nucleao, for testing only.
}

bool RFM69_ReadDIO0Pin(void)       // function to read GPIO connected to RFM69 DIO0 (RFM69 interrupt signalling)
{
  if (GO_AHEAD_READ_RFM69 == true && HAL_GPIO_ReadPin(RFM_DIO0_GPIO_Port, RFM_DIO0_Pin) == true)
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
find me in spi.c
}


void SerialPrint(char*)            // function to print to serial port a string
{
find me in usart.c
}
*/

static uint32_t thistimeout1;
bool Timeout_IsTimeout1(void)      // function for timeout handling, checks if previously set timeout expired
{
  if(HAL_GetTick() >= thistimeout1)
    {
      while (!usart_tx_ready); // force wait while usart Tx finishes.
      sprintf(log_buffer,"TimeOut1!\r\n");
      debug_printf(log_buffer);
      return true;
    }
  else
  {
    return false;
  }
}
void Timeout_SetTimeout1(uint32_t settimeoutvalue) // function for timeout handling, sets a timeout, parameter is in milliseconds (ms)
{
  thistimeout1 = HAL_GetTick() + settimeoutvalue;
}
