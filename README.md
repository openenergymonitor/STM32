# STM32 Energy Monitoring

The STM32 platform is a family of microcontrollers based on the Arm Cortex M processor, offering among many powerful features, plenty of 12-bit analog inputs and high speed sampling, making them particularly suitable for energy monitoring applications.

The following resources are a work in progress guide to using the STM32 platform for energy monitoring, being written as part of development work into the next generation of OpenEnergyMonitor hardware. To be included as a section in [http://learn.openenergymonitor.org](http://learn.openenergymonitor.org)

### OpenEnergyMonitor Forum threads:

- [STM32 Development thread](https://community.openenergymonitor.org/t/stm32-development)
- [STM32 Hardware Development](https://community.openenergymonitor.org/t/stm32-hardware-development/7135)
- [STM32 PlatformIO](https://community.openenergymonitor.org/t/stm32-platformio/7015)

### Getting started: STM32 (Arduino integration)

If you are familiar with the Arduino platform getting the basics working using the familiarity of the Arduino IDE and the STM32Dunio Arduino integration is a good place to start.

- [1. Blinking an LED using the NUCLEO-F303RE Development board & STM32Duino](docs/STM32Duino/Blink.md)
- [2. Basic NUCLEO-F303RE energy monitor using an EmonTxShield & EmonLib discreet sampling STM32Duino library](docs/STM32Duino/EmonLib.md)

### Introducing STM32CubeMX

ADC access using the Arduino analogRead command gives limited performance, its possible to sample much faster across many channels by using the lower level STM32 HAL (Hardware Access Layer) provided by ST. The development pathway to access these features is different and quite daunting if your primarily familiar with the Arduino platform. There is a tool called STM32CubeMX which is a kind of project builder that you can use to generate the initial outline of your project, from there you can enter your own 'user code' into the relevant placeholders in the generated project. The following set of guides give an introduction to this process:

- [1. Blink](docs/Blink.md)
- [2. Serial](docs/Serial.md)
- [3. Analog](docs/Analog.md)
- [4. DMA](docs/DMA.md)
- [5. RFM69](docs/RFM69.md)


### Hardware

Notes on hardware development and initial designs:

- [1. ST-LINK nucleo](docs/ST-LINK.md)
- [2. ST-LINK adapters](docs/st-link2.md)
- [3. Serial/UART Upload](docs/uartupload.md)
- [4. RaspberryPi UART Upload + autoreset](docs/rpiautoupload.md)

**STM32-pi_basic**

- [Eagle design 01](Hardware/stm32-pi_basic/1)
- [Prototype 1, breadboard, voltage follower & anti-alias](docs/prototype1.md)
- [STM32-pi_basic eagle design 02](Hardware/stm32-pi_basic/2)
- [STM32-pi_basic eagle design 03](Hardware/stm32-pi_basic/3)
- [STM32-pi_basic eagle design 04](Hardware/stm32-pi_basic/4)
- [Design Notes v4](docs/stm32notes.md)
- [STM32-pi_basic eagle design 05](Hardware/stm32-pi_basic/5)

**STM32-pi_full**

- [Hardware/stm32-pi_full](Hardware/stm32-pi_full)

**Misc**

- [Flashing a new chip](docs/Blink-fresh-chip.md)

### Firmware Examples

Firmware examples included in this repository:

- [1. Blink](Software/Blink)
- [2. ADC](Software/ADC)
- [3. DMA](Software/DMA)
- [4. Emon](Software/Emon): EmonTxShield Voltage and CT1 current measurement, single ADC example.
- [5. Emon1CT](Software/Emon1CT): EmonTxShield Voltage (ADC1) and CT3 current measurement (ADC2) example.
- [6. Emon1CT_ds18b20](Software/Emon1CT_ds18b20): EmonTxShield Voltage (ADC1) and CT3 current measurement (ADC2) example with DS18B20 temperature measurement.
- [7. Emon3CT](Software/Emon3CT): EmonTxShield Voltage (ADC1) and 3x CT inputs on ADC2.
- [8. Emon3CT_CB](Software/Emon3CT_CB): Firmware for [Hardware/stm32-pi_basic/5](Hardware/stm32-pi_basic/5)
- [9. Emon3CT_CB_v2](Software/Emon3CT_CB_v2): Firmware for [Hardware/stm32-pi_basic/5](Hardware/stm32-pi_basic/5) v2.
- [10. Emon3CT_RFM69](Software/Emon3CT_RFM69): EmonTxShield Voltage (ADC1), 3x CT inputs on ADC2 and RFM69 support.
- [11. Emon3CT_VET](Software/Emon3CT_VET): Basic firmware for [Hardware/stm32-pi_full](Hardware/stm32-pi_full) v2 by Trystan Lea.
- [12. emonTxshield_dBC (v13)](Software/emonTxshield_dBC): Mutli-channel energy monitor example firmware thanks to @dBC see [https://community.openenergymonitor.org/t/stm32-development/6815/232](https://community.openenergymonitor.org/t/stm32-development/6815/232)
- [13. RFM69](Software/RFM69): RFM69 library and examples.
- [14. MBUS](Software/MBUS): Example of reading data from an MBUS meter using Serial and DMA's.

**STM32 Pi Full**

- [STM32 pi v0.7 firmware basics](docs/stm32-pi.md)
- [Sombrero_VB_Blink](Software/Sombrero_VB_Blink)
- [Sombrero_VE_ADC-test](Software/Sombrero_VE_ADC-test)
- [Sombrero_VE_Blink](Software/Sombrero_VE_Blink)
- [Sombrero_VE_Working5](Sombrero_VE_Working5)

### Other:

- [STM32F103 BluePill Blink](docs/bluepill.md)
- [CAD files from ST](docs/cad-files.md)
- [CAD file of the enclosure](https://a360.co/3kcYF79)
