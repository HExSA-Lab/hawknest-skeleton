#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <base.h>
#include <nes/apu_regs.h>

typedef struct apu_envelope {
	bool start_flag;
	uint8_t divider;
	uint8_t decay;
} apu_envelope_t;

typedef struct apu_channel_pulse {
	uint8_t sequence[8];
	uint8_t sequence_pos;
	uint16_t timer;
	uint8_t length_counter;
	uint8_t sweep_counter;
	bool reload_sweep;
	bool is_pulse2;
	apu_envelope_t envelope;
} apu_channel_pulse_t;

typedef struct apu_channel_triangle {
	uint8_t sequence_pos;
	uint16_t timer;
	uint8_t length_counter;
	uint8_t linear_counter;
	bool reload_linear_counter;
} apu_channel_triangle_t;

typedef struct apu_channel_noise {
	apu_envelope_t envelope;
	uint8_t length_counter;
	uint16_t timer;
	uint16_t shift_reg;
} apu_channel_noise_t;

typedef struct apu_dmc_mem_reader {
	struct membus * nonnull /*unowned*/ bus;
	uint16_t sample_start;
	uint16_t addr;
	uint16_t sample_len;
	uint16_t bytes_remaining;
	bool irq_flag;
} apu_dmc_mem_reader_t;

typedef struct apu_dmc_output_unit {
	uint8_t shift_reg;
	uint8_t bits_remaining;
	uint8_t output_level;
	uint8_t silence;
} apu_dmc_output_unit_t;

typedef struct apu_channel_dmc {
	apu_dmc_mem_reader_t mem_reader;
	apu_dmc_output_unit_t output_unit;
	uint8_t sample_buffer;
	uint8_t timer;
} apu_channel_dmc_t;

void apu_envelope_quarter_frame(apu_envelope_t *evl, bool loop_flag, uint8_t period);
uint8_t apu_envelope_output(const apu_envelope_t *evl, bool active, bool constant, uint8_t volume);

void apu_pulse_reload_sweep(apu_channel_pulse_t *ch);
bool apu_pulse_is_active(const apu_channel_pulse_t *ch);
void apu_pulse_deactivate(apu_channel_pulse_t *ch);
void apu_pulse_set_lc(apu_channel_pulse_t *ch, uint8_t val);
void apu_pulse_quarter_frame(apu_channel_pulse_t *ch, const apu_reg_pulse_t *reg);
void apu_pulse_half_frame(apu_channel_pulse_t *ch, apu_reg_pulse_t *reg);
void apu_pulse_tick(apu_channel_pulse_t *ch, const apu_reg_pulse_t *reg);
uint8_t apu_pulse_sample(const apu_channel_pulse_t *ch, const apu_reg_pulse_t *reg);

bool apu_triangle_is_active(const apu_channel_triangle_t *ch);
void apu_triangle_deactivate(apu_channel_triangle_t *ch);
void apu_triangle_set_lc(apu_channel_triangle_t *ch, uint8_t val);
void apu_triangle_quarter_frame(apu_channel_triangle_t *ch, const apu_reg_triangle_t *reg);
void apu_triangle_half_frame(apu_channel_triangle_t *ch, apu_reg_triangle_t *reg);
void apu_triangle_tick(apu_channel_triangle_t *ch, const apu_reg_triangle_t *reg);
uint8_t apu_triangle_sample(const apu_channel_triangle_t *ch, const apu_reg_triangle_t *reg);

bool apu_noise_is_active(const apu_channel_noise_t *ch);
void apu_noise_deactivate(apu_channel_noise_t *ch);
void apu_noise_set_lc(apu_channel_noise_t *ch, uint8_t val);
void apu_noise_quarter_frame(apu_channel_noise_t *ch, const apu_reg_noise_t *reg);
void apu_noise_half_frame(apu_channel_noise_t *ch, apu_reg_noise_t *reg);
void apu_noise_tick(apu_channel_noise_t *ch, const apu_reg_noise_t *reg);
uint8_t apu_noise_sample(const apu_channel_noise_t *ch, const apu_reg_noise_t *reg);

void apu_dmc_restart(apu_channel_dmc_t *ch);
bool apu_dmc_is_active(const apu_channel_dmc_t *ch);
void apu_dmc_tick(apu_channel_dmc_t *ch, const apu_reg_dmc_t *reg);
uint8_t apu_dmc_sample(const apu_channel_dmc_t *ch, const apu_reg_dmc_t *reg);