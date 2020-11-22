#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdbool.h>
#include "power.h"
#include "phase.h"
#include "adc.h"
#include "main.h"
#include "gpio.h"


//--------------------------------
// ADC SETTINGS / CALIBRATION
//--------------------------------
const double VOLTS_PER_DIV = (2.048 / 4096.0); // REF191
// const double VOLTS_PER_DIV = (3.3 / 4096.0); // REF196

//--------------------------------
// VOLTAGE CALIBRATION
//--------------------------------
// const double VCAL = 225.83; // default ideal power UK, for STM32 HW v1.0 (REF196?)
const double VCAL = 290.64; // default ideal power UK, for STM32 HW v1.0 (REF191???)

//--------------------------------
// AMPERAGE CALIBRATION
//--------------------------------
const double ICAL = (100/0.05)/6.8; // (CT rated input / rated output) / burden value.






double mains_frequency;
double Ws_accumulator[CTn] = {0}; // energy accumulator per CT channel.

bool readings_ready = false;
bool readings_requested = false;


channel_t channels[CTn] = {0};
channel_t channels_ready[CTn] = {0};
bool channel_rdy_bools[CTn] = {0};


void calcPower (int ch)
{
  channel_t *chn = &channels_ready[ch];

  double Vmean = (double)chn->sum_V / (double)chn->samplecount;
  double Imean = (double)chn->sum_I / (double)chn->samplecount;

  double f32sum_V_sq_avg = (double)chn->sum_V_sq / (double)chn->samplecount;
  f32sum_V_sq_avg -= (Vmean * Vmean); // offset removal

  double f32sum_I_sq_avg = (double)chn->sum_I_sq / (double)chn->samplecount;
  f32sum_I_sq_avg -= (Imean * Imean); // offset removal

  // assuming result of offset removal always positive.
  // small chance a negative result would cause a nan at sqrt.

  Vrms = V_SCALE * sqrt(f32sum_V_sq_avg);
  Irms = I_SCALE * sqrt(f32sum_I_sq_avg);

  double f32_sum_P_avg = (double)chn->sum_P / (double)chn->samplecount;
  double mean_P = f32_sum_P_avg - (Vmean * Imean); // offset removal
  
  realPower = V_SCALE * I_SCALE * mean_P;
  apparentPower = Vrms * Irms;

  // calculate PF, is it necessary to prevent dividing by zero error?
  if (apparentPower != 0) { powerFactor = realPower / apparentPower; }
  else powerFactor = 0;

  // if (realPower < 1.0) {
  //   realPower = 0.0;
  // }

  
}

void calcEnergy (int ch, double power, int interval) {
    Ws_accumulator[ch] += (power * (interval / 1000.0));
}



