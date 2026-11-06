#include "nes.h"
#include "debug_tools.h"
#include <stdio.h>

int window_width = 512;
int window_height = 480;

Display nes_dp;
Texture *screen;

int nes_init(const char *rom_name) {
	if (create_display("NES", window_width, window_height, &nes_dp)) {
		return (1);
	}
	SDL_SetWindowPosition(nes_dp.window, 1000, 300);
	cpu_init();
	ppu_init();
	MMC1_init();
	if (read_rom(rom_name)) {
		printf("Invalid ROM\n");
		return (1);
	}
	screen = create_texture(&nes_dp, 256, 240);
	cpu_start(); // Temporary until i stop using "cpu_mem[]" inside "read_rom()"
	printf("Nes Init\n");
	nes_dp.run = true;
	return (0);
}

void handle_events(Display *nes_dp, Display *debugger_dp) {
	SDL_Event e;

	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_EVENT_QUIT) {
			nes_dp->run = false;
			debugger_dp->run = false;
		}
		SDL_Window *ewindow = SDL_GetWindowFromEvent(&e);
		if (ewindow == nes_dp->window) {
			nes_handle_event(nes_dp, &e);
		} else if (ewindow == debugger_dp->window) {
			debugger_handle_event(debugger_dp, &e);
		}
	}
}

void nes_run() {
	int curr_time;
	int last_time = 0;

	while (nes_dp.run) {
		curr_time = SDL_GetTicks();
		handle_events(&nes_dp, debugger_get_display());
		debugger_update();
		if (debug_block()) {
			nes_update(curr_time, &last_time);
		}
	}
	nes_quit();
	debugger_quit();
}

void nes_update(int curr_time, int *last_time) {
	byte cycles = 0;
	bool render = curr_time > *last_time + 16;

	if (render) {
		SDL_SetRenderDrawColor(nes_dp.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(nes_dp.renderer);
	}
	cycles = clock_cpu();
	clock_ppu(cycles);
	//clock_apu(nes_dp);
	if (render) {
		render_texture(&nes_dp, screen);
		SDL_RenderPresent(nes_dp.renderer);
		*last_time = curr_time;
	}
}

Display *nes_get_display() { return (&nes_dp); }

void nes_quit() {
	free_rom();
	destroy_texture(screen);
	clean_display(&nes_dp);
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
