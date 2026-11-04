#pragma once

#include <stdbool.h>
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NES_EOF			0x1A
#define PALLETE_SIZE	0xBD

typedef struct Nes_header {
	bool ines_format;
	bool nes2_format;

	bool trainer_area_flag;

	word prg_rom_size;
	word chr_rom_size;
} Nes_header;

int read_rom(const char *rom_name);

#ifdef __cplusplus
}
#endif
