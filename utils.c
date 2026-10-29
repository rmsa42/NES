#include "utils.h"

byte mirror_hz_byte(byte b) {
	byte ret = 0;

	for (int i = 7; i >= 0; i--) {
		SET_BIT_TO(ret, (7 - i), GET_BIT(b, i));
	}
	return (ret);
}
