#include <stdint.h>
#include <stdbool.h>

#include <nes/apu_channels.h>
#include <membus.h>

static const uint16_t dmc_rates[16] = {
	428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54,
};

uint8_t read_byte(apu_dmc_mem_reader_t *mr, const apu_reg_dmc_t *reg) {
	if (mr->bytes_remaining == 0) {
		return 0;
	}

	// TODO: "stall the CPU"
	uint8_t val = membus_read(mr->bus, mr->addr);
	mr->addr = (mr->addr + 1) | 0x8000;

	if (--mr->bytes_remaining == 0) {
		if (reg->repeat) {
			mr->addr = mr->sample_start;
			mr->bytes_remaining = mr->sample_len;
		} else if (reg->irq) {
			mr->irq_flag = true;
		}
	}

	return val;
}

void apu_dmc_restart(apu_channel_dmc_t *ch) {
	if (ch->mem_reader.bytes_remaining == 0) {
		ch->mem_reader.addr = ch->mem_reader.sample_start;
		ch->mem_reader.bytes_remaining = ch->mem_reader.sample_len;
	}
}

inline bool apu_dmc_is_active(const apu_channel_dmc_t *ch) {
	return ch->mem_reader.bytes_remaining > 0;
}

void apu_dmc_tick(apu_channel_dmc_t *ch, const apu_reg_dmc_t *reg) {
	if (ch->timer > 0) {
		ch->timer--;
		return;
	}

	ch->timer = dmc_rates[reg->freq_idx] / 2;
	
	if (!ch->output_unit.silence) {
		uint8_t *v = &ch->output_unit.output_level;
		if ((ch->output_unit.shift_reg & 1) != 0) {
			if (*v <= 125) {
				*v += 2;
			}
		} else {
			if (*v >= 2) {
				*v -= 2;
			}
		}
	}

	ch->output_unit.shift_reg >>= 1;

	if (ch->output_unit.bits_remaining > 0) {
		ch->output_unit.bits_remaining--;
		return;
	}

	ch->output_unit.shift_reg = ch->sample_buffer;
	ch->output_unit.bits_remaining = 8;
	ch->sample_buffer = read_byte(&ch->mem_reader, reg);
	ch->output_unit.silence = ch->mem_reader.bytes_remaining == 0;
}

inline uint8_t apu_dmc_sample(const apu_channel_dmc_t *ch, const apu_reg_dmc_t *reg) {
	return ch->output_unit.output_level;
}
