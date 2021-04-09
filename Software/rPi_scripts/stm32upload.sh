#!/bin/bash

# example flash command: $ stm32flash -w ./binName.bin -b 115200 /dev/tty.SLAB_USBtoUART

STM32FLASHBIN = "./binName.bin"

echo 4 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio4/direction
echo 1 > /sys/class/gpio/gpio4/value
echo "boot0 high"
sleep 0.2

echo 17 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio17/direction
echo 0 > /sys/class/gpio/gpio17/value
sleep 0.2
echo 1 > /sys/class/gpio/gpio17/value
echo "NRST"
sleep 0.2


### device info / port check ###
# stm32flash -w /dev/ttyAMA0


### device flash ###
stm32flash -w $STM32FLASHBIN -b 115200 /dev/ttyAMA0


sleep 0.2

echo 0 > /sys/class/gpio/gpio4/value
echo "boot0 low"
sleep 0.2

echo 0 > /sys/class/gpio/gpio17/value
sleep 0.2
echo 1 > /sys/class/gpio/gpio17/value
echo "NRST"
sleep 0.2

echo 4 > /sys/class/gpio/unexport
echo 17 > /sys/class/gpio/unexport
