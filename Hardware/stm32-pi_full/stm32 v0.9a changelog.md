v0.9 main changes


<h3>USB input filtering</h3>

 - ferrites and caps to help suppress noise from the power supply.

<h3>3.3V regulators</h3>

 - added extra regulator and revised the capacitor selection
 - ferrites on each regulator supply input
 - inner-layer 3V3 has separated pours for DIGITAL and ANALOG supplies.

<h3>STAR GND</h3>

 - ground separated into ANALOG ground, DIGITAL ground, and RADIO ground.
 - ferrites for separating each’s high frequency noise.

<h3>Battery Coin cell input to STM32</h3>

 - two diodes provide the voltage drop necessary to power the battery from 5V, keeping it charged (to be tested).
 -  current limiting resistor included in event of short circuited cell.

<h3>STM32 RESET now has a blocking capacitor</h3> 

- Values capacitor and resistor selected for time constant necessary to cause reset, pull-up resistor ensures capacitor is discharged - we were having problems with this before.

<br> 
<br> 
<br> 

- <h3>The MCU's integrated op-amp use for the BIAS now has it's inverting input broken out for filter testing</h3>

- <h3>UART at south-west of board connected to STM32 and rPi UART… (UART1)

- <h3>CT burdens changed to 2x parallel 0805 0.1% resistors, TH resistor option remains - values to be looked at again at the suggestion of phase errors becoming significant with CT output level 1.1Vp-p <h3>

- <h3>VT inputs for SINGLE phase and THREE phase have completely separated input chains all the way to the ADC channel inputs – Single phase retains the conventional barrel-jack and voltage divider for the 9V AC-AC adaptor, while the Terminal Block for three phase has a dedicated path to the STM32 ADC channels. THREE phase input at this terminal block to be 3.3V peak-to-peak max, sans DC offset.</h3>

- <h3>A lot more protection diodes, everywhere</h3>

- <h3>Expansion port is perhaps under-utilised - worth discussing.</h3>

- <h3>RFM69CW tuning capacitor?</h3>

- <h3>microSD card footprint included for use with standalone stm32 or esp32</h3>

- <h3>SPI1 and SPI4 selected for marginal power saving. SPI4 for the rfm69, SPI1 for SD card and comms between stm32, rPi, or ESP32.</h3>