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
#define PPU_OAM1_MEM_SIZE 256 // bytes
#define PPU_OAM2_MEM_SIZE 32 // bytes

#define TOTAL_SPRITE_NBR 64
#define TOTAL_SPRITE_IN_SCANLINE 8
#define TRANSPARENT_PIXEL 0

#define get_pattern_addr(table_addr, tile, row) (table_addr | (tile * 16 + row))
#define get_pixel_offset(scroll) (scroll & 0x07)
#define get_coarse(scroll) (scroll >> 3)
#define set_fine_y(v, fine_y) (v = v | (fine_y << 12))
#define set_coarse_x(v, coarse_x) (v = v | coarse_x)
#define set_coarse_y(v, coarse_y) (v = v | (coarse_y << 5))
#define get_fine_y(v) (v >> 12)
#define get_nametable_addr(v) (0x2000 | (v & 0x0FFF))
#define get_attribute_addr(v) (0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07))
#define get_coarse_x(v) (v & 0x001F)
#define get_coarse_y(v) ((v & 0x03E0) >> 5)
#define clear_coarse_x(v) (v = v & ~(0x001F))
#define clear_coarse_y(v) (v = v & ~(0x03E0))
#define clear_fine_y(v) (v = v & ~(0x7000))
#define toggle_hz_nametable(v) (v = v ^ 0x0400)
#define toggle_vt_nametable(v) (v = v ^ 0x0800)
#define merge_plane_bits(high_b, low_b) ((high_b << 1) | low_b)
#define oam_offset(n, m) (n * 4 + m)

#define SCANLINE_TILE_COUNT 32
#define SCANLINE_CPU_CYCLES 113.667f

typedef enum {
	SP_Y = 0,
	SP_TILE = 1,
	SP_ATTR = 2,
	SP_X = 3,
} oam_sprite_byte;

typedef enum {
	GREY,
	BG_LFM,
	SP_LFM,
	BG_RND,
	SP_RND,
	RED,
	GREEN,
	BLUE
} ppu_mask_bit;

typedef enum {
	OPEN_BUS,
	SP_OV = 5,
	SP_HIT,
	VBLANK
} ppu_status_bit;

typedef enum {
	NAMETABLE_HORIZONTAL = 0,
	NAMETABLE_VERTICAL = 1,
	VRAM_INC = 2,
	SP_PATT_ADDR = 3,
	BG_PATT_ADDR = 4
} ppu_ctrl_bit;

typedef struct pixel_shift_reg {
	word low_shift_reg;
	word high_shift_reg;
} pixel_shifter;

typedef struct patt_latch {
	byte patt_low;
	byte patt_high;
} patt_latch;

typedef struct attr_latch {
	byte attr_low;
	byte attr_high;
} attr_latch;

typedef struct bg_pixel_data {
	pixel_shifter patt_shifter;
	pixel_shifter attr_shifter;
} bg_pixel_data;

typedef struct render_data_latch {
	byte nametable;
	attr_latch attr_latch;
	patt_latch patt_latch;
} render_data_latch;

typedef struct sp_pixel_data {
	byte attr;
	byte xcounter;
	pixel_shifter patt_shifter;
} sp_pixel_data;

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

	bg_pixel_data pixel_data;
	sp_pixel_data sprite_data[8];
	render_data_latch render_dt_latch;
};

void ppu_init();
void ppu_write(word addr, byte value);
byte ppu_read(word addr);
void clock_ppu(struct display *nes_dp, byte cycles);
void ppu_write_mem(word addr, byte value);
byte ppu_read_mem(word addr);

// Just for making tests
void render_pattern_table(const int table_nbr, struct display *nes_dp);
void scanline();

#ifdef __cplusplus
}
#endif
