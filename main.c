#include "nes.h"
#include "sdl_utils.h"
#include "debug_tools.h"
#include <stdio.h>
#include <stdbool.h>

extern struct cpu cpu;

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

	clean_display(&debug_dp);
	clean_display(&nes_dp);
	//SDL_DestroyAudioStream(stream);
	SDL_Quit();
}

