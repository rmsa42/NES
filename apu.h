#pragma once

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

extern byte memory[0xFFFF];

struct apu {
	dword pulse1;
	dword pulse2;
	dword triangle;
	dword noise;
	dword dmc;
	byte status;
	byte frame_ct;
};

#ifdef __cplusplus
}
#endif
