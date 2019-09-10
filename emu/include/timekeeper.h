#pragma once

#include <base.h>
#include <reset_manager.h>

#include <stddef.h>
#include <stdint.h>

#define TIMEKEEPER_MAX_TIMERS 16

typedef struct timekeeper {
	struct {
		void (*nullable fire)(void * nonnull obj);
		uint64_t * nullable countdown;
		void * nullable /*strong*/ obj;
	} timers[TIMEKEEPER_MAX_TIMERS];
	size_t ntimers;

	uint32_t t_ref;
	uint32_t t_pause;

	double clk_period;
	uint64_t clk_cyclenum;
} timekeeper_t;

// Constructs a new timekeeper, with the relationship between virtual and real
// time defined by `clk_period`.
timekeeper_t * nullable timekeeper_new (reset_manager_t * nonnull rm, double clk_period);

// Registers a new timer with `tk`. `countdown` is a pointer to the countdown
// value, which must stay valid memory for the duration of the timer's
// existence (since there is currently no way to destroy timers, this means
// meeting or exceeding the timekeeper's lifetime). `fire` is a pointer to a
// routine which is invoked with the reference-counted object `timer` when
// `*countdown` reaches `0`. `*countdown` is decremented automatically during
// calls to `timekeeper_advance_clk()`, and can be freely modified externally.
void timekeeper_add_timer (timekeeper_t * nonnull tk, void * nonnull timer, void * nonnull fire, uint64_t * nonnull countdown);

// Advances virtual time (the system clock) by `ncycles`.
void timekeeper_advance_clk (timekeeper_t * nonnull tk, uint64_t ncycles);

// If virtual time is ahead of real time, synchronously waits until they
// correspond. Otherwise, does nothing.
void timekeeper_sync (timekeeper_t * nonnull tk);

// All real time that passes in-between calls to `timekeeper_pause()` and
// `timekeeper_resume()` is ignored by the timekeeper when later calculating
// how long to `timekeeper_sync()`. The behavior of unbalanced calls to these
// functions is undefined.
//
// After being reset, a timekeeper is in an "unpaused" state, and the behavior
// of invoking `timekeeper_resume()` before pausing is undefined.
void timekeeper_pause (timekeeper_t * nonnull tk);
void timekeeper_resume (timekeeper_t * nonnull tk);
