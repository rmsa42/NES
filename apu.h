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

typedef struct Pulse {
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
} Pulse;

typedef struct Triangle {
	byte control_flag; // Also is the length_counter_flag
	byte lin_reload_flag;
	byte lin_counter_load;
	word timer;
	byte len_counter_load;
	byte unused;
} Triangle;

typedef struct Noise {
	byte lcv;
	byte unused;
	byte mp;
	byte lc;
} Noise;

typedef struct Dmc {
	byte ilr;
	byte d;
	byte saddr;
	byte slen;
} Dmc;

typedef struct Apu {
	Pulse square1;
	Pulse square2;
	Triangle triangle;
	Noise noise;
	Dmc dmc;
	byte status;
	byte frame_ct;
} Apu;

void apu_write(word addr, byte value);
void clock_apu(Display *nes_dp);

#ifdef __cplusplus
}
#endif
