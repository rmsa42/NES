#include "nes.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "rom.h"
#include "controller.h"
#include "MMC1.h"
#include <stdio.h>

extern Controller joy1;

int window_width = 512;
int window_height = 480;
Texture *screen;

int nes_init(Display *nes_dp, const char *rom_name) {
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
	screen = create_texture(nes_dp, 256, 240);
	cpu_start();
	printf("Nes Init\n");
	nes_dp->run = true;
	return (0);

}

void update_nes(Display *nes_dp, int curr_time, int *last_time) {
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
		render_texture(nes_dp, screen);
		SDL_RenderPresent(nes_dp->renderer);
		*last_time = curr_time;
	}
}

void nes_handle_event(Display *nes_dp, SDL_Event *e) {
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
