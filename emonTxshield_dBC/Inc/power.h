#define MAX_ADC_READING 4095
#define MID_ADC_READING 2047
#define MAX_CHANNELS 15

#define LINE_FREQ 50                     // Hz
#define SAMPLE_PERIOD  119               // usecs - see ADC config
#define REPORTING_INTVL 10               // seconds
#define SAMPLES_PER_INTVL (REPORTING_INTVL * 1000000 / SAMPLE_PERIOD)  // 147058

#define SAMPLES_PER_HCYCLE  (1000000 / LINE_FREQ / SAMPLE_PERIOD / 2)   // 147 @ 50Hz

//
// For data every 10 seconds we'd want 10,000,000/68 = 147,058.8 samples per batch.
// But the zero crossing search runs until the next half cycle has completed, so
// we shave half a half cycle off here, to make sure we don't end up with one more
// half cycle than we expected.   147058 - 147/2 = 146985
//
#define SAMPLES_PER_BATCH   (SAMPLES_PER_INTVL - SAMPLES_PER_HCYCLE/2)


#define DUMP_MAX 600
#define DUMP_CHANS 2

typedef enum channel_state_ {
  INIT,                        // starting state guaranteed by bss zero'ing
  HUNTING_ZX_HEAD,
  ACCUMULATE,
  HUNTING_ZX_TAIL
} channel_state_t;

typedef struct power_stats_ {
  int64_t sigma_power;
  int64_t sigma_i_sq;
  int64_t sigma_v_sq;
  int count;
  int sigma_i;
  int sigma_v;
  bool clipped;
  bool data_ready;
  channel_state_t state;
  int last_v;
} power_stats_t;

extern void process_VI_pair(uint16_t voltage, uint16_t current, int channel);
extern void process_power_data(void);
extern void init_power(void);

