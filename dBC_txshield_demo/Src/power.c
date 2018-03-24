
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "main.h"
#include "usart.h"
#include "power.h"

//
// Volatile power stats are where the stats get accumulated
// on each interrupt, i.e. each call to process_VI_pair()
//
static volatile power_stats_t vol_power_stats[MAX_CHANNELS];

//
// Interval power stats are were the stats get moved to when
// process_VI_pair() has decided it's accumulated enough
// (147,059 currently... about 10 seconds worth).
// This data structure is still volatile, just not as volatile.
// process_power_stats() works on these intervals stats once they're
// ready.
//
static volatile power_stats_t intvl_power_stats[MAX_CHANNELS];


//
// Called from the ADC/DMA interrupt when a new V,I pair has arrived.
// Basically does the accumulation maths on the new pair.  Currently
// no zero crossing hunting is done, should it be?
//
void process_VI_pair (uint16_t voltage, uint16_t current, int channel) {
  int signed_volt, signed_curr;
  volatile power_stats_t* stats_p;

  stats_p = &vol_power_stats[channel];

  signed_volt = voltage - MID_ADC_READING;        // Remove the nominal mid-rail
  signed_curr = current - MID_ADC_READING;

  if (current == MAX_ADC_READING)                 // Make a note if we've clipped
    stats_p->clipped = true;

  //
  // Add the new pair into the stats
  //
  stats_p->sigma_power += (float)(signed_volt * signed_curr);
  stats_p->count++;
  stats_p->sigma_i += signed_curr;
  stats_p->sigma_i_sq += (float)(signed_curr * signed_curr);
  stats_p->sigma_v += signed_volt;
  stats_p->sigma_v_sq += (float)(signed_volt * signed_volt);

  if (stats_p->count >= SAMPLES_PER_BATCH) {             // Got 10 seconds worth?
    volatile power_stats_t* intvl_stats_p;

    //
    // Copy them into interval stats for process level to deal with
    // and zero out our stats ready for the next batch
    //
    intvl_stats_p = &intvl_power_stats[channel];
    memcpy ((void*)intvl_stats_p, (void*)stats_p, sizeof(power_stats_t));
    intvl_stats_p->data_ready = true;
    memset((void*)stats_p, 0, sizeof(power_stats_t));
  }
}

//
// Called often from the infinite loop in main().  Check to see if there are new interval
// stats we haven't processed yet, and if so, process them and flag them as processed.
//
void process_power_data () {
  //
  // Don't be alarmed when these come out in random order.  The timing of this
  // for loop is asynchronous to the timing of the incoming data, so it's possible
  // that at the time we check [0].data_ready there is none but by the time we get
  // to [1].data_ready there is.  But you should always get them in batches of 4,
  // every 10 seconds, so do be alarmed if that's not happening.
  //
  for (int chan=0; chan<MAX_CHANNELS; chan++)
    if (intvl_power_stats[chan].data_ready) {
      power_stats_t local_stats;
      int Vmean, Imean;
      int count;
      float Vrms, Irms, Preal, Papp, PF;

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
      Vmean = (local_stats.sigma_v + count/2)/count;
      Imean = (local_stats.sigma_i + count/2)/count;

      //
      // And remove its RMS from the accumulated RMS
      //
      local_stats.sigma_v_sq -= (float)(Vmean * Vmean);
      local_stats.sigma_i_sq -= (float)(Imean * Imean);

      //
      // Calculate the RMS values and apparent power.
      //
      Vrms = sqrt(local_stats.sigma_v_sq/(float)count);
      Irms = sqrt(local_stats.sigma_i_sq/(float)count);
      Papp = Vrms * Irms;

      //
      // Remove the offset power from the accumulated real power and
      // calculate the power factor.
      //
      Preal = local_stats.sigma_power / (float)count - (float)(Vmean * Imean);
      PF = Preal / Papp;
      
      //
      // Dump it out on the console.  If your %f's come out as blanks you need
      // to add "-u _printf_float" to your link command.  See project_name.mak
      //
      snprintf(log_buffer, sizeof(log_buffer),
	       "%d%c Vrms: %.2f, Irms: %.2f, Papp: %.2f, Preal: %.2f, PF: %.3f\n",
	       chan, local_stats.clipped?'>':':', Vrms, Irms, Papp, Preal, PF);
      debug_printf(log_buffer);
    }
  
}
