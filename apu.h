#pragma once

#include "utils.h"
#include "sdl_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SQUARE1		0x4000
#define SQUARE2		0x4004
#define TRIANGLE	0x4008
#define NOISE		0x400C
#define DMC			0x4010

#define get_timer(tl, lth)			(((0x07 & lth) << 8) | tl)
#define get_duty(dlcv)				(dlcv >> 6)
#define get_evelope_loop(dlcv)		(dlcv >> 5)
#define get_len_counter_load(lth)	((lth & 0xF8) >> 3)
#define get_lin_counter_load(cr)	(cr & 0x7F)

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
	byte lcv;
	byte unused;
	byte mp;
	byte lc;
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

void apu_write(word addr, byte value);
void clock_apu(struct display *nes_dp);

#ifdef __cplusplus
}
#endif
