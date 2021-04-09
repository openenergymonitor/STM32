// Native mode RF69 driver.

// Derived from the JeeLib RF69.h, with some small changes

// N.B. Default transmit power reduced to +7 dBm to avoid damage to RFM if no effective antenna is present.

#ifndef RF69_h
#define RF69_h

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

enum {
  REG_FIFO          = 0x00,
  REG_OPMODE        = 0x01,
  REG_FRFMSB        = 0x07,
  REG_PALEVEL       = 0x11,
  REG_LNAVALUE      = 0x18,
  REG_AFCMSB        = 0x1F,
  REG_AFCLSB        = 0x20,
  REG_FEIMSB        = 0x21,
  REG_FEILSB        = 0x22,
  REG_RSSIVALUE     = 0x24,
  REG_IRQFLAGS1     = 0x27,
  REG_IRQFLAGS2     = 0x28,
  REG_SYNCVALUE1    = 0x2F,
  REG_SYNCVALUE2    = 0x30,
  REG_NODEADDR      = 0x39,
  REG_BCASTADDR     = 0x3A,
  REG_FIFOTHRESH    = 0x3C,
  REG_PKTCONFIG2    = 0x3D,
  REG_AESKEYMSB     = 0x3E,

  MODE_SLEEP        = 0<<2,
  MODE_STANDBY      = 1<<2,
  MODE_TRANSMIT     = 3<<2,
  MODE_RECEIVE      = 4<<2,

  START_TX          = 0xC2,
  STOP_TX           = 0x42,

  RCCALSTART        = 0x80,
  IRQ1_MODEREADY    = 1<<7,
  IRQ1_RXREADY      = 1<<6,
  IRQ1_SYNADDRMATCH = 1<<0,

  IRQ2_FIFONOTEMPTY = 1<<6,
  IRQ2_PACKETSENT   = 1<<3,
  IRQ2_PAYLOADREADY = 1<<2,
};

static const uint8_t configRegs [] = {
// POR value is better for first rf_sleep  0x01, 0x00, // OpMode = sleep
  0x02, 0x00, // DataModul = packet mode, fsk
  0x03, 0x02, // BitRateMsb, data rate = 49,261 bits/s
  0x04, 0x8A, // BitRateLsb, divider = 32 MHz / 650
  0x05, 0x02, // FdevMsb = 45 kHz
  0x06, 0xE1, // FdevLsb = 45 kHz
  0x0B, 0x20, // Low M
  0x11, 0x99, // OutputPower = +7 dBm - was default = max = +13 dBm
  0x19, 0x4A, // RxBw 100 kHz
  0x1A, 0x42, // AfcBw 125 kHz
  0x1E, 0x0C, // AfcAutoclearOn, AfcAutoOn
  //0x25, 0x40, //0x80, // DioMapping1 = SyncAddress (Rx)
  0x26, 0x07, // disable clkout
  0x29, 0xA0, // RssiThresh -80 dB
  0x2D, 0x05, // PreambleSize = 5
  0x2E, 0x88, // SyncConfig = sync on, sync size = 2
  0x2F, 0x2D, // SyncValue1 = 0x2D
  0x37, 0xD0, // PacketConfig1 = variable, white, no filtering
  0x38, 0x42, // PayloadLength = 0, unlimited
  0x3C, 0x8F, // FifoThresh, not empty, level 15
  0x3D, 0x12, // 0x10, // PacketConfig2, interpkt = 1, autorxrestart off
  0x6F, 0x20, // TestDagc ...
  0x71, 0x02, // RegTestAfc
  0
};

uint8_t mode;
uint8_t myId;
uint8_t parity;

uint8_t lna;
uint8_t afc;
uint8_t rssi;

void RF69_setMode (uint8_t newMode);
void RF69_setFrequency (uint32_t hz);
void RF69_configure (const uint8_t* p);
void RF69_init (uint8_t id, uint8_t group, int freq);
void RF69_encrypt (const char* key);
void RF69_txPower (uint8_t level);
void RF69_sleep ();
int8_t RF69_receive (void* ptr, uint8_t len);
void RF69_send (uint8_t header, const void* ptr, int len);

#endif
