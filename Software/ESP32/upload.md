The STM32mini board has an integrated CP2102 USB-Serial chip to communicate with the ESP32.

https://github.com/espressif/esptool/wiki/ESP32-Boot-Mode-Selection

The reset and boot-mode pins of the ESP connect as follows: conne
CP2102 | ESP32
---
CTS | GPIO0(BOOT)
RTS | EN
Tx | UART1_Rx
Rx | UART1_Tx

The ESP32 is powered from 3V3. GPIO0 has an internal pullup. GPIO2 best unconnected or on a pullup.
GPIO 12 & 15 are best left unconnected.

Traces exist on this board to connect in the following ways:
- I2C between rPi, ESP32, and STM32.
- UART between rPi and STM32
- SPI between rPi, STM32 and ESP32... modes detailed below:

Master | Slave
---
rPi | ESP32
