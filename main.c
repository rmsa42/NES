#include "cpu.h"
#include "SDL3/SDL.h"
#include "apu.h"
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
int current_sine_sample = 0;
bool nes_run = false;

int init_SDL() {
	SDL_AudioSpec spec;

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		printf("SDL Init error\n");
		return (1);
	}
	if (!SDL_CreateWindowAndRenderer("NES", window_width, window_height, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
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
	char buff[14][50] = {0};
	int i = 0;

	memset(buff, 0, sizeof(buff));
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

	sprintf(buff[i], "CPU State");
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 10, buff[i++]);

	sprintf(buff[i], "PC: %d", cpu->pc);
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 20, buff[i++]);

	sprintf(buff[i], "ACC: %d", cpu->acc);
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 30, buff[i++]);

	sprintf(buff[i], "SP: %d", cpu->sp);
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 40, buff[i++]);

	sprintf(buff[i], "Processor Status");
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 60, buff[i++]);

	sprintf(buff[i], "Negative: %d", get_bit(cpu->ps, NF));
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 70, buff[i++]);

	sprintf(buff[i], "Overflow: %d", get_bit(cpu->ps, VF));
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 80, buff[i++]);

	sprintf(buff[i], "Unused: 0");
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 90, buff[i++]);

	sprintf(buff[i], "Break: %d", get_bit(cpu->ps, BF));
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 100, buff[i++]);

	sprintf(buff[i], "Decimal: %d", get_bit(cpu->ps, DF));
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 110, buff[i++]);

	sprintf(buff[i], "Interrupt: %d", get_bit(cpu->ps, IF));
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 120, buff[i++]);

	sprintf(buff[i], "Zero: %d", get_bit(cpu->ps, ZF));
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 130, buff[i++]);

	sprintf(buff[i], "Carry: %d", get_bit(cpu->ps, CF));
	SDL_SetRenderScale(renderer, 2, 2);
	SDL_RenderDebugText(renderer, 0, 140, buff[i++]);
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
		}
	}
	cpu_debug_SDL();

		//static float samples[512];  /* this will feed 512 samples each frame until we get to our maximum. */
		//int i;

		///* generate a 440Hz pure tone */
		//for (i = 0; i < SDL_arraysize(samples); i++) {
		//	const int freq = 440;
		//	const float phase = current_sine_sample * freq / 8000.0f;
		//	samples[i] = SDL_sinf(phase * 2 * SDL_PI_F);
		//	current_sine_sample++;
		//}

		///* wrapping around to avoid floating-point errors */
		//current_sine_sample %= 8000;

		///* feed the new data to the stream. It will queue at the end, and trickle out as the hardware needs more data. */
		//SDL_PutAudioStreamData(stream, samples, sizeof (samples));
	
	SDL_RenderPresent(renderer);
}

int main() {
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
	memory[i++] = LDA_IMM;
	memory[i++] = 0xB9;
	memory[i++] = STA_ABS;
	memory[i++] = 0x0A;
	memory[i++] = 0x40;
	memory[i++] = LDA_IMM;
	memory[i++] = 0x00;
	memory[i++] = STA_ABS;
	memory[i++] = 0x0B;
	memory[i++] = 0x40;
	//memory[i++] = LDA_IMM;
	//memory[i++] = 0x80;
	//memory[i++] = STA_ABS;
	//memory[i++] = 0x00;
	//memory[i++] = 0x40;

	printf("Running CPU\n");
	nes_run = true;
	while (nes_run) {
		clock_cpu();
		clock_apu(stream);

		update_SDL();
	}

	clean_SDL();
}
