some notes concerning phase correction between VT and CT via buffer index shifting.

. The voltage channel prvides the highest resolution of samples to choose from, providing a finer grained phase correction.
34.11111 usec per adc conversion. 0.614deg phase shift (phase resolution) per conversion at 50Hz.

. The correct term for this 'index shifting for phase correciton' is a Finite Impulse Response (FIR) filter. The filter 'gain' is a property which shifts time forwards or backwards for a given sample set.

. dBC used the rising and falling edge of the ADC trigger to adjust the ADC start time, and hence adjust phase. This method is easy to implement. Already done. No serious limitations. 

. A potential limitation of the FIR method would be the possibility of overwriting a part of the ADC buffer which the FIR filter would depend, because the FIR filter would go back in time to previous samples to get the data. This is an unlikely issue.

. A limitation of dBC's method would be the fixing of the type of VT and CT used, because total-system phase shift is the sum of VT and CT phase errors, using multiple different CT would render this method inappropriate, or make unlealistic the use of different CTs for accuracy. Can a proviso be made that this method be used when all CTs are the same or have similar frequency (phase-shift) characteristics?

. I've gone from 614 cycles per ADC conversion to 181.5 + 12.5 = 194 cycles/adc_conversion. Giving a phase resolution of 0.194deg. Tested the CPU against sending a stream of JSON parser commands, seems okay. Futher testing shows just under half of the CPU is now utilised by process_frame.

. To make best use of dBC's method, there'll need to be a clear method of starting EITHER voltage or current channels first, by switching the rising/falling edge trigger. To DeInit / CHANGE_SETTINGS / Init.

. ADC trigger event: external trigger redirection might cause delay in start times of ADC1 & ADC3.
from stm32f3xx_hal_adc_ex.h :
/*!< External triggers of regular group for ADC1&ADC2, ADC3&ADC4 */
/* Note: Triggers affected to group ADC1_2 by default, redirected to group    */
/*       ADC3_4 by driver when needed.                                        */
