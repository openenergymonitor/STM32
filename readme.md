# STM32 Development

[STM32 Development thread](https://community.openenergymonitor.org/t/stm32-development)

---

## Option 1: PlatformIO

Install platformio http://docs.platformio.org/en/latest/installation.html#local-download-mac-linux-windows

    cd STM02
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
