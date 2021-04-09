// Native mode RF69 driver.

// Derived from the JeeLib RF69.h, with some small changes

// N.B. Default transmit power reduced to +7 dBm to avoid damage to RFM if no effective antenna is present.

#include <rf69.h>
#include <spi.h>
#include "usart.h"
// --------------------------------------------------------------------------

// void spi_master (uint8_t val) {}

void RFM69_SetCSPin(bool value) {
  HAL_GPIO_WritePin(GPIOE, SPI4_CS_RFM_Pin, value); //GPIOA, GPIO_PIN_5 is LED on Nucleao, for testing only.
}
// select the RFM69 transceiver (save SPI settings, set CS low)
void RF69_select() {
  RFM69_SetCSPin(0);
}

// unselect the RFM69 transceiver (set CS high, restore SPI settings)
void RF69_unselect() {
  RFM69_SetCSPin(1);
}

uint8_t spi_rwReg (uint8_t addr, uint8_t val) {    
  RF69_select();
  SPI_transfer8_RFM(addr);
  uint8_t regval = SPI_transfer8_RFM(val);
  RF69_unselect();
  return regval;
}

// --------------------------------------------------------------------------


uint8_t readReg(uint8_t addr)
{
  RF69_select();
  SPI_transfer8_RFM(addr & 0x7F);
  uint8_t regval = SPI_transfer8_RFM(0);
  RF69_unselect();
  return regval;
}

void writeReg(uint8_t addr, uint8_t value)
{
  RF69_select();
  SPI_transfer8_RFM(addr | 0x80);
  SPI_transfer8_RFM(value);
  RF69_unselect();
}

// --------------------------------------------------------------------------

void chThdYield () {

}

// --------------------------------------------------------------------------

void RF69_setMode (uint8_t newMode) {
  mode = newMode;
  writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | newMode);
  while ((readReg(REG_IRQFLAGS1) & IRQ1_MODEREADY) == 0)
    ;
}

void RF69_setFrequency (uint32_t hz) {
  // accept any frequency scale as input, including kHz and MHz
  // multiply by 10 until freq >= 100 MHz (don't specify 0 as input!)
  while (hz < 100000000)
    hz *= 10;

  // Frequency steps are in units of (32,000,000 >> 19) = 61.03515625 Hz
  // use multiples of 64 to avoid multi-precision arithmetic, i.e. 3906.25 Hz
  // due to this, the lower 6 bits of the calculated factor will always be 0
  // this is still 4 ppm, i.e. well below the radio's 32 MHz crystal accuracy
  // 868.0 MHz = 0xD90000, 868.3 MHz = 0xD91300, 915.0 MHz = 0xE4C000
  // 434.0 MHz = 0x6C8000.
  uint32_t frf = (hz << 2) / (32000000L >> 11);
  writeReg(REG_FRFMSB, frf >> 10);
  writeReg(REG_FRFMSB+1, frf >> 2);
  writeReg(REG_FRFMSB+2, frf << 6);
}

void RF69_configure (const uint8_t* p) {
  while (true) {
    uint8_t cmd = p[0];
    if (cmd == 0)
      break;
    writeReg(cmd, p[1]);
    p += 2;
  }
}

void RF69_init (uint8_t id, uint8_t group, int freq) {
  myId = id;

  // b7 = group b7^b5^b3^b1, b6 = group b6^b4^b2^b0
  parity = group ^ (group << 4);
  parity = (parity ^ (parity << 2)) & 0xC0;

  // 10 MHz, i.e. 30 MHz / 3 (or 4 MHz if clock is still at 12 MHz)
  // spi_master(3);
  RFM69_SetCSPin(1);
  
  do {
    writeReg(REG_SYNCVALUE1, 0xAA);
  } while (readReg(REG_SYNCVALUE1) != 0xAA);
  
  do {
    writeReg(REG_SYNCVALUE1, 0x55);
  } while (readReg(REG_SYNCVALUE1) != 0x55);
  
  RF69_configure(configRegs);
  RF69_configure(configRegs);
  RF69_setFrequency(freq);

  writeReg(REG_SYNCVALUE2, group);
}

