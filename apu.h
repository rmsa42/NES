#pragma once

#include "utils.h"
#include "SDL3/SDL_audio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLE_RATE 44100 // Hz

#define SQUARE1_VECTOR 0x4000
#define SQUARE2_VECTOR 0x4004
#define TRIANGLE_VECTOR 0x4008
#define NOISE_VECTOR 0x400C
#define DMC_VECTOR 0x4010

#define get_timer(tl, lth) (((0x07 & lth) << 8) | tl)
#define get_duty(dlcv) (dlcv >> 6)
#define get_evelope_loop(dlcv) ((dlcv & 0x20) >> 5)
#define get_len_counter_load(lth) ((lth & 0xF8) >> 3)
#define get_lin_counter_load(cr) (cr & 0x7F)
#define get_noise_mode(mp) (mp >> 7)
#define get_noise_period(mp) (mp & 0x0F)

typedef struct pulse {
	float duty;
	byte env_loop;
	byte const_volume;
	byte volume;
	byte sweep;
	byte sweep_period;
	byte sweep_negate;
	byte sweep_shift;
	word timer;
	byte len_counter_load;
} pulse;

typedef struct triangle {
	byte control_flag; // Also is the length_counter_flag
	byte lin_reload_flag;
	byte lin_counter_load;
	word timer;
	byte len_counter_load;
	byte unused;
} triangle;

typedef struct noise {
	byte env_loop;
	byte const_volume;
	byte volume;
	byte mode;
	int period;
	byte length_counter_load;
	word shift_reg;
	byte unused;
} noise;

typedef struct dmc {
	byte ilr;
	byte d;
	byte saddr;
	byte slen;
} dmc;

struct apu {
	pulse square1;
	pulse square2;
	triangle triangle;
	noise noise;
	dmc dmc;
	byte status;
	byte frame_ct;
};

int apu_init();
void clock_apu(SDL_AudioStream *stream);

#ifdef __cplusplus
}
#endif
