#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define set_bit_to(byte, bit, vl) (byte = ((vl << bit) | byte))
#define get_bit(byte, bit) ((byte >> bit) & 1)
#define set_bit(byte, bit) (byte = ((1 << bit) | byte))
#define clear_bit(byte, bit) (byte = (~(1 << bit) & byte))
#define check_bit(byte, shift) ((byte >> shift) & 1)
#define make_word(hb, lb) ((hb << 8) | lb)
#define high_byte(word) (word >> 8)
#define low_byte(word) (word)

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;

#ifdef __cplusplus
}
#endif
