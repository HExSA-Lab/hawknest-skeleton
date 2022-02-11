#include <string.h>

#include <nes/apu_channels.h>

static const uint8_t pulse_sequences[4][8] = {
	{0, 0, 0, 0, 0, 0, 0, 1},
	{0, 0, 0, 0, 0, 0, 1, 1},
	{0, 0, 0, 0, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 0, 0},
};

static uint16_t target_period(const apu_channel_pulse_t *ch, const apu_reg_pulse_t *reg) {
	uint16_t raw_period = reg->timer.timer;
	uint16_t change_amount = raw_period >> reg->sweep.shift_count;
	if (reg->sweep.negative) {
		if (ch->is_pulse2) {
			return raw_period - change_amount - 1;
		} else {
			return raw_period - change_amount;
		}
	} else {
		return raw_period + change_amount;
	}
}

static void update_period(apu_channel_pulse_t *ch, apu_reg_pulse_t *reg) {
	uint16_t period = target_period(ch, reg);
	if (reg->sweep.enabled && ch->sweep_counter == 0 && period <= 0x07FF) {
		reg->timer.timer = period;
	}
	if (ch->sweep_counter == 0 || ch->reload_sweep) {
		ch->reload_sweep = false;
		ch->sweep_counter = reg->sweep.period;
	} else {
		ch->sweep_counter--;
	}
}

inline void apu_pulse_reload_sweep(apu_channel_pulse_t *ch) {
	ch->reload_sweep = true;
}

inline bool apu_pulse_is_active(const apu_channel_pulse_t *ch) {
	return ch->length_counter > 0;
}

inline void apu_pulse_deactivate(apu_channel_pulse_t *ch) {
	ch->length_counter = 0;
}

void apu_pulse_set_lc(apu_channel_pulse_t *ch, uint8_t val) {
	ch->length_counter = val;
	ch->sequence_pos = 0;
	ch->envelope.start_flag = true;
}

void apu_pulse_quarter_frame(apu_channel_pulse_t *ch, const apu_reg_pulse_t *reg) {
	apu_envelope_quarter_frame(&ch->envelope, reg->volume.lch, reg->volume.level);
}

void apu_pulse_half_frame(apu_channel_pulse_t *ch, apu_reg_pulse_t *reg) {
	if (ch->length_counter > 0 && !reg->volume.lch) {
		ch->length_counter -= 1;
	}
	update_period(ch, reg);
}

void apu_pulse_tick(apu_channel_pulse_t *ch, const apu_reg_pulse_t *reg) {
	uint16_t period = reg->timer.timer;
	if (ch->timer == 0) {
		ch->timer = period;
		if (ch->sequence_pos > 0) {
			ch->sequence_pos--;
		} else {
			ch->sequence_pos = 7;
		}
	} else {
		ch->timer--;
	}
	// this function runs at >10 MHz, but... it's 8 bytes
	memcpy(ch->sequence, pulse_sequences[reg->volume.duty], 8);
}

uint8_t apu_pulse_sample(const apu_channel_pulse_t *ch, const apu_reg_pulse_t *reg) {
	bool active = true;
	active &= ch->length_counter != 0;
	active &= reg->timer.timer >= 8;
	active &= target_period(ch, reg) <= 0x07FF;
	active &= ch->sequence[ch->sequence_pos] != 0;
	return apu_envelope_output(&ch->envelope, active, reg->volume.constant, reg->volume.level);
}
