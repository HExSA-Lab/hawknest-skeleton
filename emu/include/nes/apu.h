typedef struct apu apu_t;

uint8_t apu_mem_read(apu_t *apu, uint16_t addr, uint8_t *lane_mask);
void apu_mem_write(apu_t *apu, uint16_t addr, uint8_t data);
