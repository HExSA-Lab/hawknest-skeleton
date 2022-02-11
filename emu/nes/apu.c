#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include <base.h>
#include <rc.h>
#include <mos6502/mos6502.h>
#include <nes/apu.h>
#include <nes/apu_regs.h>
#include <nes/apu_channels.h>

#define SAMPLE_COUNT 65536
#define QUARTER_FRAME (358000 / 4)
#define ABOUT_44_1_KHZ 487

static const uint8_t length_counters[32] = {
    10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
	12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30,
};

typedef struct apu_frame_counter {
	uint8_t step;
	bool irq_flag;
} apu_frame_counter_t;

typedef struct apu {
	union {
		apu_regs_t regs;
		uint8_t reg_bytes[0x18];
	};
	
	mos6502_t * nonnull /*unowned*/ cpu;

	apu_channel_pulse_t pulse1;
	apu_channel_pulse_t pulse2;
	apu_channel_triangle_t triangle;
	apu_channel_noise_t noise;
	apu_channel_dmc_t dmc;
	
	apu_frame_counter_t frame_counter;

	float * nonnull /*owned*/ samples;
	size_t nsamples;
	SDL_AudioDeviceID dev;
	
	uint64_t cycle_countdown;
	uint64_t frame_countdown;
	bool even_cycle;
	uint64_t sample_countdown;
} apu_t;

static void fc_update_irq(apu_frame_counter_t *fc, const apu_reg_frame_counter_t *reg) {
	if (reg->disable_frame_irq) {
		fc->irq_flag = false;
	} else if (fc->step == 3 && !reg->five_step_sequence) {
		fc->irq_flag = true;
	}
}

static void fc_step(apu_frame_counter_t *fc, const apu_reg_frame_counter_t *reg) {
	fc->step++;
	fc->step %= reg->five_step_sequence ? 5 : 4;
	fc_update_irq(fc, reg);
}

static inline bool fc_at_quarter(const apu_frame_counter_t *fc, const apu_reg_frame_counter_t *reg) {
	return reg->five_step_sequence ? fc->step != 3 : true;
}

static inline bool fc_at_half(const apu_frame_counter_t *fc, const apu_reg_frame_counter_t *reg) {
	if (reg->five_step_sequence) {
		return fc->step == 1 || fc->step == 4;
	} else {
		return fc->step == 1 || fc->step == 3;
	}
}

static float apu_mix_sample(const apu_t *apu) {
	double pulse1 = apu_pulse_sample(&apu->pulse1, &apu->regs.pulse1);
	double pulse2 = apu_pulse_sample(&apu->pulse2, &apu->regs.pulse2);
	double triangle = apu_triangle_sample(&apu->triangle, &apu->regs.triangle);
	double noise = apu_noise_sample(&apu->noise, &apu->regs.noise);
	double dmc = apu_dmc_sample(&apu->dmc, &apu->regs.dmc);

	double tnd_out = 0.;
	if (triangle + noise + dmc != 0.) {
		tnd_out = 159.79 / (1. / (triangle / 8227. + noise / 12241. + dmc / 22638.) + 100.);
	}

	double pulse_out = 0.;
	if (pulse1 + pulse2 != 0.) {
		pulse_out = 95.88 / (8128. / (pulse1 + pulse2) + 100.);
	}

	return (float)(tnd_out + pulse_out);
}

static void apu_push_sample(apu_t *apu, float sample) {
	if (apu->nsamples < SAMPLE_COUNT) {
		apu->samples[apu->nsamples++] = sample;
	}
}

static void apu_cycle_tick(apu_t *apu) {
	apu->cycle_countdown = MOS6502_CLKDIVISOR;

	if (apu->dmc.mem_reader.irq_flag || apu->frame_counter.irq_flag) {
		if (!apu->cpu->p.i) {
			mos6502_raise_irq(apu->cpu);
		}
	}

	apu_triangle_tick(&apu->triangle, &apu->regs.triangle);
	if (apu->even_cycle) {
		apu_pulse_tick(&apu->pulse1, &apu->regs.pulse1);
		apu_pulse_tick(&apu->pulse2, &apu->regs.pulse2);
		apu_noise_tick(&apu->noise, &apu->regs.noise);
		apu_dmc_tick(&apu->dmc, &apu->regs.dmc);
	}

	apu->even_cycle = !apu->even_cycle;
}

