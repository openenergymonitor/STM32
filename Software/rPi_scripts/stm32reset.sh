#!/bin/bash

# stm32reset on rPi
# copy to /opt/emoncms/modules/usefulscripts/stm32/
# add the following line to /etc/rc.local (without the uncomment hash)

# /opt/emoncms/modules/usefulscripts/stm32/stm32reset.sh >> /var/log/stm32.log

echo "stm32reset.sh"

echo 4 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio4/direction
echo 0 > /sys/class/gpio/gpio4/value
echo "boot0 low - not in boot mode"
sleep 0.2

echo 17 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio17/direction
echo 0 > /sys/class/gpio/gpio17/value
sleep 0.2
echo 1 > /sys/class/gpio/gpio17/value
echo "NRST"
echo ""
sleep 0.2

echo 4 > /sys/class/gpio/unexport
echo 17 > /sys/class/gpio/unexport
