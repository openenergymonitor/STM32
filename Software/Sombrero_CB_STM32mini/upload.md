make && stm32flash -w ./build/Emon3CT_CB.bin -b 115200 -R -i rts /dev/ttyUSB0 && pio device monitor -b115200

stm32flash -w ./build/Somebrero_VC.bin -b 115200 -R -i rts /dev/ttyUSB0