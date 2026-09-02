#include "cpu.h"
#include "SDL3/SDL.h"
#include <stdio.h>
#include <stdbool.h>

extern inst insts[0xFF];
extern byte memory[0xFFFF];
extern struct cpu *cpu;

SDL_Renderer *renderer;
SDL_Window *window;
int window_width = 640;
int window_height = 480;
bool nes_run = false;

int init_SDL() {
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		printf("SDL Init error\n");
		return (1);
	}
	if (!SDL_CreateWindowAndRenderer("NES", window_width, window_height, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
		printf("SDL Window and Renderer error\n");
		return (1);
	}
	SDL_SetRenderLogicalPresentation(renderer, window_width, window_height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	return (0);
}

int clean_SDL() {
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
	memory[i++] = 0x50;
	memory[i++] = SEC_IMPL;
	memory[i++] = SBC_IMM;
	memory[i++] = 0xb0;

	printf("Running CPU\n");
	nes_run = true;
	while (nes_run) {

		byte byte = fetch_byte();

		// Get the instruction
		inst instr = insts[byte];

		// Apply the address mode
		word vls = instr.mode();

		// Run the instruction
		instr.run(vls);
		
		printf("Inst %d runned\n", byte);
		cpu_state();

		update_SDL();
	}

	clean_SDL();
}
