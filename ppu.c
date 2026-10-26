#include "ppu.h"
#include "cpu.h"
#include "rom.h"
#include "bus.h"
#include <string.h>
#include <stdio.h>

extern byte pallete_rgb[PALLETE_SIZE];

struct ppu ppu;
byte ppu_mem[PPU_MEM_SIZE];
byte ppu_oam1_mem[PPU_OAM1_MEM_SIZE];
byte ppu_oam2_mem[PPU_OAM2_MEM_SIZE];
int scanline_nbr = 0;
float cpu_cycle_counter = 0;
byte pixels[256 * 240 * 4];

void ppu_init() {
	memset(&ppu, 0, sizeof(struct ppu));
	memset(ppu_mem, 0, sizeof(ppu_mem));
	memset(ppu_oam1_mem, 0xFF, sizeof(ppu_oam1_mem));
	memset(ppu_oam2_mem, 0, sizeof(ppu_oam2_mem));
}

byte ppu_read_mem(word addr) {
	return (ppu_mem[addr]);
}

void ppu_write_mem(word addr, byte value) {
	ppu_mem[addr] = value;
}

void ppu_internal_write(byte value) {
	ppu_mem[ppu.v] = value;
	ppu.v += check_bit(ppu.ppu_ctrl, VRAM_INC) ? 32 : 1;
}

byte ppu_internal_read() {
	byte ret = ppu_mem[ppu.v];

	ppu.v += check_bit(ppu.ppu_ctrl, VRAM_INC) ? 32 : 1;
	return (ret);
}

void ppu_internal_oam_write(byte value) {
	ppu_oam1_mem[ppu.oam_addr] = value;
	ppu.oam_addr++;
}

byte ppu_internal_oam_read() {
	return (ppu_oam1_mem[ppu.oam_addr]);
}

void ppu_write(word addr, byte value) {
	switch (addr) {
		case PPUCTRL_VECTOR:
			ppu.ppu_ctrl = value;
			set_bit_to(ppu.t, 10, check_bit(ppu.ppu_ctrl, NAMETABLE_HORIZONTAL));
			set_bit_to(ppu.t, 11, check_bit(ppu.ppu_ctrl, NAMETABLE_VERTICAL));
			break;
		case PPUMASK_VECTOR:
			ppu.ppu_mask = value;
			break;
		case OAMADDR_VECTOR:
			ppu.oam_addr = value;
			break;
		case OAMDATA_VECTOR:
			ppu_internal_oam_write(value);
			break;
		case OAMDMA_VECTOR:
			ppu.oam_dma = value;
			for (int i = 0; i < 256; i++) {
				byte byte = bus_read(make_word(value, i));
				ppu_internal_oam_write(byte);
			}
			break;
		case PPUSCROLL_VECTOR:
			if (!ppu.w) {
				ppu.x = get_pixel_offset(value);
				set_coarse_x(ppu.t, get_coarse(value));
			} else {
				set_fine_y(ppu.t, get_pixel_offset(value));
				set_coarse_y(ppu.t, get_coarse(value));
			}
			ppu.w = !ppu.w;
			break;
		case PPUADDR_VECTOR:
			if (!ppu.w) {
				ppu.t = value;
			} else {
				ppu.t = make_word(ppu.t, value);
				ppu.v = ppu.t;
			}
			ppu.w = !ppu.w;
			break;
		case PPUDATA_VECTOR:
			ppu_internal_write(value);
			break;
		
		default:
			printf("$%x Not Handled\n", addr);
			break;
	}
}

byte ppu_read(word addr) {
	byte ret;

	switch (addr) {
		case PPUSTATUS_VECTOR:
			ret = ppu.ppu_status;
			clear_bit(ppu.ppu_status, VBLANK);
			ppu.w = 0;
			break;
		case OAMDATA_VECTOR:
			ret = ppu_internal_read();
			break;
		case PPUDATA_VECTOR:
			ret = ppu_internal_read();
			break;

		default:
			return (0);
	}
	return (ret);
}

