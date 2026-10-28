#pragma once

#include "utils.h"
#include "sdl_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PPUCTRL		0x2000
#define PPUMASK		0x2001
#define PPUSTATUS	0x2002
#define OAMADDR		0x2003
#define OAMDATA		0x2004
#define PPUSCROLL	0x2005
#define PPUADDR		0x2006
#define PPUDATA		0x2007
#define OAMDMA		0x4014

#define PPU_MEM_SIZE		16384 // bytes
#define PPU_OAM1_MEM_SIZE	256 // bytes
#define PPU_OAM2_MEM_SIZE	32 // bytes
#define SCANLINE_CPU_CYCLES 113.667f

#define TOTAL_SPRITE_NBR			64
#define TOTAL_SPRITE_IN_SCANLINE	8
#define TOTAL_TILES_IN_SCANLINE		32
#define TOTAL_ROWS_IN_SCANLINE		30
#define TRANSPARENT_PIXEL			0
#define NAMETABLE_HORIZONTAL_BIT	10
#define NAMETABLE_VERTICAL_BIT		11

#define PATTERN_TABLE_0_ADDR		0x0000
#define PATTERN_TABLE_1_ADDR		0x1000
#define BACKGROUND_PALETTES_ADDR	0x3F00
#define SPRITE_PALETTES_ADDR		0x3F10

#define get_pattern_addr(table_addr, tile, row) (table_addr | (tile * 16 + row))
#define get_pixel_offset(scroll)				(scroll & 0x07)
#define get_coarse(scroll)						(scroll >> 3)
#define set_fine_y(v, fine_y)					(v = v | (fine_y << 12))
#define set_coarse_x(v, coarse_x)				(v = v | coarse_x)
#define set_coarse_y(v, coarse_y)				(v = v | (coarse_y << 5))
#define get_fine_y(v)							(v >> 12)
#define get_nametable_addr(v)					(0x2000 | (v & 0x0FFF))
#define get_attribute_addr(v)					(0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07))
#define get_coarse_x(v)							(v & 0x001F)
#define get_coarse_y(v)							((v & 0x03E0) >> 5)
#define clear_coarse_x(v)						(v = v & ~(0x001F))
#define clear_coarse_y(v) 						(v = v & ~(0x03E0))
#define clear_fine_y(v)							(v = v & ~(0x7000))
#define toggle_hz_nametable(v)					(v = v ^ 0x0400)
#define toggle_vt_nametable(v)					(v = v ^ 0x0800)
#define merge_plane_bits(high, low, bit)		(((check_bit(high, bit)) << 1) | (check_bit(low, bit)))
#define oam_offset(n, m)						(n * 4 + m)
#define sprite_attr_palette(attr)				(attr & 0x03)
#define make_pixel(attr, pixel)					(attr << 2 | pixel)

enum oam_sprite_byte {
	SP_Y = 0,
	SP_TILE = 1,
	SP_ATTR = 2,
	SP_X = 3,
};

enum ppu_mask_bit {
	GREYSCALE,
	BG_LFM,
	SP_LFM,
	BG_RND,
	SP_RND,
	EMPH_RED,
	EMPH_GREEN,
	EMPH_BLUE
};

enum ppu_status_bit {
	OPEN_BUS,
	SP_OV = 5,
	SP_HIT,
	VBLANK
};

enum ppu_ctrl_bit {
	NAMETABLE_ADDR1 = 0,
	NAMETABLE_ADDR2 = 1,
	VRAM_INC = 2,
	SP_PATT_ADDR = 3,
	BG_PATT_ADDR = 4
};

typedef struct shifter {
	word low_shift_reg;
	word high_shift_reg;
} shifter;

typedef struct patt_latch {
	byte patt_low;
	byte patt_high;
} patt_latch;

typedef struct attr_latch {
	byte attr_low;
	byte attr_high;
} attr_latch;

typedef struct bg_pixel_data {
	shifter patt_shifter;
	shifter attr_shifter;
} bg_pixel_data;

typedef struct render_data_latch {
	byte nametable;
	attr_latch attr_latch;
	patt_latch patt_latch;
} render_data_latch;

typedef struct sp_pixel_data {
	byte attr;
	byte xcounter;
	shifter patt_shifter;
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

	bg_pixel_data bg_pixel_data;
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
