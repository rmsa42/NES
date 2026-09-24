#include "cpu.h"
#include "apu.h"
#include "ppu.h"
#include "rom.h"
#include "debug_tools.h"
#include "sdl_utils.h"
#include <stdio.h>
#include <stdbool.h>

extern inst insts[0xFF];
extern byte memory[0xFFFF];
extern struct cpu cpu;


SDL_AudioStream *stream;
int window_width = 640;
int window_height = 480;

void update_nes(struct display *nes_dp) {
	SDL_Event e;

	SDL_SetRenderDrawColor(nes_dp->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(nes_dp->renderer);
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_EVENT_QUIT) {
			nes_dp->run = false;
		} else if (e.type == SDL_EVENT_KEY_DOWN) {
			if (e.key.scancode == SDL_SCANCODE_Q) {
				nes_dp->run = false;
			}
		}
	}
	clock_cpu();
	clock_ppu(nes_dp);
	//clock_apu(stream);
	SDL_RenderPresent(nes_dp->renderer);
}

int nes_init(const char *rom, struct display *nes_dp) {
	if (create_SDL_display("NES", window_width, window_height, nes_dp)) {
		return (1);
	}
	cpu_init();
	ppu_init();
	read_rom(rom);
	nes_dp->run = true;
	cpu.pc = 0x8000; // This is here for now. The prg-rom is loaded in the 0x8000 addr
	return (0);
}

int aux_tools_init(const char *bp, struct display *debug_dp) {
	SDL_AudioSpec spec;

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
	nes_init(argv[1], &nes_dp);

	printf("Running CPU\n");
	while (nes_dp.run && debug_dp.run) {
		while (debug_dp.run) {
			if (update_debugger(&debug_dp)) {
				break;
			}
		}
		update_nes(&nes_dp);
	}

	clean_SDL_display(&debug_dp);
	clean_SDL_display(&nes_dp);
	//SDL_DestroyAudioStream(stream);
	SDL_Quit();
}
