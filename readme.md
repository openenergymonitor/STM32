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
- [Eagle design 01](Hardware/1)
- [Prototype 1, breadboard, voltage follower & anti-alias](docs/prototype1.md)
- [Eagle design 02](Hardware/2)
- [Eagle design 03](Hardware/3)
- [Design Notes v4](docs/stm32notes.md)

### Firmware Examples

Firmware examples included in this repository:

- [1. Blink](Blink)
- [2. ADC](ADC)
- [3. DMA](DMA)
- [4. Emon](Emon): EmonTxShield Voltage and CT1 current measurement, single ADC example.
- [5. Emon1CT](Emon1CT): EmonTxShield Voltage (ADC1) and CT3 current measurement (ADC2) example.
- [6. Emon3CT](Emon3CT): EmonTxShield Voltage (ADC1) and 3x CT inputs on ADC2.
- [7. emonTxshield_dBC (v13)](emonTxshield_dBC): Latest mutli-channel energy monitor example firmware thanks to @dBC see [https://community.openenergymonitor.org/t/stm32-development/6815/232](https://community.openenergymonitor.org/t/stm32-development/6815/232)
- [8. RFM69](RFM69): RFM69 library and examples.


### Other:

- [STM32F103 BluePill Blink](docs/bluepill.md)
- [CAD files from ST](docs/cad-files.md)
