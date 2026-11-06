#include "nes.h"
#include "debug_tools.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		printf("SDL Init error\n");
		return (1);
	}
	if (debugger_init(argv[2])) {
		return (1);
	}
	if (nes_init(argv[1])) {
		return (1);
	}
	nes_run();
	SDL_Quit();
}

