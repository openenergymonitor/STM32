### RFM69 Test Code

STM32CubeMX v5.0.1
Tried and tested on the STM32F303RE Nucleo Board.
The RFM69.c has some untested code, an attempt at creating compatibility to receive JeeLib packets.

Pinout  
_Nucleo - RFM69_  
PA9 - DIO0  
PB3 - SEL  
PB13 - CLK  
PB14 - MISO  
PB15 - MOSI

PA5 for the Onboard LED  

RFM69 Library in C, for the STM32 <https://github.com/cristi85/RFM69>  

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
