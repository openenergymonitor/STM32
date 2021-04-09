#include <string.h>
#include <stdbool.h>
#include <stdbool.h>
#include "main.h"
#include "phase.h"
#include "power.h"
#include "adc.h"
#include "usart.h"


const double adc_conversion_time = (614.0*(1.0/(72000000.0/4))); // time in seconds for an ADC conversion, for estimating mains AC frequency.
// const double adc_conversion_time = (194.0*(1.0/(72000000.0/4))); // time in seconds for an ADC conversion, for estimating mains AC frequency.
// const double adc_conversion_time = (74.0*(1.0/(72000000.0/4))); // seems to be the fastest we can sample.
// const double adc_conversion_time = (32.0*(1.0/(72000000.0/4))); // this speed causes buffer overruns.




//--------------------------
// PHASE CALIBRATION
//--------------------------
// Finite Impulse Response (FIR) filter-like Power Factor correction.
// This phase correction can be applied to individual CTs.
// This phase-correction works by selecting the relevant sample from the ADC DMA buffer.
// For example ADC1 has buffer 1, ADC2 has buffer 2. The VT is on buffer 1, the CT is on buffer 2.
// We can choose to pair the relevant sample from buffers one and two, to adjust for time differences (phase shift).
// Each sample is separated at a single conversion time, 10.7usecs, for example, or tripple that for 3-phase.
int hunt_PF[CTn] = {0}; // this is a per-channel state trigger. The array index is the CT channel.
// when above array value {0} equals:
// 0 = no power factor hunting, default at program start.
// 1 = power factor hunt start. This can be triggered from a serial command.
// 2 = power factor hunt to the right. (increase VT lead, automatic).
// 3 = power factor hunt to the left. (increase VT lag, automatic).
// 4 = hunt finishing, finding mode average (automatic).
// 5 = hunt complete.

// This stores the value of buffer indexes to shift by, for each channel.
int phase_corrections[CTn] = {0};   // store of phase corrections per channel.

// Below are misc variables to make PF hunting work.
uint8_t phHunt_direction_changes = 0; // we're aiming for phase_correction_iteration_target.
const int phHunt_direction_change_target = 5; // how many times will the phase hunt change direction?
double last_powerFactor[CTn]; // PF from previous readings.
double powerFactor_now[CTn];  // PF from most recent readings.
uint16_t highest_phase_correction = 0;





//------------------------------------------
// FIR-like filter for phase correction.
//------------------------------------------
void set_highest_phase_correction(void) { // could be done after each correction routine.
  highest_phase_correction = 0; // reset. 
  for (int i = 0; i < CTn; i++) {
    if (phase_corrections[i] > highest_phase_correction) highest_phase_correction = phase_corrections[i];
  }
}


//-----------------------------------------------
// Check the DMA buffer for a 'future' sample. 
//-----------------------------------------------
bool check_dma_index_for_phase_correction(uint16_t offset) {
  // HAL_DELAY(1); // test
  uint16_t dma_counter_now = hdma_adc4.Instance->CNDTR;
  //sprintf(log_buffer, "dma_counter_now:%d, offset:%d, highest_correction:%d\r\n", dma_counter_now, offset, highest_phase_correction);
  //debug_printf(log_buffer); log_buffer[0] = 0;
  if (offset == 0) {
    if (adc_buff_half_size - dma_counter_now > highest_phase_correction)
    { 
      return true; 
    }
    else 
    { 
      return false; 
    }
  }
  else if (offset == adc_buff_half_size) {
    if (adc_buff_size - dma_counter_now > highest_phase_correction) 
    {
      return true; 
    }
    else 
    { 
      return false; 
    }
  }
  return false;
}


//--------------------------------------------------------
// Mathematical function to find the mode of an array
//--------------------------------------------------------
int findmode(int a[],int n) { // https://www.tutorialspoint.com/learn_c_by_examples/mode_program_in_c.htm
   modeSearchFailed = false;
   int maxValue = 0, maxCount = 0, i, j;
   for (i = 0; i < n; ++i) {
      int count = 0;
      for (j = 0; j < n; ++j) {
         if (a[j] == a[i])
         ++count;
      }
      if (count > maxCount) {
         maxCount = count;
         maxValue = a[i];
      }
   }
   if (maxCount < 3) {debug_printf("No confident value found.\r\n"); modeSearchFailed = true; }
   return maxValue;
}

