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

#define PPU_MEM_SIZE		16384	// bytes
#define PPU_OAM1_MEM_SIZE	256		// bytes
#define PPU_OAM2_MEM_SIZE	32		// bytes
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

#define GET_PATTERN_ADDR(table_addr, tile, row) (table_addr | (tile * 16 + row))
#define GET_NAMETABLE_ADDR(v)					(0x2000 | (v & 0x0FFF))
#define GET_ATTRIBUTE_ADDR(v)					(0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07))

#define GET_COARSE(scroll)						(scroll >> 3)
#define GET_PIXEL_OFFSET(scroll)				(scroll & 0x07)

#define SET_COARSE_X(v, coarse_x)				(v |= coarse_x)
#define GET_COARSE_X(v)							(v & 0x001F)
#define CLEAR_COARSE_X(v)						(v &= ~(0x001F))

#define SET_COARSE_Y(v, coarse_y)				(v |= coarse_y << 5)
#define GET_COARSE_Y(v)							((v & 0x03E0) >> 5)
#define CLEAR_COARSE_Y(v) 						(v &= ~(0x03E0))

#define SET_FINE_Y(v, fine_y)					(v |= fine_y << 12)
#define GET_FINE_Y(v)							(v >> 12)
#define CLEAR_FINE_Y(v)							(v &= ~(0x7000))

#define GET_OAM_INDEX(n, m)						(n * 4 + m)

#define MERGE_PLANE_BITS(high, low, bit)		(((GET_BIT(high, bit)) << 1) | (GET_BIT(low, bit)))
#define SPRITE_PALETTE_INDEX(attr)				(attr & 0x03)
#define MAKE_PIXEL(attr, pixel)					(attr << 2 | pixel)

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
	SP_HIT = 6,
	VBLANK = 7
};

enum ppu_ctrl_bit {
	NAMETABLE_ADDR1 = 0,
	NAMETABLE_ADDR2 = 1,
	VRAM_INC = 2,
	SP_PATT_ADDR = 3,
	BG_PATT_ADDR = 4
};

typedef struct Shifter {
	word low_shift_reg;
	word high_shift_reg;
} Shifter;

typedef struct Patt_latch {
	byte patt_low;
	byte patt_high;
} Patt_latch;

typedef struct Attr_latch {
	byte attr_low;
	byte attr_high;
} Attr_latch;

typedef struct Bg_pixel_data {
	Shifter patt_shifter;
	Shifter attr_shifter;
} Bg_pixel_data;

typedef struct Render_data_latch {
	byte nametable;
	Attr_latch attr_latch;
	Patt_latch patt_latch;
} Render_data_latch;

typedef struct Sp_pixel_data {
	byte attr;
	byte xcounter;
	Shifter patt_shifter;
} Sp_pixel_data;

typedef struct Ppu {
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

	Bg_pixel_data bg_pixel_data;
	Sp_pixel_data sprite_data[8];
	Render_data_latch render_dt_latch;
} Ppu;

void ppu_init();
void ppu_write(word addr, byte value);
byte ppu_read(word addr);
void clock_ppu(Display *nes_dp, byte cycles);
void ppu_write_mem(word addr, byte value);
byte ppu_read_mem(word addr);

// Just for making tests
void render_pixel(byte bg_pixel, byte sp_pixel, byte tile_nbr, byte pixel_nbr, bool render_flag);
byte get_background_pixel(Bg_pixel_data *bg_pixel_data);
void scanline();

#ifdef __cplusplus
}
#endif
