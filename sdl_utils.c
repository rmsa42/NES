#include "sdl_utils.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

int create_panel(panel **pl, const char *name, fpoint tlc, fpoint wh) {
	*pl = calloc(1, sizeof(panel));
	if (*pl == NULL) {
		printf("Malloc Error\n");
		return (1);
	}
	(*pl)->name = name;
	(*pl)->padding = (fpoint){5, 3};
	add_fpoint(&tlc, (*pl)->padding);
	(*pl)->tlc = tlc;
	(*pl)->w = wh.x;
	(*pl)->h = wh.y;
	return (0);
}

void create_panel_border(struct display *dp, fpoint xy, fpoint wh) {
	SDL_FRect rect;

	rect.x = xy.x;
	rect.y = xy.y;
	rect.w = wh.x;
	rect.h = wh.y;
	SDL_RenderRect(dp->renderer, &rect);
}

fpoint pass_panel_pos_to_screen_pos(panel *pan) {
	fpoint ret = pan->in_pos;
	
	add_fpoint(&ret, pan->tlc);
	return (ret);
}

int start_panel(struct display *dp, const char *name, fpoint tlc, fpoint wh, panel_flags flags) {
	panel *panel = NULL;

	if (dp->stack_i > 10) {
		return (1);
	}
	if (flags & CURR_POS) {
		tlc = pass_panel_pos_to_screen_pos(dp->context);
	}
	if (create_panel(&panel, name, tlc, wh)) {
		return (1);
	}
	if (flags & WITH_BORDER) {
		create_panel_border(dp, tlc, wh);
	}
	if (dp->stack_i - 1 >= 1) {
		dp->context->in_pos.y += wh.y + dp->context->padding.y;
	}
	dp->context = panel;
	dp->panel_stack[dp->stack_i] = panel;
	dp->stack_i++;
	return (0);
}

void end_panel(struct display *dp) {
	if (dp->stack_i <= 1) {
		return;
	}
	dp->stack_i--;
	dp->context = dp->panel_stack[dp->stack_i - 1];
}

void text(struct display *dp, const char *fmt, ...) {
	panel *panel = dp->context;
	fpoint screen_pos;
	int len = 0;

	va_list(args);
	va_start(args, fmt);
	vsnprintf(panel->text_buff, sizeof(panel->text_buff), fmt, args);
	va_end(args);
	len = strlen(panel->text_buff) * 8;
	if (panel->in_pos.x + len >= panel->w) {
		panel->in_pos.y += 8;
		panel->in_pos.x = 0;
	}
	screen_pos = pass_panel_pos_to_screen_pos(panel);
	SDL_RenderDebugText(dp->renderer, screen_pos.x, screen_pos.y, panel->text_buff);
	panel->in_pos.y += 8 + panel->padding.y;
}

void text_options(struct display *dp, text_opt opts) {
	panel *panel = dp->context;

	if (opts == SAME_LINE) {
		panel->in_pos.x += strlen(panel->text_buff) * 8 + panel->padding.x;
		panel->in_pos.y -= 8 + panel->padding.y;
	}
}

void set_in_pos(struct display *dp, fpoint pt) {
	dp->context->in_pos = pt;
}

void start_frame(struct display *dp) {
	for (int i = 1; i < 10; i++) {
		free(dp->panel_stack[i]);
		dp->panel_stack[i] = NULL;
	}
	dp->context = dp->panel_stack[0];
	dp->stack_i = 1;
	dp->context->in_pos = dp->context->padding;
}

int create_SDL_display(const char *win_name, const int width, const int height, struct display *dp) {
	memset(dp, 0, sizeof(struct display));
	if (start_panel(dp, win_name, (fpoint){0, 0}, (fpoint){width, height}, NORMAL)) {
		return (1);
	}
	if (!SDL_CreateWindowAndRenderer(win_name, width, height, SDL_WINDOW_RESIZABLE, &dp->window, &dp->renderer)) {
		printf("Can't create window or renderer\n");
		return (1);
	}
	if (!SDL_SetRenderLogicalPresentation(dp->renderer, width, height, SDL_LOGICAL_PRESENTATION_DISABLED)) {
		printf("Can't change renderer presentation\n");
		return (1);
	}
	return (0);
}

void update_display(struct display *dp, void (*callback)(struct display *dp)) {
	SDL_Event e;

	SDL_SetRenderDrawColor(dp->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(dp->renderer);
	while (SDL_PollEvent(&e)) {}
	callback(dp);
	SDL_RenderPresent(dp->renderer);
}

void clean_SDL_display(struct display *dp) {
	free(dp->panel_stack[0]);
	SDL_DestroyRenderer(dp->renderer);
	SDL_DestroyWindow(dp->window);
}

