#pragma once

#include "SDL3/SDL.h"
#include "fpoint.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	NA,
	CENTERED = 1 << 1,
	SAME_LINE = 1 << 2,
} text_opt;

typedef enum {
	NORMAL = 0,
	WITH_BORDER = 1,
	CURR_POS = 2,
} panel_flags;

typedef struct panel {
	const char *name;
	fpoint in_cursor;
	fpoint old_cursor;
	fpoint padding;
	fpoint tlc;
	char text_buff[50];
	text_opt text_opts;
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

int create_display(const char *win_name, const int width, const int height, struct display *dp);
void clean_display(struct display *dp);

void start_frame(struct display *dp);
int start_panel(struct display *dp, const char *name, fpoint xy, fpoint wh, panel_flags flags);
void end_panel(struct display *dp);
void text(struct display *dp, const char *fmt, ...);
void text_options(struct display *dp, text_opt opts);

#ifdef __cplusplus
}
#endif
