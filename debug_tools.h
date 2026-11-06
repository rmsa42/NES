#pragma once

#include "sdl_utils.h"
#include "utils.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Debugger {
	Display dp;
	word breakpoint;
	bool step_into;
	bool run;
	bool until_bp;
	byte panel;
} Debugger;

int debugger_init(const char *bp);
void debugger_handle_event(Display *dp, SDL_Event *e);
void debugger_update();
Display *debugger_get_display();
void debugger_quit();
bool debug_block();

#ifdef __cplusplus
}
#endif
