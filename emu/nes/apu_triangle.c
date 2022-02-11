#include <stdint.h>
#include <stdbool.h>

#include <nes/apu_channels.h>

inline bool apu_triangle_is_active(const apu_channel_triangle_t *ch) {
	return ch->length_counter > 0;
}

inline void apu_triangle_deactivate(apu_channel_triangle_t *ch) {
	ch->length_counter = 0;
}

inline void apu_triangle_set_lc(apu_channel_triangle_t *ch, uint8_t val) {
	ch->length_counter = val;
	ch->reload_linear_counter = true;
}

void apu_triangle_quarter_frame(apu_channel_triangle_t *ch, const apu_reg_triangle_t *reg) {
	if (ch->reload_linear_counter) {
		ch->linear_counter = reg->counter.reload;
	} else if (ch->linear_counter > 0) {
		ch->linear_counter--;
	}
	if (!reg->counter.control) {
		ch->reload_linear_counter = false;
	}
}

void apu_triangle_half_frame(apu_channel_triangle_t *ch, apu_reg_triangle_t *reg) {
	if (!reg->counter.control && ch->length_counter > 0) {
		ch->length_counter--;
	}
}

void apu_triangle_tick(apu_channel_triangle_t *ch, const apu_reg_triangle_t *reg) {
	if (ch->timer > 0) {
		ch->timer--;
		return;
	}
	ch->timer = reg->timer.timer + 1;
	if (ch->linear_counter > 0 && ch->length_counter > 0) {
		ch->sequence_pos += 1;
		ch->sequence_pos %= 32;
	}
}

uint8_t apu_triangle_sample(const apu_channel_triangle_t *ch, const apu_reg_triangle_t *reg) {
	if (ch->sequence_pos <= 15) {
		return 15 - ch->sequence_pos;
	} else {
		return ch->sequence_pos - 16;
	}
}
