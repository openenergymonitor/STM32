make && stm32flash -w ./build/Sombrero_VE_Working.bin -b 115200 -R -i rts /dev/ttyUSB0 && pio device monitor -b 115200 --rts 0

stm32flash -w ./build/Sombrero_VE_Working.bin -b 115200 -R -i rts /dev/ttyUSB0