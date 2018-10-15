//
// Change these all to 1.0 if you want raw A/D units
//

#define CALIBRATED 1

#ifdef CALIBRATED
const float VCAL[MAX_CHANNELS] = {
  1.0, 1.0, 1.0,
  0.24142421201, 0.24142421201, 0.24142421201, 
  1.0, 1.0, 1.0,
  1.0, 1.0,
  1.0, 0.24142421201, 
  1.0, 1.0
};

const float ICAL[MAX_CHANNELS] = {
  1.0, 1.0, 1.0,
  0.04943131519, 0.04894363325, 0.04890794487,
  1.0, 1.0, 1.0,
  1.0, 1.0,
  1.0, 0.01963339104,
  1.0, 1.0
};
#endif

#ifdef RAW
const float VCAL[MAX_CHANNELS] = {
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 
  1.0, 1.0, 1.0,
  1.0, 1.0,
  1.0, 1.0, 
  1.0, 1.0
};

const float ICAL[MAX_CHANNELS] = {
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0,
  1.0, 1.0,
  1.0, 1.0
};
#endif


const int ADC_LAG = 269;             // ~4.8 degrees
// const int ADC_LAG = 0;             // 0 degrees
