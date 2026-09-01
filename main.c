#include "cpu.h"
#include "SDL3/SDL.h"
#include <stdio.h>
#include <stdbool.h>

extern inst insts[0xFF];
extern byte memory[0xFFFF];

SDL_Renderer *renderer;
SDL_Window *window;

int init_SDL() {
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		printf("SDL Init error\n");
		return (1);
	}
	if (!SDL_CreateWindowAndRenderer("NES", 640, 480, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
		printf("SDL Window and Renderer error\n");
		return (1);
	}
	return (0);
}

int main() {
	int res;
	bool run = true;

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
	for (int i = 0; i < 3; i++) {
		byte byte = fetch_byte();

		// Get the instruction
		inst instr = insts[byte];

		// Apply the address mode
		word vls = instr.mode();

		// Run the instruction
		instr.run(vls);
		
		printf("Inst %d runned\n", byte);
		cpu_state();
	}
}
