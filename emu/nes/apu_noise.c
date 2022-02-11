#include <stdint.h>
#include <stdbool.h>

#include <nes/apu_channels.h>

static const uint16_t noise_periods[16] = {
	4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068,
};

static inline uint16_t sr_bit(const apu_channel_noise_t *ch, uint8_t index) {
	return (ch->shift_reg >> index) & 1;
}

inline bool apu_noise_is_active(const apu_channel_noise_t *ch) {
	return ch->length_counter > 0;
}

inline void apu_noise_deactivate(apu_channel_noise_t *ch) {
	ch->length_counter = 0;
}

inline void apu_noise_set_lc(apu_channel_noise_t *ch, uint8_t val) {
	ch->length_counter = val;
	ch->envelope.start_flag = true;
}

void apu_noise_quarter_frame(apu_channel_noise_t *ch, const apu_reg_noise_t *reg) {
	apu_envelope_quarter_frame(&ch->envelope, reg->lch, reg->volume);
}

void apu_noise_half_frame(apu_channel_noise_t *ch, apu_reg_noise_t *reg) {
	if (!reg->lch && ch->length_counter > 0) {
		ch->length_counter--;
	}
}

void apu_noise_tick(apu_channel_noise_t *ch, const apu_reg_noise_t *reg) {
	if (ch->timer > 0) {
		ch->timer--;
		return;
	}
	ch->timer = noise_periods[reg->period];
	uint16_t feedback = sr_bit(ch, 0) ^ sr_bit(ch, reg->mode ? 6 : 1);
	ch->shift_reg = (ch->shift_reg >> 1) | (feedback << 14);
}

uint8_t apu_noise_sample(const apu_channel_noise_t *ch, const apu_reg_noise_t *reg) {
	bool active = true;
	active &= ch->length_counter > 0;
	active &= sr_bit(ch, 0) != 0;
	return apu_envelope_output(&ch->envelope, active, reg->lch, reg->volume);
}

