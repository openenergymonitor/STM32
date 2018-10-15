
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "main.h"
#include "usart.h"
#include "adc.h"
#include "power.h"
#include "calib.h"

#define ENERGY_MONITORING 1

//
// Interval power stats are where the stats get moved to when
// process_VI_pair() has decided it's accumulated enough
// (147,059 currently... about 10 seconds worth).
// process_power_stats() works on these intervals stats once they're
// ready.
//
static volatile power_stats_t intvl_power_stats[MAX_CHANNELS];


#ifdef DUMPING
static volatile uint16_t dump[DUMP_CHANS][DUMP_MAX];
static volatile int dump_index[DUMP_CHANS];
#endif

//
// Called from the ADC/DMA interrupt when a new V,I pair has arrived.
// Basically does the accumulation maths on the new pair.  It starts
// accumulating after the first zero crossing on V, accumulates for
// ~10 seconds (rounded to another zero crossing on V) and continues
// forever.  All zero crossing detection is done on the raw readings
// minus the nominal mid-rail, so won't be perfect, but should be better
// than jumping in/out at the peak.
//
void process_VI_pair (uint16_t voltage, uint16_t current, int channel) {
  static power_stats_t power_stats[MAX_CHANNELS];
  int signed_volt, signed_curr;
  power_stats_t* stats_p = &power_stats[channel];

#ifdef DUMPING
  if ((channel == 7) && dump_index[0] < DUMP_MAX) {
    dump[0][dump_index[0]++] = voltage;
    dump[1][dump_index[1]++] = current;
  }
#endif
  
  if (current == MAX_ADC_READING)                 // Make a note if we've clipped
    stats_p->clipped = true;

  signed_volt = voltage - MID_ADC_READING;        // Remove the nominal mid-rail
  signed_curr = current - MID_ADC_READING;

  //
  // If it's the very first time through, we don't have a useful
  // last_v to check for a zero crossing, so use this first one
  // to prime last_v but otherwise ignore it.
  //
  if (stats_p->state == INIT) {
    stats_p->last_v = signed_volt;
    stats_p->state = HUNTING_ZX_HEAD;
    return;
  }

  //
  // Are we waiting for the leading zero cross?
  //
  if (stats_p->state == HUNTING_ZX_HEAD) {
    if (stats_p->last_v * signed_volt < 0) {         // Found a zero crossing
      stats_p->state = ACCUMULATE;                   // promote the state and use this sample
      stats_p->last_v = signed_volt;                 // not really needed, but keep it consistent
    } else {
      stats_p->last_v = signed_volt;                 // Make a note for next time
      return;                                        // but otherwise ignore this sample
    }
  }
    
  //
  // Are we waiting for the trailing zero cross?
  //
  if (stats_p->state == HUNTING_ZX_TAIL) {
    if (stats_p->last_v * signed_volt < 0) {         // Found a zero crossing
      volatile power_stats_t* intvl_stats_p;
      //
      // We've got a batch worth, and we've just seen a zero crossing
      // The first sample after the zero crossing goes into the next
      // batch, so before including this one we need to flush out the accumulation.
      // This sample will then become the fist sample in the new batch.
      // Copy the accumulated stats into interval stats for process level to deal with
      // and zero out our stats ready for the next batch (including this new sample).
      //
      intvl_stats_p = &intvl_power_stats[channel];
      memcpy ((void*)intvl_stats_p, (void*)stats_p, sizeof(power_stats_t));
      intvl_stats_p->data_ready = true;
      memset((void*)stats_p, 0, sizeof(power_stats_t));
      stats_p->last_v = signed_volt;                 // not really needed, but keep it consistent
      stats_p->state = ACCUMULATE;                   // promote the state and...
                                                     // use this sample by not returning
    } else {
      stats_p->last_v = signed_volt;                 // Make a note for next time and....
                                                     // use this sample by not returning
    }
  }

  //
  // See if we've got a batch full.  Once we have,
  // we'll keep going until the next zero crossing.
  //
  if (stats_p->count >= SAMPLES_PER_BATCH) {             // Got almost 10 seconds worth?
    stats_p->last_v = signed_volt;                       // Make a note for next time and....
    stats_p->state = HUNTING_ZX_TAIL;                    // Start looking for a trailing ZX and...
                                                         // use this sample by not returning
  }

  //
  // If we get to here, the state machine has decided this sample pair should be included
  // in the stats.  It bails early when it's decided we want to ignore this sample.
  // In the interest of continuous sampling, that only ever happens when we're looking
  // for the very first zero crossing.  Almost all samples make it through to here.
  //
  stats_p->sigma_power += (signed_volt * signed_curr);
  stats_p->count++;
  stats_p->sigma_i += signed_curr;
  stats_p->sigma_i_sq += (signed_curr * signed_curr);
  stats_p->sigma_v += signed_volt;
  stats_p->sigma_v_sq += (signed_volt * signed_volt);
}

