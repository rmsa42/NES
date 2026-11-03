#include "controller.h"
#include <stdio.h>

struct Controller joy1;

void controller_init() {
	memset(&joy1, 0, sizeof(joy1));
}

void controller_write(word addr, byte value) {
	switch (addr) {
		case JOY1_VECTOR:
			joy1.poll = GET_BIT(value, 0);
			if (joy1.poll) {
				joy1.input_shift_reg = joy1.input_buffer;
			}
			break;

		default:
			break;
	}
}

byte controller_read(word addr) {
	byte input;

	switch (addr) {
		case JOY1_VECTOR:
			input = GET_BIT(joy1.input_shift_reg, 0);
			joy1.input_shift_reg >>= 1;
			return (input);

		default:
			return (0);
	}
}

void set_btn(byte btn, byte state) {
	SET_BIT_TO(joy1.input_buffer, btn, state);
}

