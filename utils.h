#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SET_BIT_TO(byte, bit, vl)	(byte = ((byte & (~(1 << bit))) | (vl << bit)))
#define SET_BIT(byte, bit)			(byte = ((1 << bit) | byte))
#define CLEAR_BIT(byte, bit)		(byte = (~(1 << bit) & byte))
#define GET_BIT(byte, shift)		((byte >> shift) & 1)
#define MAKE_WORD(hb, lb)			((hb << 8) | lb)
#define GET_HIGH_BYTE(word)			(word >> 8)
#define GET_LOW_BYTE(word)			(word & 0xFF)
#define TOGGLE_BIT(byte, bit)		(byte ^= 1 << bit)

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;

byte mirror_hz_byte(byte b);
void print_buff(byte *buff, const size_t size);

#ifdef __cplusplus
}
#endif
