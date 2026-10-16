#include "ppu.h"
#include "cpu.h"
#include "rom.h"
#include <string.h>
#include <stdio.h>

extern int cpu_cycles;
extern byte pallete_rgb[PALLETE_SIZE];
extern SDL_Texture *texture;

struct ppu ppu;
byte ppu_mem[PPU_MEM_SIZE];
int scanline_g = 0;
int counter = 0;
byte pixels[256 * 240 * 4];

void ppu_init() {
	memset(&ppu, 0, sizeof(struct ppu));
	memset(ppu_mem, 0, sizeof(ppu_mem));
}

void ppu_internal_write(byte value) {
	ppu_mem[ppu.v] = value;
	ppu.v += check_bit(ppu.ppu_ctrl, 2) ? 32 : 1;
}

byte ppu_internal_read() {
	byte ret = ppu_mem[ppu.v];

	ppu.v += check_bit(ppu.ppu_ctrl, 2) ? 32 : 1;
	return (ret);
}

void ppu_write(word addr, byte value) {
	switch (addr) {
		case PPUCTRL_VECTOR:
			ppu.ppu_ctrl = value;
			set_bit_to(ppu.t, 10, check_bit(ppu.ppu_ctrl, 0));
			set_bit_to(ppu.t, 11, check_bit(ppu.ppu_ctrl, 1));
			printf("$2000: %x\n", ppu.ppu_ctrl);
			//printf("Increment: %x\n", check_bit(ppu.ppu_ctrl, 2));
			break;
		case PPUMASK_VECTOR:
			ppu.ppu_mask = value;
			printf("$2001: %x\n", ppu.ppu_mask);
			printf("Rendering Background: %x\n", check_bit(ppu.ppu_mask, BG_RND));
			break;
		case OAMADDR_VECTOR:
			ppu.oam_addr = value;
			printf("$2003: %x\n", ppu.oam_addr);
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
				ppu.t = (ppu.t << 8) | value;
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
			clear_bit(ppu.ppu_status, 7);
			ppu.w = 0;
			break;
		case PPUDATA_VECTOR:
			ret = ppu_internal_read();
			break;

		default:
			return (0);
	}
	return (ret);
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

void render_pixel_sliver(const byte tile, const byte attr, const byte low_plane, const byte high_plane, const byte scanline, const byte x, struct display *nes_dp) {
	word pli_addr = 0x3F00;

	for (int k = 0; k < 8; k++) {
		byte pl1bit = check_bit(low_plane, (7 - k));
		byte pl2bit = check_bit(high_plane, (7 - k));
		byte pixel = (pl2bit << 1) | pl1bit;
		SDL_FRect rect;
		//if (pixel >= 1) {
			pli_addr = pli_addr | (attr & 0x03) << 2 | pixel;
			word pli = ppu_mem[pli_addr] * 3;
			//printf("Pli_Attr: %x\n", pli);
			rect.h = rect.w = 2;
			rect.x = (x + k) * rect.w;
			rect.y = scanline * rect.h;
			//printf("Rect: %f %f\n", rect.x, rect.y);
			printf("Pli: %x\n", pli);
			SDL_SetRenderDrawColor(nes_dp->renderer, pallete_rgb[pli], pallete_rgb[pli + 1], pallete_rgb[pli + 2], SDL_ALPHA_OPAQUE);
			SDL_RenderFillRect(nes_dp->renderer, &rect);
		//}
		//printf("%d", pixel);
	}
	//printf("\n");
}

void render_scanline(struct display *nes_dp, const byte scanline) {
	for (byte i = 0; i < 32; i++) {
		byte tile_id = ppu_mem[get_nametable_addr(ppu.v)]; // 1 Fetch
		byte attr = ppu_mem[get_attribute_addr(ppu.v)]; // 2 Fetch

		word pattern_data_addr = tile_id * 16 + get_fine_y(ppu.v);
		byte low_plane = ppu_mem[pattern_data_addr]; // 3 Fetch
		byte high_plane = ppu_mem[pattern_data_addr + 8]; // 4 Fetch
		render_pixel_sliver(tile_id, attr, low_plane, high_plane, scanline, i * 8, nes_dp);
		ppu.pixel_data.lowp_shift_reg = make_word(low_plane, ppu.pixel_data.lowp_shift_reg);
		ppu.pixel_data.highp_shift_reg = make_word(high_plane, ppu.pixel_data.highp_shift_reg);
		ppu.pixel_data.attr_latch = attr & 0x03; // Currently getting the top left color
		if (get_coarse_x(ppu.v) == 31) {
			clear_coarse_x(ppu.v);
			toggle_hz_nametable(ppu.v);
		} else {
			ppu.v++;
		}
		//printf("VRAM: %x\n", ppu.v);
	}
}

void render_nametable(struct display *nes_dp) {
	ppu.v = ppu.t;
	for (size_t scanline = 0; scanline < 240; scanline++) {
		set_coarse_x(ppu.v, get_coarse_x(ppu.t));
		set_bit_to(ppu.v, 10, check_bit(ppu.t, 10));
		render_scanline(nes_dp, scanline);
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
	}
	//printf("VRAM: %x\n", vram_addr);
}

void render_pixel(pixel_data *pixel_data, byte tile_nbr, byte pixel_nbr) {
	byte low_plane_bit = check_bit(pixel_data->lowp_shift_reg, 0);
	byte high_plane_bit = check_bit(pixel_data->highp_shift_reg, 0);
	//printf("Tile_low: %x\n", pixel_data->lowp_shift_reg);
	//printf("Tile_high: %x\n", pixel_data->highp_shift_reg);
	byte pixel_color_index = (high_plane_bit << 1) | low_plane_bit;
	word palette_addr = 0x3F00 | pixel_data->attr_latch << 2 | pixel_color_index;
	//printf("Pixel_C_I: %d\n", pixel_color_index);
	//printf("Attr: %x\n", pixel_data->attr_latch);
	//printf("Color: %x\n", palette_addr);
	int pixel = ppu_mem[palette_addr] * 3;
	int index = ((scanline_g * 256) + (tile_nbr * 8) + pixel_nbr) * 4;

	pixels[index] = pallete_rgb[pixel];
	pixels[index + 1] = pallete_rgb[pixel + 1];
	pixels[index + 2] = pallete_rgb[pixel + 2];
	pixels[index + 3] = 255;
	//printf("Index: %d\n", index);
	pixel_data->lowp_shift_reg >>= 1;
	pixel_data->highp_shift_reg >>= 1;
}

void transfer_addr_latch_to_pixel_data(addr_latch *addr_latch, pixel_data *pixel_data) {
	pixel_data->lowp_shift_reg = make_word(addr_latch->tile_low, pixel_data->lowp_shift_reg);
	pixel_data->highp_shift_reg = make_word(addr_latch->tile_high, pixel_data->highp_shift_reg);
	//printf("Tile_low: %x\n", pixel_data->lowp_shift_reg);
	//printf("Tile_high: %x\n", pixel_data->highp_shift_reg);
	pixel_data->attr_latch = addr_latch->attr & 0x03;
}

void get_tile_data(addr_latch *addr_latch) {
	word pattern_data_addr;

	addr_latch->nametable = ppu_mem[get_nametable_addr(ppu.v)]; // 1 Fetch
	addr_latch->attr = ppu_mem[get_attribute_addr(ppu.v)]; // 2 Fetch
	pattern_data_addr = addr_latch->nametable * 16 + get_fine_y(ppu.v);
	addr_latch->tile_low = ppu_mem[pattern_data_addr]; // 3 Fetch
	addr_latch->tile_high = ppu_mem[pattern_data_addr + 8]; // 4 Fetch
}

void scanline() {
	if (!check_bit(ppu.ppu_mask, BG_RND)) {
		scanline_g += 1;
		return;
	}
	set_coarse_x(ppu.v, get_coarse_x(ppu.t));
	set_bit_to(ppu.v, 10, check_bit(ppu.t, 10));
	for (int i = 0; i < 32; i++) {
		get_tile_data(&ppu.addr_latch); // 8 PPU Cycles
		for (int k = 0; k < 8; k++) {
			render_pixel(&ppu.pixel_data, i, k);
		}
		transfer_addr_latch_to_pixel_data(&ppu.addr_latch, &ppu.pixel_data);
		if (get_coarse_x(ppu.v) == 31) {
			clear_coarse_x(ppu.v);
			toggle_hz_nametable(ppu.v);
		} else {
			ppu.v++;
		}
	}
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
	scanline_g += 1;
}

void clock_ppu(struct display *nes_dp, byte cycles) {
	//SDL_SetRenderScale(nes_dp->renderer, 1, 1);
	//render_pattern_table(0, nes_dp);
	counter += cycles;
	if (scanline_g == 240 && !check_bit(ppu.ppu_status, 7)) {
		set_bit(ppu.ppu_status, 7);
		printf("Flag Check: %x %x\n", check_bit(ppu.ppu_status, 7), check_bit(ppu.ppu_ctrl, 7));
		if (check_bit(ppu.ppu_status, 7) == 1 && check_bit(ppu.ppu_ctrl, 7) == 1) {
			nmi();
		}
	} else if (scanline_g == 260) {
		clear_bit(ppu.ppu_status, 7);
		scanline_g = 0;
		counter = 0;
	} else if (!check_bit(ppu.ppu_status, 7) && counter >= 113) {
		scanline();
		counter = 0;
	} else if (counter >= 113) {
		scanline_g += 1;
		counter = 0;
	}
	//if (check_bit(ppu.ppu_mask, BG_RND)) {
	//	ppu.t = 0;
	//	render_nametable(nes_dp);
	//}
}