static void apu_frame_tick(apu_t *apu) {
	apu->frame_countdown = QUARTER_FRAME;
	fc_step(&apu->frame_counter, &apu->regs.frame_counter);

	if (fc_at_quarter(&apu->frame_counter, &apu->regs.frame_counter)) {
		apu_pulse_quarter_frame(&apu->pulse1, &apu->regs.pulse1);
		apu_pulse_quarter_frame(&apu->pulse2, &apu->regs.pulse2);
		apu_triangle_quarter_frame(&apu->triangle, &apu->regs.triangle);
		apu_noise_quarter_frame(&apu->noise, &apu->regs.noise);
	}
	if (fc_at_half(&apu->frame_counter, &apu->regs.frame_counter)) {
		apu_pulse_half_frame(&apu->pulse1, &apu->regs.pulse1);
		apu_pulse_half_frame(&apu->pulse2, &apu->regs.pulse2);
		apu_triangle_half_frame(&apu->triangle, &apu->regs.triangle);
		apu_noise_half_frame(&apu->noise, &apu->regs.noise);
	}

	SDL_QueueAudio(apu->dev, apu->samples, apu->nsamples * sizeof(float));
	
	apu->nsamples = 0;
}

static void apu_sample_tick(apu_t *apu) {
	apu->sample_countdown = ABOUT_44_1_KHZ;
	apu_push_sample(apu, apu_mix_sample(apu));
}

uint8_t apu_mem_read(apu_t *apu, uint16_t addr, uint8_t *lane_mask) {
	if (addr == 0x15) {
		*lane_mask = 0xFF;

		apu_reg_control_status_t status;
		status.val = 0;

		status.pulse1 = apu_pulse_is_active(&apu->pulse1);
		status.pulse2 = apu_pulse_is_active(&apu->pulse2);
		status.triangle = apu_triangle_is_active(&apu->triangle);
		status.noise = apu_noise_is_active(&apu->noise);
		status.dmc = apu_dmc_is_active(&apu->dmc);
		
		status.dmc_irq = apu->dmc.mem_reader.irq_flag;
		status.frame_irq = apu->frame_counter.irq_flag;
		
		apu->frame_counter.irq_flag = false;
		
		return status.val;
	} else {
		*lane_mask = 0;
		return 0;
	}
}

void apu_mem_write(apu_t *apu, uint16_t addr, uint8_t data) {
	if (addr > 0x17) {
		return;
	}
	apu->reg_bytes[addr] = data;
	uint8_t length_counter = length_counters[data >> 3];
	switch (addr) {
		case 0x01:
			apu_pulse_reload_sweep(&apu->pulse1);
			break;
		case 0x03:
			apu_pulse_set_lc(&apu->pulse1, length_counter);
			break;
		case 0x05:
			apu_pulse_reload_sweep(&apu->pulse2);
			break;
		case 0x07:
			apu_pulse_set_lc(&apu->pulse2, length_counter);
			break;
		case 0x0B:
			apu_triangle_set_lc(&apu->triangle, length_counter);
			break;
		case 0x0F:
			apu_noise_set_lc(&apu->noise, length_counter);
			break;
		case 0x10:
			if (!apu->regs.dmc.irq) {
				apu->dmc.mem_reader.irq_flag = false;
			}
			break;
		case 0x11:
			apu->dmc.output_unit.output_level = apu->regs.dmc.direct_load;
			break;
		case 0x12:
			apu->dmc.mem_reader.sample_start = apu->regs.dmc.sample_addr * 64 + 0xC000;
			apu->dmc.mem_reader.addr = apu->dmc.mem_reader.sample_start;
			break;
		case 0x13:
			apu->dmc.mem_reader.sample_len = apu->regs.dmc.sample_len * 16 + 1;
			apu->dmc.mem_reader.bytes_remaining = apu->dmc.mem_reader.sample_len;
			break;
		case 0x15:
			apu->dmc.mem_reader.irq_flag = false;
			if (apu->regs.control.dmc) {
				apu_dmc_restart(&apu->dmc);
			}
			break;
		case 0x17:
			apu->frame_counter.step = 0;
			if (apu->regs.frame_counter.five_step_sequence) {
				apu_pulse_quarter_frame(&apu->pulse1, &apu->regs.pulse1);
				apu_pulse_quarter_frame(&apu->pulse2, &apu->regs.pulse2);
				apu_triangle_quarter_frame(&apu->triangle, &apu->regs.triangle);
				apu_noise_quarter_frame(&apu->noise, &apu->regs.noise);

				apu_pulse_half_frame(&apu->pulse1, &apu->regs.pulse1);
				apu_pulse_half_frame(&apu->pulse2, &apu->regs.pulse2);
				apu_triangle_half_frame(&apu->triangle, &apu->regs.triangle);
				apu_noise_half_frame(&apu->noise, &apu->regs.noise);
			}
			fc_update_irq(&apu->frame_counter, &apu->regs.frame_counter);
			break;
		default:
			break;
	}
}

