## STM32 ADC part 2

Starting from where we left off in the previous example with ADC1 channel 1 and 2 enabled and configured.

- Enable Continuous Conversions
- Enable DMA Continuous Requests
- End of Conversion to be End of Sequence

*Note (from @dBC):<br>
Default ADC clock setting is 72MHz (see Clock Configuration tab).<br>
Sampling Time is how long it will spend charging the S&H cap (20.8ns to 8.35us)<br>
Total conversion time for a single pin in the sequence is Sampling Time + 12.5 cycles.<br>
The selected Sampling Time depends on the source impedance of the signal.*<br>

- Add DMA channel in the DMA Settings tab.
- Select Circular mode

*Note (from @dBC):<br>
Circular tells the DMA controller to loop back to the beginning when it hits the end. We want it to increment the Memory address during the xfer, but not the Peripheral address, and we want it to do Half Word width xfers… the 12-bit conversion results are read from a 16-bit register in the ADC and the dma buffer is an array of uint16_ts.*
