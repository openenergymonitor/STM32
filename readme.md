# STM32 Development

[STM32 Development thread](https://community.openenergymonitor.org/t/stm32-development)

---

## Option 1: PlatformIO

Install platformio http://docs.platformio.org/en/latest/installation.html#local-download-mac-linux-windows

To download this repo, compile and upload with platformIO:

    git clone https://github.com/TrystanLea/STM32Dev.git
    cd STM32Dev/STM02
    pio run -t upload

To view serial output:

    pio device monitor

## Option 2: Using makefile

Compilation:

    make
    
Upload:

    cp build/STM01.bin /media/username/NODE_F401RE/
    
Serial monitor:

    minicom -F -b115200 -D/dev/ttyACM0

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

