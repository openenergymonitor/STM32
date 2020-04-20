### Emon3CT & RFM69 Code Merge

Tried and tested on the STM32F303RE Nucleo Board.
STM32CubeMX v4.25.0
LowPowerLab RFM69 library based Tx/Rx code.  

![Wiring](../images/rfm69_images/emon3ct_rfm69_wiring.jpg)

![Pinout](../images/rfm69_images/emon3ct_rfm69_cubemx_pinout.jpg)


See RFM69 doco page and RFM69 project folder for details on the radio implementation.  

CubeMX was used again to initialize the extra SPI and GPIO pins, as per RFM69 documentation.  

The Makefile was modified to include the RFM69 C sources.  

Clicking 'Generate Code' in CubeMX sometimes broke the code.. Optimisation level '-O3' cause a break.  
The option -Og worked (debug optimised).  
Method found here:  
<http://blog.atollic.com/optimizing-code-size-with-the-gnu-gcc-compiler-for-stm32-and-other-arm-cortex-m-targets>

The RFM69 Receive and transmit code is instantiated after the ADC/DMA interrupts.

Main.c contains the radio initialization code.  
Adc.  contains the receive and transmit sample code, the RFM69 module is communicated with **after** the ADC/DMA interrupts.

    /* USER CODE BEGIN 1 */
    void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
    {
      if (hadc==&hadc2) process_frame(0);
      RunReceive();
    }

    void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
    {
      if (hadc==&hadc2) process_frame(3000);
      RunReceive();
    }
