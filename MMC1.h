#pragma once

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CHR0	0xA000
#define CHR1	0xC000
#define PRG0	0xE000
#define CONTROL 0x8000

#define PRG_BANK_MODE(ctrl_reg) ((ctrl_reg & 0x0C) >> 2)

typedef struct MMC1 {
	byte shift_reg;
	byte ct;

	byte control_reg;
} MMC1;

void MMC1_init();
void MMC1_write(word addr, byte value);

#ifdef __cplusplus
}
#endif
