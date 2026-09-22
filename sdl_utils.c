#include "sdl_utils.h"
#include <stdio.h>

int create_SDL_display(const char *win_name, const int width, const int height, struct display *dp) {
	memset(dp, 0, sizeof(struct display));
	if (!SDL_CreateWindowAndRenderer(win_name, width, height, SDL_WINDOW_RESIZABLE, &dp->window, &dp->renderer)) {
		printf("Can't create window or renderer\n");
		return (1);
	}
	if (!SDL_SetRenderLogicalPresentation(dp->renderer, 640, 480, SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
		printf("Can't change renderer presentation\n");
		return (1);
	}
	return (0);
}

void clean_SDL_display(struct display *dp) {
	SDL_DestroyRenderer(dp->renderer);
	SDL_DestroyWindow(dp->window);
}