//------------------------------------------
// Process Half-Buffer into Accumulators.
//------------------------------------------
void process_frame (uint16_t offset)
{
  /********** 
  // debugging
  if (!hia) {
    debug_printf("Hello! First process_frame!\r\n");
    hia = 1;
  }
  **********/
  if(ledBlink){HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET); } // blink the led
  
  while (!check_dma_index_for_phase_correction(offset));

  int16_t sample_V, sample_I, signed_V, signed_I;
  
   // to hunt one adc_buffer_index per readings_ready, because it's only in readings_ready where PF is calc'd.
  /*
  sprintf(log_buffer, "adc_buff_half_size:%d\r\n", adc_buff_half_size);
  debug_printf(log_buffer);
  
  */

  for (uint16_t i = 0; i < adc_buff_half_size; i += CTn) // CTn = CT channel quanity.
  {
    
    /* // debug buffer
    sprintf(log_buffer, "signed_v:%d\r\n", signed_V);
    debug_printf(log_buffer);
    
    */

    //----------------------------------------
    // Power
    //----------------------------------------
    // Cycle through channels, accumulating
    for (int ch = 0; ch < CTn; ch++)
    {
      channel_t *channel = &channels[ch];
      //----------------------------------------
      // Voltage
      //----------------------------------------
      // phase correction could happen here to have higher resolution. 
      // to shift voltage gives finer grained phase resolution in single-phase mode
      // because the voltage channel is used by all CTs.
      int16_t sample_V_index = offset + i + ch + phase_corrections[ch];
      // check for buffer overflow
      if (sample_V_index >= adc_buff_size) sample_V_index -= adc_buff_size;
      else if (sample_V_index < 0) sample_V_index += adc_buff_size;
      
      sample_V = adc1_dma_buff[sample_V_index];
      //if (sample_V == 4095) Vclipped = true; // unlikely
      signed_V = sample_V - MID_ADC_READING;
      channel->sum_V += signed_V;
      channel->sum_V_sq += signed_V * signed_V; // for Vrms
      //----------------------------------------
      // Current
      //----------------------------------------
      sample_I = adc3_dma_buff[offset + i + ch];
       // debug the buffer 
       
      // if (ch == 4) {
      //   char valuebuff[10];
      //   while (!usart_tx_ready); // force wait while usart Tx finishes.
      //   sprintf(valuebuff, "%d\r\n", sample_I);
      //   debug_printf(valuebuff);
      // }
      
      if (sample_I == 4095) channel->Iclipped = true; // much more likely, useful safety information.
      signed_I = sample_I - MID_ADC_READING; // mid-rail removal possible through ADC4, option for future perhaps.
      channel->sum_I += signed_I; // 
      channel->sum_I_sq += signed_I * signed_I; // for Irms
      //----------------------------------------
      // Power, instantaneous.
      //----------------------------------------
      channel->sum_P += signed_V * signed_I;
      
      channel->samplecount++; // number of adc samples.
      
      //----------------------------------------
      // Upwards-zero-crossing detection, whole AC cycles.
      channel->last_positive_V = channel->positive_V; // retrieve the previous value.
      if (signed_V >= 0) { channel->positive_V = true; } // changed > to >= . not important as MID_ADC_READING 2048 not accurate anyway.
      else { channel->positive_V = false; }
      //--------------------------------------------------
      //--------------------------------------------------
      if (dontGiveAMonkeys || (!channel->last_positive_V && channel->positive_V)) { // looking out for a upwards-zero crossing.
        channel->cycles++; // rather than count cycles for readings_ready, better to have the main loop ask for them.
        // debug cycle count 
        /*
        sprintf(log_buffer, "cycles%d:%ld\r\n", ch, channel->cycles);
        debug_printf(log_buffer); 
        */
        //----------------------------------------
        if (readings_requested && !channel_rdy_bools[ch]) { // if readings are needed by the main loop and channel is not copied/ready.
          channel_t *channel_ready = &channels_ready[ch];
          // copy accumulators for use in main loop.
          memcpy((void*)channel_ready, (void*)channel, sizeof(channel_t));
          // reset accumulators to zero.
          memset((void*)channel, 0, sizeof(channel_t));
          // follow through the state of positive_v so no extra AC cycle is erroneously counted.
          channel->positive_V = true;
          // set 'channel ready' for this channel.
          channel_rdy_bools[ch] = true;
          // are all the channels ready?
          int chn_ready_count = 0;
          for (int j = 0; j < CTn; j++) {
            if (channel_rdy_bools[j]) chn_ready_count++;
          }
          if (chn_ready_count == CTn) { readings_ready = true; readings_requested = false; }
          
          // test waveform sync. printed result should go from 1 to 9.
          /*
          sprintf(log_buffer, "channelsRdy:%d\r\n", chn_ready_count);
          debug_printf(log_buffer); 
          */
        }
      }
      //-------------------------------------------------- 
      //-------------------------------------------------- end zero-crossing.
    } // end per channel routine
  } // end buffer routine
  if(ledBlink) { HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);}
  
  // while(1) {
  //   __NOP();
  // }
} // end process_frame().








#ifdef __cplusplus
 extern "C" {
#endif


#ifdef __cplusplus
}
#endif
