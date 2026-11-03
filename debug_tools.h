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

int debugger_init(const char *bp, Display *dp);
void debugger_handle_event(Display *dp, SDL_Event *e);
void update_debugger(Display *dp);
bool debug_block();

#ifdef __cplusplus
}
#endif
