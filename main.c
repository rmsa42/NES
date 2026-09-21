#include "cpu.h"
#include "SDL3/SDL.h"
#include "apu.h"
#include "rom.h"
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

int clean_SDL() {
	SDL_DestroyAudioStream(stream);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return (0);
}

void cpu_debug_SDL() {
	char buff[17][50] = {0};
	int i = 0;

	memset(buff, 0, sizeof(buff));
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

	sprintf(buff[i], "CPU State");
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 10, buff[i++]);

	sprintf(buff[i], "PC: %x", cpu->pc);
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 20, buff[i++]);

	sprintf(buff[i], "ACC: %x", cpu->acc);
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 30, buff[i++]);

	sprintf(buff[i], "IX: %x", cpu->ix);
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 40, buff[i++]);

	sprintf(buff[i], "IY: %x", cpu->iy);
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 50, buff[i++]);

	sprintf(buff[i], "SP: %x", cpu->sp);
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 60, buff[i++]);

	sprintf(buff[i], "Processor Status");
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 80, buff[i++]);

	sprintf(buff[i], "Negative: %d", get_bit(cpu->ps, NF));
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 90, buff[i++]);

	sprintf(buff[i], "Overflow: %d", get_bit(cpu->ps, VF));
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 100, buff[i++]);

	sprintf(buff[i], "Unused: 0");
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 110, buff[i++]);

	sprintf(buff[i], "Break: %d", get_bit(cpu->ps, BF));
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 120, buff[i++]);

	sprintf(buff[i], "Decimal: %d", get_bit(cpu->ps, DF));
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 130, buff[i++]);

	sprintf(buff[i], "Interrupt: %d", get_bit(cpu->ps, IF));
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 140, buff[i++]);

	sprintf(buff[i], "Zero: %d", get_bit(cpu->ps, ZF));
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 150, buff[i++]);

	sprintf(buff[i], "Carry: %d", get_bit(cpu->ps, CF));
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 160, buff[i++]);

	// Memory

	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 130, 180, "Memory");

	for (int k = 0; k < 4; k++) {
		if (cpu->pc + k >= 0xFFFF) {
			break;
		}
		sprintf(buff[i], "%x", memory[cpu->pc + k]);
		SDL_SetRenderScale(renderer, 2, 2);
		SDL_RenderDebugText(renderer, 100 + (k * 30), 190, buff[i++]);
	}
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
		} else if (e.type == SDL_EVENT_KEY_UP) {
		}
	}
	cpu_debug_SDL();
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
		advance = true;
	}

	clean_SDL();
}
