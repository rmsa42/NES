#pragma once

#include "SDL3/SDL.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct display {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_AudioStream *stream;
	bool run;
};

int create_SDL_display(const char *win_name, const int width, const int height, struct display *dp);
void clean_SDL_display(struct display *dp);

#ifdef __cplusplus
}
#endif
