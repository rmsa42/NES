#include "sdl_utils.h"

int w_h = 480;
int w_w = 640;

void loop(struct Display *dp) {
	SDL_Event e;

	start_frame(dp);
	SDL_SetRenderDrawColor(dp->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(dp->renderer);
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_EVENT_QUIT) {
			dp->run = false;
		} else if (e.type == SDL_EVENT_KEY_DOWN) {
			if (e.key.scancode == SDL_SCANCODE_Q) {
				dp->run = false;
			}
		}
	}
	SDL_SetRenderScale(dp->renderer, 1, 1);
	SDL_SetRenderDrawColor(dp->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	start_panel(dp, "LOP", (fpoint){0, 0}, (fpoint){w_w, w_h}, CURR_POS | WITH_BORDER);
	text(dp, "Start");
	start_panel(dp, "LOP", (fpoint){0, 50}, (fpoint){100, 200}, CURR_POS | WITH_BORDER);
	text(dp, "LArp");
	text(dp, "LArp");
	text(dp, "LArp");
	text(dp, "LArp");
	text(dp, "LArp");
	text(dp, "LArp");
	text_options(dp, SAME_LINE);
	text(dp, "LArp");
	text_options(dp, SAME_LINE);
	text(dp, "Basdfjasjdh");
	text(dp, "LArp");
	text_options(dp, CENTERED);
	text(dp, "LArp");
	end_panel(dp);
	text(dp, "End");
	end_panel(dp);
	SDL_RenderPresent(dp->renderer);
}

int main() {
	struct Display dp;

	create_display("WIN", w_w, w_h, &dp);
	dp.run = true;

	while (dp.run) {
		loop(&dp);
	}

	clean_display(&dp);
}