//----------------------------------------------------------------------------------------------------------------
// This is a phase correction per CT channel method based on the DMA buffer index selection (CT vs VT DMA buffer).
//----------------------------------------------------------------------------------------------------------------
int pf_mode_array[5]; // stores the phase_corrections value at the point of switching direction.
int *pf_ptr = pf_mode_array;
int phase_corrections_store_previous_value;
void pfHunt(int ch) {
  // while (!usart_tx_ready); // force wait while usart Tx finishes.
  if (hunt_PF[ch] == 0 || hunt_PF[ch] == 5) { 
    return;
  }
  else if (hunt_PF[ch] == 1) { // start hunting one direction
    phase_corrections_store_previous_value = phase_corrections[ch];
    phase_corrections[ch]++; hunt_PF[ch]++;
    // print phase correction.
    sprintf(log_buffer, "PFstart++ \r\nSetting phase_corrections[%d] to %d\r\n", ch, phase_corrections[ch]);
    //debug_printf(log_buffer); 
  }
  else if (hunt_PF[ch] == 2) { // continue this direction unless...
    sprintf(log_buffer, "PF_now:%.6lf | last_PF:%.6lf | phase_corrections[] for CT%d was at %d, equal to %.2lf degrees phase shift.\r\n",
     powerFactor_now[ch], last_powerFactor[ch], ch+1, phase_corrections[ch], (360.0*((adc_conversion_time*phase_corrections[ch])/(1/mains_frequency))));
    debug_printf(log_buffer); 
    if (powerFactor_now[ch] > last_powerFactor[ch]) phase_corrections[ch]++; // keep going forwards into the buffer
    else { hunt_PF[ch]++; phHunt_direction_changes++; phase_corrections[ch]--; *pf_ptr = phase_corrections[ch]; pf_ptr++; } // switch direction and start going backwards into the buffer
    // print phase correction.
    sprintf(log_buffer, "PF++ | Setting phase_corrections[] for CT%d to %d. Switched direction %d times.\r\n", ch+1, phase_corrections[ch], phHunt_direction_changes);
    //debug_printf(log_buffer); 
    if (phHunt_direction_changes == phHunt_direction_change_target) hunt_PF[ch] = 4;
  }
  else if (hunt_PF[ch] == 3) { // hunt the other direction until...
    sprintf(log_buffer, "PF_now:%.6lf | last_PF:%.6lf | phase_corrections[%d] was at %d, equal to %.2lf degrees phase shift.\r\n", powerFactor_now[ch], last_powerFactor[ch], ch, phase_corrections[ch], (360.0*((adc_conversion_time*phase_corrections[ch])/(1/mains_frequency)))); debug_printf(log_buffer);     
    if (powerFactor_now[ch] > last_powerFactor[ch]) phase_corrections[ch]--; // keep going backwards into the buffer
    else { hunt_PF[ch]--; phHunt_direction_changes++; phase_corrections[ch]++; *pf_ptr = phase_corrections[ch]; pf_ptr++; } // switch direction and go forwards again into the buffer
    // print phase correction.
    sprintf(log_buffer, "PF-- | Setting phase_corrections[] for CT%d to %d. Switched direction %d times.\r\n", ch+1, phase_corrections[ch], phHunt_direction_changes);
    //debug_printf(log_buffer); 
    if (phHunt_direction_changes == phHunt_direction_change_target) hunt_PF[ch] = 4;
  }
  else if (hunt_PF[ch] == 4) {
    // finish up...
    for (int i = 0; i < phHunt_direction_change_target; i++) { sprintf(log_buffer, "pf_mode_array[%d] value was %d\r\n", i, pf_mode_array[i]); debug_printf(log_buffer); }
  
    sprintf(log_buffer, "mode:%d\r\n", findmode(pf_mode_array, phHunt_direction_change_target)); 
    debug_printf(log_buffer); 

    phase_corrections[ch] = findmode(pf_mode_array, phHunt_direction_change_target); // the result is set in phase_corrections[] for this channel now.
    if (modeSearchFailed) {
      sprintf(log_buffer, "huntPF FAILED – phase_corrections[] for CT%d set to original value %d.\r\n", ch+1, phase_corrections_store_previous_value); 
      phase_corrections[ch] = phase_corrections_store_previous_value;
    }
    else {
      sprintf(log_buffer, "Maximum PF found! phase_correction[%d] is at %d, equal to %.2lf degrees phase shift.\r\n", ch, phase_corrections[ch], (360.0*((adc_conversion_time*phase_corrections[ch])/(1/mains_frequency))));
      set_highest_phase_correction(); // update the highest value for 'value ready?' buffer checking.
    }
    phHunt_direction_changes = 0;
    *pf_ptr = 0;
    hunt_PF[ch] = 5;
  }
}





#ifdef __cplusplus
 extern "C" {
#endif

#ifdef __cplusplus
}
#endif
