make && stm32flash -w ./build/Sombrero_VE_Working.bin -b 115200 -R -i rts /dev/tty.SLAB_USBtoUART && pio device monitor -b 115200 --rts 0

stm32flash -w ./build/Somebrero_VC.bin -b 115200 -R -i rts /dev/ttyUSB0