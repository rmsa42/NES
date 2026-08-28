#include "apu.h"
#include <stdlib.h>
#include <string.h>

struct apu *apu;

int apu_instance() {
	if (apu != NULL) {
		return (1);
	}
	apu = malloc(1 * sizeof(struct apu));

	if (!apu) { return (1); }
	memset(apu, 0, sizeof(struct apu));
	return (0);
}
