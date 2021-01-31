void rfm_init(uint8_t RF_freq);
bool rfm_send(uint8_t *data, uint8_t size, uint8_t group, uint8_t node, const int threshold, uint8_t timeout);
void rfm_sleep(void);
void writeReg(uint8_t addr, uint8_t value);
uint8_t readReg(uint8_t addr);
void select();
void unselect();
void rfm_rst(void);