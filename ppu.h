#pragma once

#include "utils.h"
#include "sdl_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PPUCTRL_VECTOR 0x2000
#define PPUMASK_VECTOR 0x2001
#define PPUSTATUS_VECTOR 0x2002
#define OAMADDR_VECTOR 0x2003
#define OAMDATA_VECTOR 0x2004
#define PPUSCROLL_VECTOR 0x2005
#define PPUADDR_VECTOR 0x2006
#define PPUDATA 0x2007
#define OAMDMA_VECTOR 0x4014

struct ppu {
	byte ppu_ctrl;
	byte ppu_mask;
	byte ppu_status;
	byte oam_addr;
	byte oam_data;
	byte ppu_scroll;
	byte ppu_addr;
	byte ppu_data;
	byte oam_dma;
};

void ppu_init();
void clock_ppu(struct display *nes_dp);

#ifdef __cplusplus
}
#endif
