#include "sdl_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

int create_panel(Panel **pl, const char *name, FPoint tlc, FPoint wh) {
	*pl = calloc(1, sizeof(Panel));
	if (*pl == NULL) {
		printf("Malloc Error\n");
		return (1);
	}
	(*pl)->name = name;
	(*pl)->padding = (FPoint){2, 2};
	divide_fpoint(&tlc, 8);
	(*pl)->tlc = tlc;
	(*pl)->w = wh.x / 8;
	(*pl)->h = wh.y / 8;
	return (0);
}

void advc_panel_cursor_x_w_padding(Panel *pan, float inc_x) {
	pan->old_cursor.x = pan->in_cursor.x;
	pan->in_cursor.x += inc_x;
}

void advc_panel_cursor_y_w_padding(Panel *pan, float inc_y) {
	pan->old_cursor.y = pan->in_cursor.y;
	pan->in_cursor.y += inc_y;
}

void advc_panel_cursor_w_padding(Panel *pan, float inc_x, float inc_y) {
	advc_panel_cursor_x_w_padding(pan, inc_x);
	advc_panel_cursor_y_w_padding(pan, inc_y);
}

void set_panel_cursor_x_pos(Panel *pan, float xpos) {
	pan->old_cursor.x = pan->in_cursor.x;
	pan->in_cursor.x = xpos;
}

void set_panel_cursor_y_pos(Panel *pan, float ypos) {
	pan->old_cursor.y = pan->in_cursor.y;
	pan->in_cursor.y = ypos;
}

void set_panel_cursor_pos(Panel *pan, float xpos, float ypos) {
	set_panel_cursor_x_pos(pan, xpos);
	set_panel_cursor_y_pos(pan, ypos);
}

void create_panel_border(Display *dp, FPoint xy, FPoint wh) {
	SDL_FRect rect;

	rect.x = xy.x;
	rect.y = xy.y;
	rect.w = wh.x;
	rect.h = wh.y;
	SDL_RenderRect(dp->renderer, &rect);
}

FPoint pass_panel_pos_to_screen_pos(Panel *pan) {
	FPoint ret = pan->in_cursor;
	
	add_fpoint(&ret, pan->tlc);
	multiply_fpoint(&ret, 8);
	return (ret);
}

void pass_panel_wh_to_screen_wh(Panel *pan, int *w, int *h) {
	*w = pan->w * 8;
	*h = pan->h * 8;
}

int start_panel(Display *dp, const char *name, FPoint tlc, FPoint wh, panel_flags flags) {
	Panel *panel = NULL;

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

void end_panel(Display *dp) {
	if (dp->stack_i <= 1) {
		return;
	}
	dp->stack_i--;
	dp->context = dp->panel_stack[dp->stack_i - 1];
}

void text(Display *dp, const char *fmt, ...) {
	Panel *pan = dp->context;
	FPoint screen_pos;
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

void text_options(Display *dp, text_opt opts) {
	dp->context->text_opts = opts;
}

void start_frame(Display *dp) {
	for (int i = 1; i < 10; i++) {
		free(dp->panel_stack[i]);
		dp->panel_stack[i] = NULL;
	}
	dp->context = dp->panel_stack[0];
	dp->stack_i = 1;
	set_panel_cursor_pos(dp->context, 0, 0);
}

int create_display(const char *win_name, const int width, const int height, Display *dp) {
	memset(dp, 0, sizeof(Display));
	if (start_panel(dp, win_name, (FPoint){0, 0}, (FPoint){width, height}, NORMAL)) {
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
	SDL_AudioSpec spec;
	spec.channels = 1;
	spec.format = SDL_AUDIO_F32;
	spec.freq = SAMPLE_RATE;
	dp->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
	if (!dp->stream) {
		printf("SDL Audio Stream error\n");
		return (1);
	}
	SDL_ResumeAudioStreamDevice(dp->stream);
	return (0);
}

void clean_display(Display *dp) {
	for (int i = 0; i < 10; i++) {
		free(dp->panel_stack[i]);
		dp->panel_stack[i] = NULL;
	}
	SDL_DestroyRenderer(dp->renderer);
	dp->renderer = NULL;
	SDL_DestroyWindow(dp->window);
	dp->window = NULL;
	SDL_DestroyAudioStream(dp->stream);
	dp->stream = NULL;
}

Texture *create_texture(Display *dp, int w, int h) {
	Texture *texture = malloc(sizeof(Texture));

	if (texture == NULL) {
		return (NULL);
	}
	texture->w = w;
	texture->h = h;
	texture->pitch = w * 4;
	texture->pixels = calloc(h, texture->pitch);
	if (texture->pixels == NULL) {
		free(texture);
		printf("Error Malloc\n");
		return (NULL);
	}
	texture->sdl_texture = SDL_CreateTexture(dp->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, w, h);
	if (texture->sdl_texture == NULL) {
		free(texture->pixels);
		free(texture);
		printf("Error SDL\n");
		return (NULL);
	}
	texture->status = OK;
	return (texture);
}

void render_texture(Display *dp, Texture *texture) {
	if (texture->status == NEED_UPDATE) {
		SDL_UpdateTexture(texture->sdl_texture, NULL, texture->pixels, texture->pitch);
		texture->status = OK;
	}
	SDL_RenderTexture(dp->renderer, texture->sdl_texture, NULL, NULL);
}

void write_texture_pixel(Texture *texture, int x, int y, byte r, byte g, byte b, byte a) {
	byte *p = texture->pixels;
	int i = (y * texture->pitch) + (x * 4);

	p[i++] = r;
	p[i++] = g;
	p[i++] = b;
	p[i] = a;
	texture->status = NEED_UPDATE;
}

void destroy_texture(Texture *texture) {
	free(texture->pixels);
	texture->pixels = NULL;
	SDL_DestroyTexture(texture->sdl_texture);
	free(texture);
	texture = NULL;
}
