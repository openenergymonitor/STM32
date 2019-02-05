### Emon3CT & RFM69 Code Merge

Tried and tested on the STM32F303RE Nucleo Board.
STM32CubeMX v4.27.0
LowPowerLab RFM69 library based Tx/Rx code.  

See RFM69 project folder for details on the radio implementation.  

CubeMX was used again to initialise the extra SPI and GPIO pins.

Clicking 'Generate Code' set the Makefile option, optimisation, to -O3 and this cause a break. The option -Og worked (debug optimised).  
Method found here:  
<http://blog.atollic.com/optimizing-code-size-with-the-gnu-gcc-compiler-for-stm32-and-other-arm-cortex-m-targets>
