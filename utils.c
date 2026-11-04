#include "utils.h"
#include <stdio.h>

byte mirror_hz_byte(byte b) {
	byte ret = 0;

	for (int i = 7; i >= 0; i--) {
		SET_BIT_TO(ret, (7 - i), GET_BIT(b, i));
	}
	return (ret);
}

void print_buff(byte *buff, const size_t size) {
	for (size_t i = 0; i < size; i++) {
		printf("%zx: %x\n", i, buff[i]);
	}
}

