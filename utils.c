#include "utils.h"

byte mirror_hz_byte(byte b) {
	byte ret = 0;

	for (int i = 7; i >= 0; i--) {
		set_bit_to(ret, (7 - i), check_bit(b, i));
	}
	return (ret);
}
