#include "cpu.h"
#include "apu.h"
#include "rom.h"
#include "debug_tools.h"
#include "SDL3/SDL.h"
#include <stdio.h>
#include <stdbool.h>

extern inst insts[0xFF];
extern byte memory[0xFFFF];
extern struct cpu *cpu;

SDL_Renderer *renderer;
SDL_Window *window;
SDL_AudioStream *stream;
int window_width = 640;
int window_height = 480;
bool nes_run = false;
bool advance = false;

int init_SDL() {
	SDL_AudioSpec spec;

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		printf("SDL Init error\n");
		return (1);
	}
	if (!SDL_CreateWindowAndRenderer("DebugNes", window_width, window_height, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
		printf("SDL Window and Renderer error\n");
		return (1);
	}
	SDL_SetRenderLogicalPresentation(renderer, window_width, window_height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	spec.channels = 1;
	spec.format = SDL_AUDIO_F32;
	spec.freq = SAMPLE_RATE;
	stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
	if (!stream) {
		printf("SDL Audio Stream error\n");
		return (1);
	}
	SDL_ResumeAudioStreamDevice(stream);
	return (0);
}

void clean_SDL() {
	SDL_DestroyAudioStream(stream);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void update_SDL() {
	SDL_Event e;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_EVENT_QUIT) {
			nes_run = false;
		} else if (e.type == SDL_EVENT_KEY_DOWN) {
			if (e.key.scancode == SDL_SCANCODE_Q) {
				nes_run = false;
			}
			if (e.key.scancode == SDL_SCANCODE_RETURN) {
				advance = true;
			}
		}
	}
	cpu_debug();
	SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
	int res;

	res = cpu_instance();
	if (res) {
		printf("Error Init CPU\n");
		return (res);
	}
	if (init_SDL()) {
		return (1);
	}

	int i = 0;
	//memory[i++] = LDA_IMM;
	//memory[i++] = 0xFD;
	//memory[i++] = BNE_RLT;
	//memory[i++] = 0xFE;
	//memory[i++] = 0x40;
	//memory[i++] = LDA_IMM;
	//memory[i++] = 0x00;
	//memory[i++] = STA_ABS;
	//memory[i++] = 0x03;
	//memory[i++] = 0x40;
	//memory[i++] = LDA_IMM;
	//memory[i++] = 0x80;
	//memory[i++] = STA_ABS;
	//memory[i++] = 0x00;
	//memory[i++] = 0x40;

	printf("Running CPU\n");
	read_rom(argv[1]);
	nes_run = true;
	cpu->pc = 0x8000; // This is here for now. The prg-rom is loaded in the 0x8000 addr
	while (nes_run) {
		while (advance == false) {
			update_SDL();
		}
		clock_cpu();
		//clock_apu(stream);

		update_SDL();
		advance = false;
	}

	clean_SDL();
}
