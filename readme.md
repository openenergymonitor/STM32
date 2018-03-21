# STM32 Development

[STM32 Development thread](https://community.openenergymonitor.org/t/stm32-development)

---

## PlatformIO

    cd STM02
    sudo pio run -t upload

## Using makefile

Compilation:

    make
    
Upload:

    cp build/STM01.bin /media/username/NODE_F401RE/
    
Serial monitor:

    minicom -F -b115200 -D/dev/ttyACM0
