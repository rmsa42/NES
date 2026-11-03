#include "nes.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "rom.h"
#include "controller.h"
#include "MMC1.h"
#include <stdio.h>

extern byte pixels[256 * 240 * 4];
extern struct controller joy1;

int window_width = 512;
int window_height = 480;
SDL_Texture *texture;

int nes_init(struct Display *nes_dp, const char *rom_name) {
	if (create_display("NES", window_width, window_height, nes_dp)) {
		return (1);
	}
	SDL_SetWindowPosition(nes_dp->window, 1000, 300);
	cpu_init();
	ppu_init();
	MMC1_init();
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

void update_nes(struct Display *nes_dp, int curr_time, int *last_time) {
	byte cycles = 0;
	bool render = curr_time > *last_time + 16;

	if (render) {
		SDL_SetRenderDrawColor(nes_dp->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(nes_dp->renderer);
	}
	cycles = clock_cpu();
	clock_ppu(nes_dp, cycles);
	//clock_apu(nes_dp);
	if (render) {
		SDL_UpdateTexture(texture, NULL, (void *)pixels, 256 * 4);
		SDL_RenderTexture(nes_dp->renderer, texture, NULL, NULL);
		SDL_RenderPresent(nes_dp->renderer);
		*last_time = curr_time;
	}
}

void nes_handle_event(struct Display *nes_dp, SDL_Event *e) {
	if (e->type == SDL_EVENT_KEY_DOWN) {
		if (e->key.scancode == SDL_SCANCODE_Q) {
			nes_dp->run = false;
		} else if (e->key.scancode == SDL_SCANCODE_RETURN) {
			set_btn(START, 1);
		} else if (e->key.scancode == SDL_SCANCODE_BACKSPACE) {
			set_btn(SELECT, 1);
		} else if (e->key.scancode == SDL_SCANCODE_UP) {
			set_btn(UP, 1);
		} else if (e->key.scancode == SDL_SCANCODE_LEFT) {
			set_btn(LEFT, 1);
		} else if (e->key.scancode == SDL_SCANCODE_DOWN) {
			set_btn(DOWN, 1);
		} else if (e->key.scancode == SDL_SCANCODE_RIGHT) {
			set_btn(RIGHT, 1);
		} else if (e->key.scancode == SDL_SCANCODE_X) {
			set_btn(A, 1);
		} else if (e->key.scancode == SDL_SCANCODE_Z) {
			set_btn(B, 1);
		}
	} else if (e->type == SDL_EVENT_KEY_UP) {
		if (e->key.scancode == SDL_SCANCODE_RETURN) {
			set_btn(START, 0);
		} else if (e->key.scancode == SDL_SCANCODE_BACKSPACE) {
			set_btn(SELECT, 0);
		} else if (e->key.scancode == SDL_SCANCODE_UP) {
			set_btn(UP, 0);
		} else if (e->key.scancode == SDL_SCANCODE_LEFT) {
			set_btn(LEFT, 0);
		} else if (e->key.scancode == SDL_SCANCODE_DOWN) {
			set_btn(DOWN, 0);
		} else if (e->key.scancode == SDL_SCANCODE_RIGHT) {
			set_btn(RIGHT, 0);
		} else if (e->key.scancode == SDL_SCANCODE_X) {
			set_btn(A, 0);
		} else if (e->key.scancode == SDL_SCANCODE_Z) {
			set_btn(B, 0);
		}
	}
}