void RF69_encrypt (const char* key) {
  uint8_t cfg = readReg(REG_PKTCONFIG2) & ~0x01;
  if (key) {
    for (int i = 0; i < 16; ++i) {
      writeReg(REG_AESKEYMSB + i, *key);
      if (*key != 0)
        ++key;
    }
    cfg |= 0x01;
  }
  writeReg(REG_PKTCONFIG2, cfg);
}

void RF69_txPower (uint8_t level) {
  writeReg(REG_PALEVEL, (readReg(REG_PALEVEL) & ~0x1F) | level);
}

void RF69_sleep () {
  RF69_setMode(MODE_SLEEP);
}

int8_t RF69_receive (void* ptr, uint8_t len) {
  if (mode != MODE_RECEIVE) {
    RF69_setMode(MODE_RECEIVE);
  } else {
    static uint8_t lastFlag;
    if ((readReg(REG_IRQFLAGS1) & IRQ1_RXREADY) != lastFlag) {
      lastFlag ^= IRQ1_RXREADY;
      if (lastFlag) { // flag just went from 0 to 1
        lna = (readReg(REG_LNAVALUE) >> 3) & 0x7;
//        rssi = readReg(REG_RSSIVALUE);  // It appears this can report RSSI of the previous packet.
#if RF69_SPI_BULK
        RF69_select();
        SPI_transfer8_RFM(REG_AFCMSB);
        afc = SPI_transfer8_RFM(0) << 8;
        afc |= SPI_transfer8_RFM(0);
        RF69_unselect();
#else
        afc = readReg(REG_AFCMSB) << 8;
        afc |= readReg(REG_AFCLSB);
#endif
      }
    }

    if (readReg(REG_IRQFLAGS2) & IRQ2_PAYLOADREADY) {

#if RF69_SPI_BULK
      RF69_select();
      SPI_transfer8_RFM(REG_FIFO);
      uint8_t count = SPI_transfer8_RFM(0);
      for (uint8_t i = 0; i < count; ++i) {
        uint8_t v = SPI_transfer8_RFM(0);
        if (i < len)
          ((uint8_t*) ptr)[i] = v;
      }
      RF69_unselect();
#else
      rssi = readReg(REG_RSSIVALUE);   // Duplicated here - RW
      uint8_t count = readReg(REG_FIFO);
      for (uint8_t i = 0; i < count; ++i) {
        uint8_t v = readReg(REG_FIFO);
        if (i < len)
          ((uint8_t*) ptr)[i] = v;
      }
#endif

      // only accept packets intended for us, or broadcasts
      // ... or any packet if we're the special catch-all node
      uint8_t dest = *(uint8_t*) ptr;
      if ((dest & 0xC0) == parity) {
        uint8_t destId = dest & 0x3F;
        if (destId == myId || destId == 0 || myId == 63) {
          return count;
        }
      }
    }
  }
  return -1;
}

void RF69_send (uint8_t header, const void* ptr, int len) {
  RF69_setMode(MODE_SLEEP);

#if RF69_SPI_BULK
  RF69_select();
  SPI_transfer8_RFM(REG_FIFO | 0x80);
  SPI_transfer8_RFM(len + 2);
  SPI_transfer8_RFM((header & 0x3F) | parity);
  SPI_transfer8_RFM((header & 0xC0) | myId);
  for (int i = 0; i < len; ++i)
    SPI_transfer8_RFM(((const uint8_t*) ptr)[i]);
  RF69_unselect();
#else
  writeReg(REG_FIFO, len + 2);
  writeReg(REG_FIFO, (header & 0x3F) | parity);
  writeReg(REG_FIFO, (header & 0xC0) | myId);
  for (int i = 0; i < len; ++i)
    writeReg(REG_FIFO, ((const uint8_t*) ptr)[i]);
#endif

  RF69_setMode(MODE_TRANSMIT);
  while ((readReg(REG_IRQFLAGS2) & IRQ2_PACKETSENT) == 0)
    chThdYield();

  RF69_setMode(MODE_STANDBY);
}
