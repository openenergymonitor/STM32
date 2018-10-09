# STM32 Development

- [STM32 Development thread](https://community.openenergymonitor.org/t/stm32-development)

Getting started with STM32CubeMX:

- [1. Blink](docs/Blink.md)
- [2. Serial](docs/Serial.md)
- [3. Analog](docs/Analog.md)
- [4. DMA](docs/DMA.md)

## Installation

Install toolchain
    
    sudo apt-get install gcc-arm-none-eabi
    
Clone this repo:

    git clone https://github.com/TrystanLea/STM32Dev.git

## Blink, using makefile

    cd STM32Dev/Blink
    make
    cp build/Blink.bin /media/username/NODE_F303RE

## Blink: PlatformIO

Install platformio http://docs.platformio.org/en/latest/installation.html#local-download-mac-linux-windows

    cd STM32Dev/Blink
    pio run -t upload

## Arduino IDE: STM32Duino

Preferences: Additional boards manager link

    https://github.com/stm32duino/BoardManagerFiles/raw/master/STM32/package_stm_index.json
    
Select STM32 Cores from Boards Manager, install or update.

To test: select example 'blink', Board: Nucleo-64 & Part number: Nucleo F303RE.
Compile and upload.

**EmonTxShield with EmonLib**

Clone EmonLib into your Arduino libraries folder using git.

    git clone https://github.com/openenergymonitor/EmonLib.git
    git checkout STM32
    
Open example 'voltage_and_current'. See post for Shield hardware modifications:
[https://community.openenergymonitor.org/t/stm32-development/6815/4](https://community.openenergymonitor.org/t/stm32-development/6815/4)

