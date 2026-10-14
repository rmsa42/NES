#include "sdl_utils.h"
#include "ppu.h"
#include "rom.h"
#include <stdio.h>

extern byte ppu_mem[PPU_MEM_SIZE];
extern byte pallete_rgb[0x3F * 3];

int win_w = 512;
int win_h = 480;

void fill_nametable(byte value) {
	for (int i = 0x2000; i < 0x23C0; i++) {
		ppu_mem[i] = value;
	}
	printf("Filled Nametable\n");
}

void fill_attributes(byte value) {
	for (int i = 0x23C0; i < 0x2400; i++) {
		ppu_mem[i] = value;
	}
}

void fill_palletes(byte value) {
	for (int i = 0x3F10; i < 0x3F20; i++) {
		ppu_mem[i] = value;
	}
}

int main(int argc, char *argv[]) {
	struct display dp;

	if (create_SDL_display("Rendring_Tests", win_w, win_h, &dp)) {
		return (1);
	}
	if (read_rom(argv[1])) {
		return (1);
	}
	dp.run = true;
	fill_nametable(0x01);
	fill_attributes(0x40);
	fill_palletes(0x2B);
	while (dp.run) {
		SDL_Event e;

		SDL_SetRenderDrawColor(dp.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(dp.renderer);
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_EVENT_QUIT) {
				dp.run = false;
			} else if (e.type == SDL_EVENT_KEY_DOWN) {
				if (e.key.scancode == SDL_SCANCODE_Q) {
					dp.run = false;
				}
			}
		}
		//render_pattern_table(0, &dp);
		render_nametable(&dp);
		SDL_RenderPresent(dp.renderer);
	}
	clean_SDL_display(&dp);
	return (0);
}
