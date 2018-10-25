## Prototype 1: Voltage follower and anti-alias filter

![prototype1.JPG](../images/prototype1.JPG)

[Emon1CT Firmware](https://github.com/TrystanLea/STM32Dev/tree/master/Emon1CT):

- Voltage follower on PA6 & PA7 for bias buffer (note: HAL_OPAMP_Start(&hopamp2); required)
- Implements @Robert.Wall's offset removal technique

[Emon3CT Firmware](https://github.com/TrystanLea/STM32Dev/tree/master/Emon3CT):

- Extension of the 1CT example to 3 CT inputs.

Anti-alias filter based on: 

![antialias.png](../images/antialias.png)

Two 1k resistors + 22pF capacitors to ground, biased with output from voltage follower.

40W incandescent lightbulb:

    Vrms    Irms    RP      AP      PF      Count
    239.86  0.184   43.8    44.1    0.995   287122
    239.85  0.183   43.8    43.9    0.998   289544
    239.94  0.184   43.9    44.1    0.995   289507
    240.07  0.183   43.9    43.9    0.999   287142
    240.12  0.184   43.9    44.1    0.996   289462
    239.98  0.184   43.9    44.1    0.995   282377
    239.76  0.184   43.8    44.0    0.995   284698
    
Electric room heater:

    229.96  7.908   1816.4  1818.5  0.999   289401
    229.84  7.901   1813.9  1816.0  0.999   291787
    229.83  7.900   1813.5  1815.6  0.999   289468
    230.05  7.907   1816.9  1819.0  0.999   289500
    230.07  7.907   1817.2  1819.2  0.999   287145
    230.05  7.906   1816.7  1818.8  0.999   291873
    
Laptop:

    239.41  0.134   17.1    32.0    0.535   287153
    239.48  0.139   17.9    33.2    0.540   284790
    239.58  0.140   18.1    33.4    0.542   291839
    239.62  0.144   18.8    34.5    0.544   287131
    239.50  0.132   17.0    31.6    0.538   289426
    239.45  0.136   17.6    32.5    0.541   291730

Readings are printed every ~2.5 seconds (125 cycles 50Hz), ~287122 VI sample pairs every 2.5s = 115 kHz

22pF filter cap on CT input:

![22pfCT.png](../images/DS/22pfCT.png)

100nF filter cap on CT input:

![100pfCT.png](../images/DS/100pfCT.png)

No filter cap on CT input:

![nocapCT.png](../images/DS/nocapCT.png)

100nF filter cap on Voltage input (22pF on other screenshots), showing to low cut off frequency:

![100pfV.png](../images/DS/100pfV.png)

