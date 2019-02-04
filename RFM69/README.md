### RFM69 Test Code

Tried and tested on the STM32F303RE Nucleo Board.
STM32CubeMX v4.27.0
LowPowerLab RFM69 library based Tx/Rx code.
The RFM69.c has some untested code, an attempt at creating compatibility to receive JeeLib packets.

Pinout  
_Nucleo - RFM69_  
PA9 - DIO0  
PB3 - SEL  
PB13 - CLK  
PB14 - MISO  
PB15 - MOSI

PA5 for the Onboard LED  

RFM69 Library written in C, based on LowPowerLab library.   <https://github.com/cristi85/RFM69>  
Thanks to @cristi85.

The primary change made to the library was to add the following line to RFM69.c/.h

    #define RFM69_DATA(x) data[x]

The basic usage in main.c is as follows.

    /* Private variables ---------------------------------------------------------*/

    uint16_t networkID = 210; // a.k.a. Network Group
    uint8_t nodeID = 1;
    uint16_t freqBand = 433;
      /*
      // available frequency bands
      #define RF69_315MHZ            315
      #define RF69_433MHZ            433
      #define RF69_868MHZ            868
      #define RF69_915MHZ            915
      see registers.h for more.
      */
    uint8_t toAddress = 1;
    bool requestACK = false;

    int main(void) {
        if (RFM69_initialize(freqBand, nodeID, networkID)) {
          sprintf(log_buffer, "RFM69 Initialized. Freq %dMHz. Node %d. Group %d.\r\n", freqBand, nodeID, networkID);
          debug_printf(log_buffer);
        }
        else {
          sprintf(log_buffer, "RFM69 not connected.\r\n");
          debug_printf(log_buffer);
        }

        /* Infinite loop */
        /* USER CODE BEGIN WHILE */
        while (1) {        
          // SAMPLE RECEIVE CODE
          if (RFM69_ReadDIO0Pin()) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1); // turn on LED
            RFM69_interruptHandler();
          }
          if (RFM69_receiveDone()) {
            debug_printf("Payload Received!\r\n");
            PrintRawBytes();
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0); // turn off LED
          }
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
      }
      /* USER CODE END 3 */
    }

ATMEGA_test_examples folder contains LowPowerLabs Tx/Rx examples for Arduino / ATMEGA, used for testing.  
Memory structure differences between STM32 and ATMEGA can result in the struct not working the same between the two.

    typedef struct {
      int nodeId; //store this nodeId
      unsigned long uptime; //uptime in ms
    } Payload;
    Payload theData;

This will result in a 6 byte struct on the ATMEGA and a 8 byte struct on the STM32.  
<https://stackoverflow.com/questions/119123/why-isnt-sizeof-for-a-struct-equal-to-the-sum-of-sizeof-of-each-member>  
For this reason three different methods were found for printing the received data on the STM32 before deciding which is the method to use, included in RFM69.c
