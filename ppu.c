#include "ppu.h"
#include "cpu.h"
#include "rom.h"
#include <string.h>
#include <stdio.h>

extern byte pallete_rgb[PALLETE_SIZE];

struct ppu ppu;
byte ppu_mem[PPU_MEM_SIZE];
int scanline_nbr = 0;
float cpu_cycle_counter = 0;
byte pixels[256 * 240 * 4];

void ppu_init() {
	memset(&ppu, 0, sizeof(struct ppu));
	memset(ppu_mem, 0, sizeof(ppu_mem));
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

void render_pixel(pixel_data *pixel_data, byte tile_nbr, byte pixel_nbr, bool render_flag) {
	// Getting the pixel_color_index via the low_plane and high_plane bits
	byte low_plane_bit = check_bit(pixel_data->lowp_shift_reg, 0);
	byte high_plane_bit = check_bit(pixel_data->highp_shift_reg, 0);
	byte pixel_color_index = (high_plane_bit << 1) | low_plane_bit;

	// Mashing the attributes and pixel_color_index and the addr 0x3F00 to get the palette color of the pixel
	word palette_addr = 0x3F00 | pixel_data->attr_latch << 2 | pixel_color_index;
	// Get the RGB color from the loaded palette (2C03)
	int pixel = ppu_mem[palette_addr] * 3;

	if (render_flag == true) {
		int index = ((scanline_nbr * 256) + (tile_nbr * 8) + (7 - pixel_nbr)) * 4;
		if (index < 0) {
			index = -index;
		}

		pixels[index] = pallete_rgb[pixel];
		pixels[index + 1] = pallete_rgb[pixel + 1];
		pixels[index + 2] = pallete_rgb[pixel + 2];
		pixels[index + 3] = 255;
	}
	pixel_data->lowp_shift_reg >>= 1;
	pixel_data->highp_shift_reg >>= 1;
}

void transfer_addr_latch_to_pixel_data(render_data_latch *render_dt_latch, pixel_data *pixel_data) {
	pixel_data->lowp_shift_reg = make_word(render_dt_latch->tile_low, pixel_data->lowp_shift_reg);
	pixel_data->highp_shift_reg = make_word(render_dt_latch->tile_high, pixel_data->highp_shift_reg);
	pixel_data->attr_latch = render_dt_latch->attr & 0x03;
}

void get_tile_data(render_data_latch *render_dt_latch) {
	word pattern_data_addr;

	render_dt_latch->nametable = ppu_mem[get_nametable_addr(ppu.v)]; // 1 Fetch
	render_dt_latch->attr = ppu_mem[get_attribute_addr(ppu.v)]; // 2 Fetch
	pattern_data_addr = render_dt_latch->nametable * 16 + get_fine_y(ppu.v);
	render_dt_latch->tile_low = ppu_mem[pattern_data_addr]; // 3 Fetch
	render_dt_latch->tile_high = ppu_mem[pattern_data_addr + 8]; // 4 Fetch
}

void render_tile(const byte tile, bool render_flag) {
	get_tile_data(&ppu.render_dt_latch);
	for (int pixel_nbr = 0; pixel_nbr < 8; pixel_nbr++) {
		render_pixel(&ppu.pixel_data, tile, pixel_nbr, render_flag);
	}
	transfer_addr_latch_to_pixel_data(&ppu.render_dt_latch, &ppu.pixel_data);
	ppu.v++;
}

void scanline() {
	set_bit_to(ppu.v, 10, check_bit(ppu.t, 10));
	for (int i = 0; i < SCANLINE_TILE_COUNT; i++) {
		render_tile(i, true);
	}
	ppu.v -= 3; // Decrement as it's supposed to finish 31 not 32
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

	toggle_hz_nametable(ppu.v);
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

