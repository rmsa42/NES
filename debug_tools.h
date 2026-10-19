#pragma once

#include "sdl_utils.h"
#include "utils.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct debugger {
	word breakpoint;
	bool step_into;
	bool run;
	bool until_bp;
	byte panel;
} debugger;

int debugger_init(const char *bp, struct display *dp);
void debugger_handle_event(struct display *dp, SDL_Event *e);
void update_debugger(struct display *dp);
bool debug_block();

#ifdef __cplusplus
}
#endif
