#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define set_bit_to(byte, bit, vl) (byte = ((byte & (~(1 << bit))) | (vl << bit)))
#define set_bit(byte, bit) (byte = ((1 << bit) | byte))
#define clear_bit(byte, bit) (byte = (~(1 << bit) & byte))
#define check_bit(byte, shift) ((byte >> shift) & 1)
#define make_word(hb, lb) ((hb << 8) | lb)
#define high_byte(word) (word >> 8)
#define low_byte(word) (word & 0xFF)

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;

#ifdef __cplusplus
}
#endif
