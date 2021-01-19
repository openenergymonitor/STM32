#ifndef __power_H
#define __power_H

#ifdef __cplusplus
 extern "C" {
#endif


#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "adc.h"

const double VOLTS_PER_DIV;
const double VCAL;
const double ICAL;

//---------------------------------------------
// Calculate Power Data One Channel at a Time
//---------------------------------------------
double Vrms;
double Irms;
double realPower;
double apparentPower;
double powerFactor;

double mains_frequency;
double Ws_accumulator[CTn];

bool readings_ready;
bool readings_requested;

double V_SCALE;
double I_SCALE;

// approximating the BIAS.
#define MID_ADC_READING 2048

//------------------------
// Channel Accumulators
//------------------------
typedef struct channel_
{
  int64_t sum_P;
  uint64_t sum_V_sq;
  uint64_t sum_I_sq;
  int64_t sum_V;
  int64_t sum_I;
  bool Iclipped; 
  uint32_t samplecount;
  uint32_t cycles;
  bool positive_V;
  bool last_positive_V;
} channel_t; // data struct per CT channel, stm32 only does 32-bit struct items, anything else gets padded out to 32-bit automatically by

channel_t channels[CTn];
channel_t channels_ready[CTn];
bool channel_rdy_bools[CTn];



void calcPower (int ch);
void calcEnergy (int ch, double power, int interval);
void process_frame (uint16_t offset);



#ifdef __cplusplus
}
#endif
#endif