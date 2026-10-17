#pragma once

#include "sdl_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

int nes_init(struct display *nes_dp, const char *rom_name);
void update_nes(struct display *nes_dp, int curr_time, int *last_time);
void nes_handle_event(struct display *nes_dp, SDL_Event *e);

#ifdef __cplusplus
}
#endif
