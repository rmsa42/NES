#include "sdl_utils.h"
#include "ppu.h"
#include "rom.h"
#include "nes.h"
#include <stdio.h>

extern struct ppu ppu;
extern byte ppu_mem[PPU_MEM_SIZE];
extern byte pallete_rgb[0x3F * 3];
extern byte pixels[256 * 240 * 4];
extern SDL_Texture *texture;

int win_w = 512;
int win_h = 480;

void fill_nametable(byte value) {
	for (int i = 0x2000; i < 0x23C0; i++) {
		ppu_mem[i] = value;
	}
	ppu_mem[0x2000] = 0x1D;
	printf("Filled Nametable\n");
}

void fill_attributes(byte value) {
	for (int i = 0x23C0; i < 0x2400; i++) {
		ppu_mem[i] = value;
	}
}

void fill_palletes(byte value) {
	for (int i = 0x3F00; i < 0x3F20; i++) {
		ppu_mem[i] = value;
	}
	ppu_mem[0x3F01] = 0x20;
}

//int main(int argc, char *argv[]) {
//	struct display dp;
//
//	if (create_SDL_display("Rendring_Tests", win_w, win_h, &dp)) {
//		return (1);
//	}
//	if (read_rom(argv[1])) {
//		return (1);
//	}
//	dp.run = true;
//	fill_nametable(0x01);
//	fill_attributes(0x40);
//	fill_palletes(0x2B);
//	while (dp.run) {
//		SDL_Event e;
//
//		SDL_SetRenderDrawColor(dp.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
//		SDL_RenderClear(dp.renderer);
//		while (SDL_PollEvent(&e)) {
//			if (e.type == SDL_EVENT_QUIT) {
//				dp.run = false;
//			} else if (e.type == SDL_EVENT_KEY_DOWN) {
//				if (e.key.scancode == SDL_SCANCODE_Q) {
//					dp.run = false;
//				}
//			}
//		}
//		//render_pattern_table(0, &dp);
//		render_nametable(&dp);
//		SDL_RenderPresent(dp.renderer);
//	}
//	clean_SDL_display(&dp);
//	return (0);
//}

int main() {
	struct display dp;
	pixel_data data;

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		printf("SDL Init error\n");
		return (1);
	}
	nes_init(&dp, "../roms/TetrisNes/Tetris.nes");
	SDL_SetRenderDrawColor(dp.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(dp.renderer);
	fill_nametable(0x01);
	fill_attributes(0x40);
	fill_palletes(0x0F);
	set_bit(ppu.ppu_mask, BG_RND);

	data.highp_shift_reg = 0;
	data.lowp_shift_reg = 0;
	data.attr_latch = 0x40 & 0x03;
	for (int i = 0; i < 239; i++) {
		scanline();
	}
	SDL_UpdateTexture(texture, NULL, (void *)pixels, 256 * 4);
	SDL_RenderTexture(dp.renderer, texture, NULL, NULL);
	SDL_RenderPresent(dp.renderer);
	SDL_Delay(3000);
	clean_SDL_display(&dp);
	SDL_Quit();
}
