#pragma once

#include "SDL3/SDL.h"
#include "fpoint.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	NA,
	SAME_LINE
} text_opt;

typedef enum {
	NORMAL = 0,
	WITH_BORDER = 1,
	CURR_POS = 2,
} panel_flags;

typedef struct panel {
	const char *name;
	fpoint tlc;
	fpoint in_pos;
	char text_buff[50];
	fpoint padding;
	int w;
	int h;
} panel;

struct display {
	bool run;
	panel *context;
	panel *panel_stack[10];
	int stack_i;

	// Backend
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_AudioStream *stream;
};

int create_SDL_display(const char *win_name, const int width, const int height, struct display *dp);
void update_display(struct display *dp, void (*callback)(struct display *dp));
void clean_SDL_display(struct display *dp);

void start_frame(struct display *dp);
int start_panel(struct display *dp, const char *name, fpoint xy, fpoint wh, panel_flags flags);
void end_panel(struct display *dp);
void text(struct display *dp, const char *fmt, ...);
void text_options(struct display *dp, text_opt opts);
void set_in_pos(struct display *dp, fpoint pt);

#ifdef __cplusplus
}
#endif
