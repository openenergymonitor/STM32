v0.9 main changes

- ferrites and caps on USB input to suppress noise from the power supply.
- added extra 3.3V regulator for analog supplies and revised the capacitor selection of the other.
- ferrites on each regulator supply input
- inner-layer 3V3 copper has separated pours for DIGITAL and ANALOG supplies
- ground inner pours separated into ANALOG ground, DIGITAL ground, and RADIO ground.
- ferrites for separating each GND’s to final GND (for testing).
- reset line has a blocking capacitor (I think) of the correct value now, calculated from datasheet information.
- The MCU’s integrated op-amp use for the BIAS now has it’s inverting input broken out for filter testing
- UART at south-west of board added accessible from outside the enclosure, connected to the shared STM32 and rPi UART… (UART1) I might revise this to UART 2 if there’s a clash trying to upload firmware while rPi connected.
- CT burdens changed to 2x parallel 0805 0.1% resistors for cost reasons, through-hole resistor option remains.
- VT inputs for SINGLE phase and THREE phase now have separated input chains all the way to the ADC channel inputs, previous versions tried to combine the AC-AC adaptor and L1 of the 3-phase input – Single phase retains the conventional barrel-jack and voltage divider for the 9V AC-AC adaptor, while the Terminal Block for three phase has a dedicated path to the STM32 ADC channels. Three-phase inputs at this terminal block would be 3.3V peak-to-peak max, sans DC offset.
- A lot more protection diodes, everywhere
- Expansion port is perhaps under-utilised - worth discussing.
- RFM69CW tuning capacitor?
- microSD card footprint included for use with standalone stm32 or esp32
- SPI1 and SPI4 selected for marginal power saving. SPI4 for the rfm69, SPI1 for SD card and comms between stm32, rPi, or ESP32.
