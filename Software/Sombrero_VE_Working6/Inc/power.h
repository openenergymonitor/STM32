#include <stdio.h>
#include <string.h>

#ifndef __gpio_H
#define __gpio_H
#ifdef __cplusplus
 extern "C" {
#endif

void set_highest_phase_correction(void);
bool check_dma_index_for_phase_correction(uint16_t offset);
int findmode(int a[],int n);
void pfHunt(int ch);
void calcPower (int ch);
void process_frame (uint16_t offset);

#ifdef __cplusplus
}
#endif
#endif