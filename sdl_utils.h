#pragma once

#include "SDL3/SDL.h"
#include "fpoint.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLE_RATE 44100 // Hz

typedef enum {
	NA,
	CENTERED	= 1 << 1,
	SAME_LINE	= 1 << 2,
} text_opt;

typedef enum {
	NORMAL		= 0,
	WITH_BORDER	= 1,
	CURR_POS	= 2,
} panel_flags;

typedef struct Panel {
	const char *name;
	FPoint in_cursor;
	FPoint old_cursor;
	FPoint padding;
	FPoint tlc;
	char text_buff[50];
	text_opt text_opts;
	int w;
	int h;
} Panel;

typedef struct Display {
	bool run;
	Panel *context;
	Panel *panel_stack[10];
	int stack_i;

	// Backend
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_AudioStream *stream;
} Display;

int create_display(const char *win_name, const int width, const int height, Display *dp);
void clean_display(Display *dp);

void start_frame(Display *dp);
int start_panel(Display *dp, const char *name, FPoint xy, FPoint wh, panel_flags flags);
void end_panel(Display *dp);
void text(Display *dp, const char *fmt, ...);
void text_options(Display *dp, text_opt opts);

#ifdef __cplusplus
}
#endif
