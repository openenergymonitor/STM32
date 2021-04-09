v1.2 changes
- diode placed between backup battery regulator and vbat, stopping reverse current from vbat to gnd through regulator.
- rotated the boot and reset headers.
- ac-ac input voltage divider changed to 140k/10k divider. new output impedance 9.33kOhms
- ac-ac input pins swapped. the new opamp based ct inputs causes an inversion of the signal, thus the ac-ac pins have been swapped to keep it in phase.

v1.0 & v1.1 changes

- removed usart2 header. all debugging and uart firmware upload through usart1 at corner of board.
- changed protection diode method on BIAS
- changed expansion header to through-hole type.
- 3v3 regulator for vbat, keeping battery charged.
- removed various components for testing.
- opamp reconfigured, footprint included for testing another opamp.
- various small changes to enclosure design.
- burden value at 6r8 and is now a single 0805 size.
- opamp bias input traces fixed.
- removed prototype solder jumpers
- rPi connections normally closed.
- 3-phase lmv824 op-amps typology implemented.
- ref19x for VREF now 2.048V and supplied by analog 3v3 instead of 5v.
- lmv824 3-phase feedback resistors changed to 330R.
- universal input CT channels!
- since bias input no longer loaded, opamp compensation done away with.
- SMA connector change, though hole instead of edge mount, for manufacturability.
- input protection changes. all are SOD-523 diode pairs. CT channels protected via bias' diodes.
- boot mode button smt footprint for easier dev.