static void process_power_channel (int chan) {
  power_stats_t local_stats;
  int Vmean, Imean;
  int count;
  double Vrms, Irms, Preal, Papp, PF;
  
  //
  // Copy them to a local stack copy that is completely non-volatile.  Not essential
  // as these aren't going to get overwritten for ages, but removing the volatility
  // means the compiler can make optimisations it might not otherwise.
  //
  memcpy(&local_stats, (void*)&intvl_power_stats[chan], sizeof(power_stats_t));
  intvl_power_stats[chan].data_ready = false;                 // flag it as done
  count = local_stats.count;
  
  //
  // The nominal mid-rail voltage was removed above in process_VI_pair(), here
  // we calculate what's left.
  //
  if (local_stats.sigma_v > 0)
    Vmean = (local_stats.sigma_v + count/2)/count;
  else
    Vmean = (local_stats.sigma_v - count/2)/count;
  
  if (local_stats.sigma_i > 0)
    Imean = (local_stats.sigma_i + count/2)/count;
  else
    Imean = (local_stats.sigma_i - count/2)/count;
  
  //
  // And remove its RMS from the accumulated RMS.  If the mid-rail is not stable
  // and the signal is hugging the mid-rail,
  // this subtraction can send things negative, so we nip that in the bud with 0
  // rather than generate a nan.
  //
  local_stats.sigma_v_sq /= count;
  local_stats.sigma_i_sq /= count;
  local_stats.sigma_v_sq -= (Vmean * Vmean);
  local_stats.sigma_i_sq -= (Imean * Imean);
  if (local_stats.sigma_v_sq < 0)
    local_stats.sigma_v_sq = 0;
  if (local_stats.sigma_i_sq < 0)
    local_stats.sigma_i_sq = 0;
  
  //
  // Calculate the RMS values and apparent power.
  //
  Vrms = sqrt((double)local_stats.sigma_v_sq);
  Irms = sqrt((double)local_stats.sigma_i_sq);
  Papp = Vrms * Irms;
  
  //
  // Remove the offset power from the accumulated real power and
  // calculate the power factor.
  //
  if (local_stats.sigma_power > 0)
    Preal = (double)((local_stats.sigma_power + count/2)/count - (Vmean * Imean));
  else
    Preal = (double)((local_stats.sigma_power - count/2)/count - (Vmean * Imean));
  
  if (Papp != 0)
    PF = Preal / Papp;
  else PF = 0;
  
  //
  // Dump it out on the console.  If your %f's come out as blanks you need
  // to add "-u _printf_float" to your link command.  See project_name.mak
  //
  snprintf(log_buffer, sizeof(log_buffer),
	   "%2d%c Vrms: %6.2f, Irms: %5.2f, Papp: %7.2f, Preal: %7.2f, PF: %.3f, Count:%d\n",
	   chan, local_stats.clipped?'>':':', Vrms*VCAL[chan], Irms*ICAL[chan],
	   Papp*VCAL[chan]*ICAL[chan], Preal*VCAL[chan]*ICAL[chan], PF, count);
  debug_printf(log_buffer);
}
//
// Called often from the infinite loop in main().  Check to see if there are new interval
// stats we haven't processed yet, and if so, process them and flag them as processed.
//
void process_power_data () {


#ifdef ENERGY_MONITORING
  //
  // If any of them are not ready, come back later.  This ensures they'll always
  // come out in 0..3 order.
  //
  for (int chan=3; chan<6; chan++)
    if (!intvl_power_stats[chan].data_ready)
      return;
  if (!intvl_power_stats[12].data_ready)
    return;
    
  for (int chan=3; chan<6; chan++)
    process_power_channel(chan);
  process_power_channel(12);
  
#endif

#ifdef DUMPING
  if ((dump_index[0] == DUMP_MAX) && (dump_index[1] == DUMP_MAX)) {
    for (int i=0; i<DUMP_MAX; i++) {
      snprintf(log_buffer, sizeof(log_buffer),
	       "%d, %d, %d\n", i, dump[0][i], dump[1][i]);
      debug_printf(log_buffer);
    }
    dump_index[0] = dump_index[1] = DUMP_MAX+1;
  }
#endif
  
}

void init_power (void) {

  start_ADCs(ADC_LAG);                 // start ADC with x usec lag
}
