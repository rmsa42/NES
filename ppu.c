#include "ppu.h"
#include <string.h>
#include <stdio.h>

struct ppu ppu;
byte ppu_mem[PPU_MEM_SIZE];

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
			ppu.ppu_scroll = value;
			break;
		case PPUADDR_VECTOR:
			if (!ppu.w) {
				ppu.v = value;
			} else {
				ppu.v = (ppu.v << 8) | value;
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
	switch (addr) {
		case PPUSTATUS_VECTOR:
			return (ppu.ppu_status);
		case PPUDATA_VECTOR:
			return (ppu_internal_read());

		default:
			return (0);
	}
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

void render_pixel_sliver(const byte tile, const byte row, const byte scanline, const byte x, struct display *nes_dp) {
	const word addr = (tile * 16) + row;
	byte plane1 = ppu_mem[addr];
	byte plane2 = ppu_mem[addr + 8];

	for (int k = 0; k < 8; k++) {
		byte pl1bit = check_bit(plane1, (7 - k));
		byte pl2bit = check_bit(plane2, (7 - k));
		byte pixel = (pl2bit << 1) | pl1bit;
		SDL_FRect rect;
		if (pixel >= 1) {
			rect.h = rect.w = 2;
			rect.x = (x + k) * rect.w;
			rect.y = scanline * rect.h;
			if (pixel == 3) {
				SDL_SetRenderDrawColor(nes_dp->renderer, 192, 192, 192, SDL_ALPHA_OPAQUE);
			} else if (pixel == 2) {
				SDL_SetRenderDrawColor(nes_dp->renderer, 255, 222, 173, SDL_ALPHA_OPAQUE);
			} else {
				SDL_SetRenderDrawColor(nes_dp->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
			}
			SDL_RenderFillRect(nes_dp->renderer, &rect);
		}
		//printf("%d", pixel);
	}
	//printf("\n");
}

void render_nametable(struct display *nes_dp) {
	word start = 0x2000;
	word vram_addr = 0x2000;

	for (size_t scanline = 0; scanline < 240; scanline++) {
		if (!(scanline % 8) && scanline != 0) {
			start += 0x0020;
		}
		vram_addr = start;
		for (size_t i = 0; i < 32; i++) {
			byte tile = ppu_mem[vram_addr];
			render_pixel_sliver(tile, scanline % 8, scanline, i * 8, nes_dp);
			vram_addr++;
		}
	}
	//printf("VRAM: %x\n", vram_addr);
}

void clock_ppu(struct display *nes_dp) {
	SDL_SetRenderScale(nes_dp->renderer, 1, 1);
	//render_nametable();
	//for (int i = 0; i < 8; i++) {
	//	render_pixel_sliver(0x1D, i % 8, i, 100, nes_dp);
	//}
	//render_pattern_table(0, nes_dp);
	//printf("2006: %x\n", ppu.v);
	if (check_bit(ppu.ppu_mask, BG_RND)) {
		render_nametable(nes_dp);
	}
}

