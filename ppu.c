#include "ppu.h"
#include <string.h>
#include <stdio.h>

struct ppu ppu;
byte ppu_mem[PPU_MEM_SIZE];

void ppu_init() {
	memset(&ppu, 0, sizeof(struct ppu));
	memset(ppu_mem, 0, sizeof(ppu_mem));
}

void clock_ppu(struct display *nes_dp) {
	SDL_SetRenderScale(nes_dp->renderer, 1, 1);
	int padding_w = 7;
	int padding_h = 0;
	int ct = 0;
	for (size_t l = 0x00; l < 0x1000; l += 0x10) {
		int h = 0;
		for (size_t k = 0; k < 8; k++) {
			byte plane1 = ppu_mem[l + k];
			byte plane2 = ppu_mem[l + k + 8];
			for (int i = 7; i >= 0; i--) {
				byte pl1bit = check_bit(plane1, i);
				byte pl2bit = check_bit(plane2, i);
				byte pixel = (pl2bit << 1) | pl1bit;
				SDL_FRect rect;
				if (pixel >= 1) {
					rect.h = rect.w = 3;
					rect.x = (padding_w - i) * rect.w;
					rect.y = (h + padding_h) * rect.h;
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
			printf("\t%x %x", l + k, l + k + 8);
			h++;
			printf("\n");
		}
		padding_w += 8;
		ct++;
		printf("\n");
		if (ct == 16) {
			ct = 0;
			padding_h += 8;
			padding_w = 7;
		}
	}
}

