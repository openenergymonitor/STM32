#ifndef __phase_H
#define __phase_H

#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "adc.h"


const double adc_conversion_time;

//--------------------------
// PHASE CALIBRATION
//--------------------------
int hunt_PF[CTn];
int phase_corrections[CTn];
uint8_t phHunt_direction_changes;
const int phHunt_direction_change_target;
double last_powerFactor[CTn];
double powerFactor_now[CTn];
uint16_t highest_phase_correction;

bool modeSearchFailed;


void set_highest_phase_correction(void);
bool check_dma_index_for_phase_correction(uint16_t offset);
int findmode(int a[],int n);
void pfHunt(int ch);





#ifdef __cplusplus
 extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif