#include "nes.h"
#include "sdl_utils.h"
#include "debug_tools.h"
#include <stdio.h>
#include <stdbool.h>

extern struct cpu cpu;
extern byte *chr_rom;

void handle_events(struct Display *nes_dp, struct Display *debugger_dp) {
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

int aux_tools_init(const char *bp, struct Display *debug_dp) {
	if (debugger_init(bp, debug_dp)) {
		return (1);
	}
	debug_dp->run = true;
	printf("Debugger Init\n");
	return (0);
}

int main(int argc, char *argv[]) {
	struct Display debug_dp;
	struct Display nes_dp;

	memset(&debug_dp, 0, sizeof(struct Display));
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		printf("SDL Init error\n");
		return (1);
	}

	if (aux_tools_init(argv[2], &debug_dp)) {
		return (1);
	}
	if (nes_init(&nes_dp, argv[1])) {
		return (1);
	}

	int last_time = 0;
	int curr_time;
	while (nes_dp.run) {
		curr_time = SDL_GetTicks();
		handle_events(&nes_dp, &debug_dp);
		update_debugger(&debug_dp);
		if (debug_block()) {
			update_nes(&nes_dp, curr_time, &last_time);
		}
	}

	free(chr_rom);
	chr_rom = NULL;
	clean_display(&debug_dp);
	clean_display(&nes_dp);
	SDL_Quit();
}

