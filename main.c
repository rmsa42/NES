#include "cpu.h"
#include "apu.h"
#include "ppu.h"
#include "rom.h"
#include "debug_tools.h"
#include "sdl_utils.h"
#include <stdio.h>
#include <stdbool.h>

extern struct cpu cpu;

int window_width = 512;
int window_height = 480;
extern bool stop;

void nes_handle_event(struct display *nes_dp, SDL_Event *e) {
	if (e->type == SDL_EVENT_KEY_DOWN) {
		if (e->key.scancode == SDL_SCANCODE_Q) {
			nes_dp->run = false;
		}
	}
}

void handle_events(struct display *nes_dp, struct display *debugger_dp) {
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

void update_nes(struct display *nes_dp) {
	SDL_SetRenderDrawColor(nes_dp->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	if (stop == true) {
		SDL_RenderClear(nes_dp->renderer);
	}
	clock_cpu();
	clock_ppu(nes_dp);
	//clock_apu(stream);
	SDL_RenderPresent(nes_dp->renderer);
}

int nes_init(struct display *nes_dp) {
	if (create_SDL_display("NES", window_width, window_height, nes_dp)) {
		return (1);
	}
	SDL_SetWindowPosition(nes_dp->window, 1000, 300);
	cpu_init();
	ppu_init();
	nes_dp->run = true;
	cpu.pc = 0x8000; // This is here for now. The prg-rom is loaded in the 0x8000 addr
	printf("Nes Init\n");
	return (0);
}

int aux_tools_init(const char *bp, struct display *debug_dp) {
	//SDL_AudioSpec spec;

	// Need to make this start in the display function
	//spec.channels = 1;
	//spec.format = SDL_AUDIO_F32;
	//spec.freq = SAMPLE_RATE;
	//stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
	//if (!stream) {
	//	printf("SDL Audio Stream error\n");
	//	return (1);
	//}
	//SDL_ResumeAudioStreamDevice(stream);

	if (debugger_init(bp, debug_dp)) {
		return (1);
	}
	debug_dp->run = true;
	printf("Debugger Init\n");
	return (0);
}

int main(int argc, char *argv[]) {
	struct display debug_dp;
	struct display nes_dp;

	memset(&debug_dp, 0, sizeof(struct display));
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		printf("SDL Init error\n");
		return (1);
	}

	if (aux_tools_init(argv[2], &debug_dp)) {
		return (1);
	}
	nes_init(&nes_dp);
	if (read_rom(argv[1])) {
		printf("Invalid ROM\n");
		nes_dp.run = false; // Just to make the loop stop
	}

	while (nes_dp.run && debug_dp.run) {
		handle_events(&nes_dp, &debug_dp);
		if (update_debugger(&debug_dp)) {
			update_nes(&nes_dp);
		}
	}

	clean_SDL_display(&debug_dp);
	clean_SDL_display(&nes_dp);
	//SDL_DestroyAudioStream(stream);
	SDL_Quit();
}
