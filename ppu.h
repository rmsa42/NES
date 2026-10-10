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
#define PPUDATA_VECTOR 0x2007
#define OAMDMA_VECTOR 0x4014

#define PPU_MEM_SIZE 16384 // bytes

#define get_pixel_offset(scroll) (scroll & 0x07)
#define get_coarse(scroll) (scroll >> 3)
#define set_fine_y(v, fine_y) (v = v | (fine_y << 12))
#define set_coarse_x(v, coarse_x) (v = v | coarse_x)
#define set_coarse_y(v, coarse_y) (v = v | (coarse_y << 5))

#define get_fine_y(v) (v >> 12)
#define get_nametable_addr(v) (0x2000 | (v & 0x0FFF))
#define get_coarse_x(v) (v & 0x001F)
#define get_coarse_y(v) (v & 0x03E0)
#define clear_coarse_x(v) (v = v & ~(0x001F))
#define clear_coarse_y(v) (v = v & ~(0x03E0))
#define clear_fine_y(v) (v = v & ~(0x7000))
#define toggle_hz_nametable(v) (v = v ^ 0x0400)
#define toggle_vt_nametable(v) (v = v ^ 0x0800)

typedef enum {
	GREY,
	BG_LFM,
	SP_LFM,
	BG_RND,
	SP_RND,
	RED,
	GREEN,
	BLUE
} mask;

typedef enum {
	OPEN_BUS,
	SP_OV = 5,
	SP_HIT,
	VBLANK
} ppu_status;

struct ppu {
	byte ppu_ctrl;
	byte ppu_mask;
	byte ppu_status;
	byte oam_addr;
	byte oam_data;
	byte ppu_addr;
	byte ppu_data;
	byte oam_dma;

	word v;
	word t;
	byte x;
	bool w;
};

void ppu_init();
void ppu_write(word addr, byte value);
byte ppu_read(word addr);
void clock_ppu(struct display *nes_dp, byte cycles);

#ifdef __cplusplus
}
#endif
