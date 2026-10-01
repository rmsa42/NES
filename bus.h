#pragma once

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

void bus_write(word addr, byte value);
byte bus_read(word addr);

#ifdef __cplusplus
}
#endif
