#include <string.h>
#include <stdio.h>
#include <math.h>
#include "main.h"
#include "usart.h"
#include "ds18b20.h"

#define ONE_WIRE_SKIP_ROM 0xCC
#define ONE_WIRE_MATCH_ROM 0x55
#define ONE_WIRE_READ_ROM 0x33
#define ONE_WIRE_READ_SCRATCH 0xBE
#define ONE_WIRE_CONVERT_T 0x44

static uint8_t tx_buff[10];
static uint8_t rx_buff[MAX_1WIRE_BUS][10];
static uint8_t romaddr[MAX_1WIRE_BUS][8];
static uint8_t bus_status[MAX_1WIRE_BUS], crc_status[MAX_1WIRE_BUS];
static float temp_f[MAX_1WIRE_BUS];
static int16_t temp_i[MAX_1WIRE_BUS];

char log_buffer[100];

//
// This code is NOT an example
// of how to use the DS18B20, in fact, it’s an example of how NOT to use it.
// It simply uses two DS18B20s (one per bus) as a handy OneWire slave in order
// to demo/test UART off-load of OneWire transactions. As such it does no device
// discovery.  It will only work provided there
// is only one DS18B20 per bus. There are plenty of examples on the net implementing
// the full DS18B20 discovery and addressing stuff, use them instead if you’re
// trying to do anything serious with DS18B20s. Here they’re just a handy OneWire
// slave to bang against in a very primitive fashion.
//


// This table comes from Dallas sample code where it is freely reusable,
// though Copyright (C) 2000 Dallas Semiconductor Corporation
static const uint8_t dscrc_table[] = {
      0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
    157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
     35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
    190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
     70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
    219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
    101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
    248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
    140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
     17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
    175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
     50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
    202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
     87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
    233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
    116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

static uint8_t crc8(const uint8_t *addr, uint8_t len)
{
  uint8_t crc = 0;
  
  while (len--) {
    crc = dscrc_table[crc ^ *addr++];
  }
  return crc;
}

void init_ds18b20s (void) {

  init_uarts();

  for (int i=0; i<MAX_1WIRE_BUS; i++)
    bus_status[i] = onewire_reset(i);

  tx_buff[0] = ONE_WIRE_READ_ROM;
  for (int i=0; i<MAX_1WIRE_BUS; i++)
    onewire_tx(i, tx_buff, 1);

  for (int i=0; i<MAX_1WIRE_BUS; i++) 
    onewire_rx1(i, romaddr[i], 8);    // get all 8 bytes of rom address - rx1 starts the xfer

  for (int i=0; i<MAX_1WIRE_BUS; i++) 
    onewire_rx2(i, romaddr[i], 8);    // get all 8 bytes of rom address - rx2 fetches the xfer

  for (int i=0; i<MAX_1WIRE_BUS; i++) 
    crc_status[i] = crc8(romaddr[i], 8);

  for (int i=0; i<MAX_1WIRE_BUS; i++) {
    snprintf(log_buffer, sizeof(log_buffer),
	     "bus%d status: 0x%02x, crc: %s, addr: 0x%02x%02x%02x%02x%02x%02x%02x%02x \r\n",
	     i, bus_status[i], crc_status[i] ? "bad" : " ok",
	     romaddr[i][0], romaddr[i][1], romaddr[i][2], romaddr[i][3], 
	     romaddr[i][4], romaddr[i][5], romaddr[i][6], romaddr[i][7]);
    debug_printf(log_buffer);
  }
 }

void process_ds18b20s (void) {

  static uint32_t last_readout;
  static uint8_t conversion_in_progress;

  //
  // Dump out the temp readings every 10 seconds
  // It takes nearly a second to do the conversion at 12-bit
  // resolution, so we start that 1 second before we're due
  // to report.
  //
  if ((HAL_GetTick() - last_readout > 9000) && !conversion_in_progress) {

    for (int i=0; i<MAX_1WIRE_BUS; i++)
      bus_status[i] = onewire_reset(i);
    tx_buff[0] = ONE_WIRE_SKIP_ROM;          // Tell all....
    tx_buff[1] = ONE_WIRE_CONVERT_T;         // ... to convert.
    for (int i=0; i<MAX_1WIRE_BUS; i++)      // both buses
      onewire_tx(i, tx_buff, 2);

    conversion_in_progress = 1;

  } else if (HAL_GetTick() - last_readout > 10000) {

    for (int i=0; i<MAX_1WIRE_BUS; i++)
      bus_status[i] = onewire_reset(i);

    tx_buff[0] = ONE_WIRE_MATCH_ROM;            // This example, we select the device
    tx_buff[9] = ONE_WIRE_READ_SCRATCH;         // and request the results via the scratchpad
    for (int i=0; i<MAX_1WIRE_BUS; i++) {
      memcpy(&tx_buff[1], romaddr[i], 8);       // Copy in the ROM address
      onewire_tx(i, tx_buff, 10);               // Tell the device we want the scratchpad
    }
    for (int i=0; i<MAX_1WIRE_BUS; i++)
      onewire_rx1(i, rx_buff[i], 9);            // get all 9 bytes of scratch pad - rx1 starts the xfer

    for (int i=0; i<MAX_1WIRE_BUS; i++) {
      onewire_rx2(i, rx_buff[i], 9);            // get all 9 bytes of scratch pad - rx2 fetches the xfer
      crc_status[i] = crc8(rx_buff[i], 9);
      temp_i[i] = ((int16_t)rx_buff[i][1] << 11 | (int16_t)rx_buff[i][0] << 3);
      temp_f[i] = temp_i[i] * 0.0078125;
      snprintf(log_buffer, sizeof(log_buffer),
	       "bus%d status: 0x%02x, crc: %s, temp: %.3fC\r\n", i,
	       bus_status[i], crc_status[i] ? "bad" : " ok", temp_f[i]);
      debug_printf(log_buffer);
    }

    conversion_in_progress = 0;
    last_readout = HAL_GetTick();
  }
}
