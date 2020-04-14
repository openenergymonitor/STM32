#pi@emonpiisohv:~/pyspitest $ cat spitest.py 
# spitest.py
# A brief demonstration of the Raspberry Pi SPI interface, using the Sparkfun
# Pi Wedge breakout board and a SparkFun Serial 7 Segment display:
# https://www.sparkfun.com/products/11629

import time
import spidev

# We only have SPI bus 0 available to us on the Pi
bus = 0

#Device is the chip select pin. Set to 0 or 1, depending on the connections
device = 1

# Enable SPI
spi = spidev.SpiDev()

# Open a connection to a specific bus and device (chip select pin)
spi.open(bus, device)

# Set SPI speed and mode
spi.max_speed_hz = 500000
spi.mode = 0

# Clear display
msg = [0x76]
spi.xfer2(msg)

time.sleep(5)

# Turn on one segment of each character to show that we can
# address all of the segments
i = 1
while i < 0x7f:

    # The decimals, colon and apostrophe dots
    msg = [0x77]
    msg.append(i)
    result = spi.xfer2(msg)

    # The first character
    msg = [0x7b]
    msg.append(i)
    result = spi.xfer2(msg)

    # The second character
    msg = [0x7c]
    msg.append(i)
    result = spi.xfer2(msg)

    # The third character
    msg = [0x7d]
    msg.append(i)
    result = spi.xfer2(msg)

    # The last character
    msg = [0x7e]
    msg.append(i)
    result = spi.xfer2(msg)

    # Increment to next segment in each character
    i <<= 1

    # Pause so we can see them
    time.sleep(5)


# Clear display again
msg = [0x76]
spi.xfer2(msg)

