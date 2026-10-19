#include "sdl_utils.h"
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
	(*pl)->padding = (fpoint){2, 2};
	divide_fpoint(&tlc, 8);
	(*pl)->tlc = tlc;
	(*pl)->w = wh.x / 8;
	(*pl)->h = wh.y / 8;
	return (0);
}

void advc_panel_cursor_x_w_padding(panel *pan, float inc_x) {
	pan->old_cursor.x = pan->in_cursor.x;
	pan->in_cursor.x += inc_x;
}

void advc_panel_cursor_y_w_padding(panel *pan, float inc_y) {
	pan->old_cursor.y = pan->in_cursor.y;
	pan->in_cursor.y += inc_y;
}

void advc_panel_cursor_w_padding(panel *pan, float inc_x, float inc_y) {
	advc_panel_cursor_x_w_padding(pan, inc_x);
	advc_panel_cursor_y_w_padding(pan, inc_y);
}

void set_panel_cursor_x_pos(panel *pan, float xpos) {
	pan->old_cursor.x = pan->in_cursor.x;
	pan->in_cursor.x = xpos;
}

void set_panel_cursor_y_pos(panel *pan, float ypos) {
	pan->old_cursor.y = pan->in_cursor.y;
	pan->in_cursor.y = ypos;
}

void set_panel_cursor_pos(panel *pan, float xpos, float ypos) {
	set_panel_cursor_x_pos(pan, xpos);
	set_panel_cursor_y_pos(pan, ypos);
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
	fpoint ret = pan->in_cursor;
	
	add_fpoint(&ret, pan->tlc);
	multiply_fpoint(&ret, 8);
	return (ret);
}

void pass_panel_wh_to_screen_wh(panel *pan, int *w, int *h) {
	*w = pan->w * 8;
	*h = pan->h * 8;
}

int start_panel(struct display *dp, const char *name, fpoint tlc, fpoint wh, panel_flags flags) {
	panel *panel = NULL;

	if (dp->stack_i >= 10) {
		return (1);
	}
	if (flags & CURR_POS) {
		tlc = pass_panel_pos_to_screen_pos(dp->context);
		if (dp->stack_i - 1 >= 1) {
			dp->context->in_cursor.y += wh.y / 8;
		}
	}
	if (create_panel(&panel, name, tlc, wh)) {
		return (1);
	}
	if (flags & WITH_BORDER) {
		create_panel_border(dp, tlc, wh);
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
	panel *pan = dp->context;
	fpoint screen_pos;
	int len = 0;

	va_list(args);
	va_start(args, fmt);
	vsnprintf(pan->text_buff, sizeof(pan->text_buff), fmt, args);
	va_end(args);
	len = strlen(pan->text_buff);
	if (pan->text_opts & CENTERED) {
		advc_panel_cursor_x_w_padding(pan, (pan->w / 2.0f) - (len / 2.0f));
	}
	if (pan->text_opts & SAME_LINE) {
		set_panel_cursor_pos(pan, pan->old_cursor.x, pan->old_cursor.y);
	}
	if (pan->in_cursor.x + len >= pan->w) {
		set_panel_cursor_x_pos(pan, 0);
		advc_panel_cursor_y_w_padding(pan, 1);
	}
	screen_pos = pass_panel_pos_to_screen_pos(pan);
	SDL_RenderDebugText(dp->renderer, screen_pos.x, screen_pos.y, pan->text_buff);
	advc_panel_cursor_w_padding(pan, len, 1);
	set_panel_cursor_x_pos(pan, 0);
}

void text_options(struct display *dp, text_opt opts) {
	dp->context->text_opts = opts;
}

void start_frame(struct display *dp) {
	for (int i = 1; i < 10; i++) {
		free(dp->panel_stack[i]);
		dp->panel_stack[i] = NULL;
	}
	dp->context = dp->panel_stack[0];
	dp->stack_i = 1;
	set_panel_cursor_pos(dp->context, 0, 0);
}

int create_display(const char *win_name, const int width, const int height, struct display *dp) {
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

void clean_display(struct display *dp) {
	for (int i = 0; i < 10; i++) {
		free(dp->panel_stack[i]);
		dp->panel_stack[i] = NULL;
	}
	SDL_DestroyRenderer(dp->renderer);
	dp->renderer = NULL;
	SDL_DestroyWindow(dp->window);
	dp->window = NULL;
}

