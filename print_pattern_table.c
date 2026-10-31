#include "nes.h"
#include "ppu.h"
#include "utils.h"
#include "MMC1.h"
#include <stdio.h>
#include <stdlib.h>

extern byte ppu_mem[PPU_MEM_SIZE];

void map_patt_table(byte set) {
	for (int i = 0; i < 5; i++) {
		MMC1_write(0xA000, GET_BIT(set, 0));
		set >>= 1;
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
			byte pl1bit = GET_BIT(plane1, k);
			byte pl2bit = GET_BIT(plane2, k);
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
			//printf("%d", pixel);
		}
		//printf("\n");
	}
	//printf("\n");
}

void render_pattern_table(const int table_nbr, struct display *nes_dp) {
	for (size_t tile_y = 0; tile_y < 16; tile_y++) {
		for (size_t tile_x = 0; tile_x < 16; tile_x++) {
			render_pattern_table_tile(tile_x, tile_y + (table_nbr * 16), nes_dp);
		}
	}
}

int main(int argc, char *argv[]) {
	struct display dp;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if (nes_init(&dp, argv[1])) {
		return (1);
	}

	SDL_SetRenderDrawColor(dp.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(dp.renderer);

	map_patt_table(atoi(argv[2]));
	render_pattern_table(0, &dp);

	SDL_RenderPresent(dp.renderer);
	SDL_Delay(5000);

	clean_display(&dp);
	SDL_Quit();
}
