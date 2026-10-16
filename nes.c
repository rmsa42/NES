#include "nes.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "rom.h"
#include <stdio.h>

extern int scanline_g;
extern byte pixels[256 * 240 * 4];

int window_width = 512;
int window_height = 480;
SDL_Texture *texture;

int nes_init(struct display *nes_dp, const char *rom_name) {
	if (create_SDL_display("NES", window_width, window_height, nes_dp)) {
		return (1);
	}
	SDL_SetWindowPosition(nes_dp->window, 1000, 300);
	cpu_init();
	ppu_init();
	if (read_rom(rom_name)) {
		printf("Invalid ROM\n");
		return (1);
	}
	texture = SDL_CreateTexture(nes_dp->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 256, 240);
	cpu_start();
	printf("Nes Init\n");
	nes_dp->run = true;
	return (0);

}

void update_nes(struct display *nes_dp) {
	byte cycles = 0;

	SDL_UpdateTexture(texture, NULL, (void *)pixels, 256 * 4);
	SDL_SetRenderDrawColor(nes_dp->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(nes_dp->renderer);
	cycles = clock_cpu();
	clock_ppu(nes_dp, cycles);
	//clock_apu(stream);
	SDL_RenderTexture(nes_dp->renderer, texture, NULL, NULL);
	SDL_RenderPresent(nes_dp->renderer);
}

void nes_handle_event(struct display *nes_dp, SDL_Event *e) {
	if (e->type == SDL_EVENT_KEY_DOWN) {
		if (e->key.scancode == SDL_SCANCODE_Q) {
			nes_dp->run = false;
		}
	}
}