void fill_attr_latch(attr_latch *latch, byte attr, byte attr_low_bit) {
	latch->attr_low = check_bit(attr, attr_low_bit);
	latch->attr_high = check_bit(attr, (attr_low_bit + 1));
}

void fill_patt_latch(patt_latch *latch, byte tile, byte row, word patt_table_addr) {
	word patt_addr = get_pattern_addr(patt_table_addr, tile, row);
	latch->patt_low = ppu_mem[patt_addr];
	latch->patt_high = ppu_mem[patt_addr + 8];
}

void fill_shifter_high_bits(pixel_shifter *shifter, byte patt_low, byte patt_high) {
	shifter->low_shift_reg = make_word(patt_low, shifter->low_shift_reg);
	shifter->high_shift_reg = make_word(patt_high, shifter->high_shift_reg);
}

void fill_shifter_low_bits(pixel_shifter *shifter, byte patt_low, byte patt_high) {
	shifter->low_shift_reg |= patt_low;
	shifter->high_shift_reg |= patt_high;
}

void advance_shifter(pixel_shifter *shifter) {
	shifter->low_shift_reg <<= 1;
	shifter->high_shift_reg <<= 1;
}

void render_pattern_table_tile(const int tile_x, const int tile_y, struct display *nes_dp) {
	const size_t addr = (tile_y * 16 + tile_x) * 16;
	const size_t dp_row = tile_x * 8 + 7;
	const size_t dp_height = (tile_y % 16) * 8;

	for (size_t i = 0; i < 8; i++) {
		byte plane1 = ppu_mem[addr + i];
		byte plane2 = ppu_mem[addr + i + 8];
		for (int k = 7; k >= 0; k--) {
			byte pl1bit = check_bit(plane1, k);
			byte pl2bit = check_bit(plane2, k);
			byte pixel = (pl2bit << 1) | pl1bit;
			SDL_FRect rect;
			if (pixel >= 1) {
				rect.h = rect.w = 3;
				rect.x = (dp_row - k) * rect.w;
				rect.y = (i + dp_height) * rect.h;
				if (pixel == 3) {
					SDL_SetRenderDrawColor(nes_dp->renderer, 192, 192, 192, SDL_ALPHA_OPAQUE);
				} else if (pixel == 2) {
					SDL_SetRenderDrawColor(nes_dp->renderer, 255, 222, 173, SDL_ALPHA_OPAQUE);
				} else {
					SDL_SetRenderDrawColor(nes_dp->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
				}
				SDL_RenderFillRect(nes_dp->renderer, &rect);
			}
			printf("%d", pixel);
		}
		printf("\n");
	}
	printf("\n");
}

void render_pattern_table(const int table_nbr, struct display *nes_dp) {
	for (size_t tile_y = 0; tile_y < 16; tile_y++) {
		for (size_t tile_x = 0; tile_x < 16; tile_x++) {
			render_pattern_table_tile(tile_x, tile_y + (table_nbr * 16), nes_dp);
		}
	}
}

byte select_sprite(sp_pixel_data *sp_pixel, byte tile) {
	byte pixel_color_index = 0;

	if (sp_pixel->xcounter != 0) {
		sp_pixel->xcounter -= 1;
		return (pixel_color_index);
	}
	pixel_color_index = merge_plane_bits(
		check_bit(sp_pixel->patt_shifter.high_shift_reg, 7),
		check_bit(sp_pixel->patt_shifter.low_shift_reg, 7)
	);
	advance_shifter(&sp_pixel->patt_shifter);
	return (pixel_color_index);
}

void render_pixel(bg_pixel_data *pixel_data, byte sp_pixel, byte tile_nbr, byte pixel_nbr, bool render_flag) {
	// Getting the pixel_color_index via the low_plane and high_plane bits
	byte pixel_color_index = merge_plane_bits(
		check_bit(pixel_data->patt_shifter.high_shift_reg, 15),
		check_bit(pixel_data->patt_shifter.low_shift_reg, 15)
	);
	byte attr_index = merge_plane_bits(
		check_bit(pixel_data->attr_shifter.high_shift_reg, 15),
		check_bit(pixel_data->attr_shifter.low_shift_reg, 15)
	);
	word palette_addr;

	if (sp_pixel & 0x03) {
		palette_addr = 0x3F10 | sp_pixel;
	} else {
		// Mashing the attributes and pixel_color_index and the addr 0x3F00 to get the palette color of the pixel
		palette_addr = 0x3F00 | attr_index << 2 | pixel_color_index;
	}
	// Get the RGB color from the loaded palette (2C03)
	int pixel = ppu_mem[palette_addr] * 3;

	if (render_flag == true) {
		int index = ((scanline_nbr * 256) + (tile_nbr * 8 + pixel_nbr)) * 4;
		pixels[index] = pallete_rgb[pixel];
		pixels[index + 1] = pallete_rgb[pixel + 1];
		pixels[index + 2] = pallete_rgb[pixel + 2];
		pixels[index + 3] = 255;
	}
	advance_shifter(&pixel_data->patt_shifter);
	advance_shifter(&pixel_data->attr_shifter);
}

void transfer_render_latch_to_bg_pixel_data(render_data_latch *render_dt_latch, bg_pixel_data *pixel_data) {
	fill_shifter_low_bits(&pixel_data->patt_shifter, render_dt_latch->patt_latch.patt_low, render_dt_latch->patt_latch.patt_high);
	fill_shifter_low_bits(&pixel_data->attr_shifter, render_dt_latch->attr_latch.attr_low * 0xFF, render_dt_latch->attr_latch.attr_high * 0xFF);
}

void get_tile_data(render_data_latch *render_dt_latch) {
	word pattern_data_addr = 0x1000; //check_bit(ppu.ppu_ctrl, BG_PATT_ADDR) ? 0x1000 : 0;
	byte attr_side = merge_plane_bits(
		check_bit(get_coarse_y(ppu.v), 1),
		check_bit(get_coarse_x(ppu.v), 1)
	) * 2;

	render_dt_latch->nametable = ppu_mem[get_nametable_addr(ppu.v)];
	fill_attr_latch(&render_dt_latch->attr_latch, ppu_mem[get_attribute_addr(ppu.v)], attr_side);
	fill_patt_latch(&render_dt_latch->patt_latch, render_dt_latch->nametable, get_fine_y(ppu.v), pattern_data_addr);
}

void render_tile(const byte tile, bool render_flag) {
	get_tile_data(&ppu.render_dt_latch);
	for (int pixel_nbr = 0; pixel_nbr < 8; pixel_nbr++) {
		byte sp_pixel = 0;
		for (int i = 0; i < TOTAL_SPRITE_IN_SCANLINE; i++) {
			byte pixel_color = select_sprite(&ppu.sprite_data[i], tile);
			if (pixel_color != (byte)(TRANSPARENT_PIXEL)) {
				sp_pixel = (ppu.sprite_data[i].attr & 0x03) << 2 | pixel_color;
			}
		}
		render_pixel(&ppu.pixel_data, sp_pixel, tile, pixel_nbr, render_flag);
	}
	transfer_render_latch_to_bg_pixel_data(&ppu.render_dt_latch, &ppu.pixel_data);
	ppu.v++;
}

void transfer_oam2_data_to_sp_pixel_data(sp_pixel_data *sp_pixel, byte sp_nbr) {
	word patt_table_addr = check_bit(ppu.ppu_ctrl, SP_PATT_ADDR) ? 0x1000 : 0x1000;
	byte row = scanline_nbr - ppu_oam2_mem[oam_offset(sp_nbr, SP_Y)];
	word patt_addr = get_pattern_addr(
		patt_table_addr,
		ppu_oam2_mem[oam_offset(sp_nbr, SP_TILE)],
		row
	);
	fill_shifter_low_bits(&sp_pixel->patt_shifter, ppu_mem[patt_addr], ppu_mem[patt_addr + 8]);
	sp_pixel->attr = ppu_oam2_mem[oam_offset(sp_nbr, SP_ATTR)];
	sp_pixel->xcounter = ppu_oam2_mem[oam_offset(sp_nbr, SP_X)];
}

void oam_evaluation() {
	byte oam2_addr = 0;
	ppu.oam_addr = 0;
	for (int i = 0; i < TOTAL_SPRITE_NBR; i++) {
		byte y = ppu_oam1_mem[oam_offset(i, 0)];
		int row = scanline_nbr - y;
		if (row <= -1 || row >= 8 || oam2_addr == PPU_OAM2_MEM_SIZE) {
			continue;
		}
		ppu_oam2_mem[oam2_addr++] = y;
		ppu_oam2_mem[oam2_addr++] = ppu_oam1_mem[oam_offset(i, SP_TILE)];
		ppu_oam2_mem[oam2_addr++] = ppu_oam1_mem[oam_offset(i, SP_ATTR)];
		ppu_oam2_mem[oam2_addr++] = ppu_oam1_mem[oam_offset(i, SP_X)];
	}
	for (int i = 0; i < oam2_addr / 4; i++) {
		transfer_oam2_data_to_sp_pixel_data(&ppu.sprite_data[i], i);
	}
}

void scanline() {
	//set_bit_to(ppu.v, 10, check_bit(ppu.t, 10));
	//set_bit_to(ppu.v, 11, check_bit(ppu.t, 11));
	set_bit_to(ppu.v, 10, 0); // For now, always render the 0 (2000-2400) nametable
	set_bit_to(ppu.v, 11, 0);

	for (int tile = 0; tile < SCANLINE_TILE_COUNT; tile++) {
		render_tile(tile, true);
	}
	ppu.v -= 3; // Decrement as it's supposed to finish 31 not 34
	// Reset the coarse_x
	clear_coarse_x(ppu.v);

	// Adavance/Reset the fine_y and coarse_y
	if (get_fine_y(ppu.v) < 7) {
		ppu.v += 0x1000;
	} else {
		clear_fine_y(ppu.v);
		if (get_coarse_y(ppu.v) == 29) {
			clear_coarse_y(ppu.v);
			toggle_vt_nametable(ppu.v);
		} else if (get_coarse_y(ppu.v) == 31) {
			clear_coarse_y(ppu.v);
		} else {
			ppu.v += 0x20;
		}
	}


	// Render next 2 tiles
	for (int i = 0; i < 2; i++) {
		render_tile(i, false);
	}
	// OAM
	memset(ppu_oam2_mem, 0xFF, sizeof(ppu_oam2_mem)); // Init the oam2 to 0xFF
	oam_evaluation();

	toggle_hz_nametable(ppu.v);
	//exit(0);
}

void clock_ppu(struct display *nes_dp, byte cycles) {
	cpu_cycle_counter += cycles;
	if (cpu_cycle_counter < 114) {
		return;
	}
	if (check_bit(ppu.ppu_mask, BG_RND) && scanline_nbr >= 0 && scanline_nbr <= 239) {
		scanline();
	} else if (scanline_nbr == 241) {
		set_bit(ppu.ppu_status, VBLANK);
		if (check_bit(ppu.ppu_status, VBLANK) == 1 && check_bit(ppu.ppu_ctrl, VBLANK) == 1) {
			nmi();
		}
	} else if (scanline_nbr == 260) {
		clear_bit(ppu.ppu_status, VBLANK);
	} else if (scanline_nbr == 261) {
		if (check_bit(ppu.ppu_mask, BG_RND)) {
			ppu.v = 0; // Perhaps the vertical scroll bits are reloaded in this step
			render_tile(0, false);
			render_tile(1, false);
		}
		scanline_nbr = -1;
	}
	scanline_nbr += 1;
	cpu_cycle_counter -= SCANLINE_CPU_CYCLES;
}

