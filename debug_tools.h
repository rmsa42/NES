#pragma once

#include "sdl_utils.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int debugger_init(const char *bp, struct display *dp);
bool update_debugger(struct display *dp);

#ifdef __cplusplus
}
#endif
