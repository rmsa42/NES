#pragma once

#include "sdl_utils.h"
#include "utils.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Debugger {
	word breakpoint;
	bool step_into;
	bool run;
	bool until_bp;
	byte panel;
} Debugger;

int debugger_init(const char *bp, struct Display *dp);
void debugger_handle_event(struct Display *dp, SDL_Event *e);
void update_debugger(struct Display *dp);
bool debug_block();

#ifdef __cplusplus
}
#endif
