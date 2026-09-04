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

#define get_timer_bits(tl, lth) (((0x07 & lth) << 8) | tl)

typedef struct pulse {
	byte dlcv;
	byte epns;
	byte tl;
	byte lth;
} pulse;

typedef struct triangle {
	byte cr;
	byte unused;
	byte tl;
	byte lth;
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

void clock_apu(SDL_AudioStream *stream);

#ifdef __cplusplus
}
#endif