static void deinit(apu_t * nonnull apu) {
	free(apu->samples);

	SDL_CloseAudioDevice(apu->dev);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

static void reset(apu_t * nonnull apu) {
	#define ZERO(x) memset(&x, 0, sizeof(x));
	
	ZERO(apu->pulse1);
	ZERO(apu->pulse2);
	apu->pulse2.is_pulse2 = true;

	ZERO(apu->triangle);

	ZERO(apu->noise);
	apu->noise.shift_reg = 1;

	ZERO(apu->dmc.mem_reader.sample_start);
	ZERO(apu->dmc.mem_reader.sample_len);
	ZERO(apu->dmc.mem_reader.addr);
	ZERO(apu->dmc.mem_reader.bytes_remaining);
	ZERO(apu->dmc.sample_buffer);
	ZERO(apu->dmc.timer);
	ZERO(apu->dmc.output_unit);
	
	ZERO(apu->frame_counter);

	ZERO(apu->even_cycle);
	ZERO(apu->reg_bytes);

	apu->cycle_countdown = MOS6502_CLKDIVISOR;
	apu->frame_countdown = QUARTER_FRAME;
	apu->sample_countdown = ABOUT_44_1_KHZ;

	#undef ZERO
}

apu_t *
apu_new(reset_manager_t *rm, mos6502_t *cpu)
{
	apu_t * apu = rc_alloc(sizeof(apu_t), deinit);
	reset_manager_add_device(rm, apu, reset);
	timekeeper_add_timer(cpu->tk, apu, apu_cycle_tick, &apu->cycle_countdown);
	timekeeper_add_timer(cpu->tk, apu, apu_frame_tick, &apu->frame_countdown);
	timekeeper_add_timer(cpu->tk, apu, apu_sample_tick, &apu->sample_countdown);
	
	apu->cpu = cpu;
	apu->dmc.mem_reader.bus = cpu->bus;

	apu->pulse2.is_pulse2 = true;
	apu->samples = calloc(SAMPLE_COUNT, sizeof(float));
	
	if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
		ERROR_PRINT("Could not init SDL audio: %s", SDL_GetError());
		goto initerror;
	}
	
	SDL_AudioSpec spec;
	spec.freq = 44100;
	spec.format = AUDIO_F32SYS;
	spec.channels = 1;
	spec.callback = NULL;
	apu->dev = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
	if (!apu->dev) {
		ERROR_PRINT("Could not open audio device: %s", SDL_GetError());
		goto deverror;
	}

	SDL_PauseAudioDevice(apu->dev, 0);
	
	return apu;
	
deverror:
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
initerror:
	free(apu->samples);
	rc_release(apu);
	return NULL;
}