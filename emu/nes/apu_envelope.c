#include <nes/apu_channels.h>

void apu_envelope_quarter_frame(apu_envelope_t *evl, bool loop_flag, uint8_t period) {
	if (evl->start_flag) {
		evl->start_flag = false;
		evl->divider = period;
		evl->decay = 15;
		return;
	}
	
	if (evl->divider > 0) {
		evl->divider--;
		return;
	} else {
		evl->divider = period;
	}
	
	if (evl->decay > 0) {
		evl->decay--;
	} else if (loop_flag) {
		evl->decay = 15;
	}
}

uint8_t apu_envelope_output(const apu_envelope_t *evl, bool active, bool constant, uint8_t volume) {
	if (active) {
		if (constant) {
			return volume;
		} else {
			return evl->decay;
		}
	} else {
		return 0;
	}
}
