#pragma once

#include "utils.h"
#include "sdl_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define JOY1_VECTOR 0x4016
#define JOY2_VECTOR 0x4017

typedef enum {
	A = 0,
	B = 1,
	SELECT = 2,
	START = 3,
	UP = 4,
	DOWN = 5,
	LEFT = 6,
	RIGHT = 7,
} joy_btn;

struct controller {
	bool poll;
	byte input_shift_reg;
	byte input_buffer;
};

void controller_write(word addr, byte value);
byte controller_read(word addr);
void set_btn(byte btn, byte state);

#ifdef __cplusplus
}
#endif
