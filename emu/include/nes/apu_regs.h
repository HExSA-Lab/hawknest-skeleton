#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct apu_volume {
	uint8_t level: 4;
	bool constant: 1;
	bool lch: 1;
	uint8_t duty: 2;
} apu_volume_t;

typedef struct apu_sweep {
	uint8_t shift_count: 3;
	bool negative: 1;
	uint8_t period: 3;
	bool enabled: 1;
} apu_sweep_t;

typedef struct apu_timer {
	uint16_t timer: 11;
	uint8_t load: 5;
} apu_timer_t;

typedef struct apu_triangle_counter {
	uint8_t reload: 7;
	bool control: 1;
} apu_triangle_counter_t;

typedef struct apu_reg_pulse {
	apu_volume_t volume;
	apu_sweep_t sweep;
	apu_timer_t timer;
} apu_reg_pulse_t;

typedef struct apu_reg_triangle {
	apu_triangle_counter_t counter;
	uint8_t _unused_4009;
	apu_timer_t timer;
} apu_reg_triangle_t;

typedef struct apu_reg_noise {
	uint8_t volume: 4;
	bool constant: 1;
	bool lch: 1;
	uint8_t _unused_a: 2;

	uint8_t _unused_400d: 8;
	
	uint8_t period: 4;
	uint8_t _unused_b: 3;
	bool mode: 1;
	
	uint8_t _unused_c: 3;
	uint8_t load: 5;
} apu_reg_noise_t;

typedef struct apu_reg_dmc {
	uint8_t freq_idx: 4;
	uint8_t _unused_a: 2;
	bool repeat: 1;
	bool irq: 1;
	
	uint8_t direct_load: 7;
	uint8_t _unused_b: 1;
	
	uint8_t sample_addr: 8;
	
	uint8_t sample_len: 8;
} apu_reg_dmc_t;

typedef union apu_reg_control_status {
	struct {
		bool pulse1: 1;
		bool pulse2: 1;
		bool triangle: 1;
		bool noise: 1;
		bool dmc: 1;
		uint8_t _unused: 1;
		bool frame_irq: 1;
		bool dmc_irq: 1;
	};
	uint8_t val;
} apu_reg_control_status_t;

typedef struct apu_reg_frame_counter {
	uint8_t unused: 6;
	bool disable_frame_irq: 1;
	bool five_step_sequence: 1;
} apu_reg_frame_counter_t;

typedef struct apu_regs {
	// $4000-$4003
	apu_reg_pulse_t pulse1;
	// $4003-$4007
	apu_reg_pulse_t pulse2;
	// $4008-$400B
	apu_reg_triangle_t triangle;
	// $400C-$400F
	apu_reg_noise_t noise;
	// $4010-$4013
	apu_reg_dmc_t dmc;
	// $4014
	uint8_t _unused_4014;
	// $4015
	apu_reg_control_status_t control;
	// $4016
	uint8_t _unused_4016;
	// $4017
	apu_reg_frame_counter_t frame_counter;
} apu_regs_t;
