#pragma once

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

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
