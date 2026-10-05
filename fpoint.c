#include "fpoint.h"
#include <math.h>
#include <stdio.h>

void add_fpoint(fpoint *fpt, fpoint add) {
	fpt->x = fpt->x + add.x;
	fpt->y = fpt->y + add.y;
}

void minus_fpoint(fpoint *fpt, fpoint minus) {
	fpt->x = fpt->x - minus.x;
	fpt->y = fpt->y - minus.y;
}

void multiply_fpoint(fpoint *fpt, const double scalar) {
	fpt->x = fpt->x * scalar;
	fpt->y = fpt->y * scalar;
}

void divide_fpoint(fpoint *fpt, const double scalar) {
	if (scalar == 0)
		return;
	fpt->x = fpt->x / scalar;
	fpt->y = fpt->y / scalar;
}

double length_fpoint(fpoint *fpt) {
	return (sqrt(fpt->x * fpt->x + fpt->y * fpt->y));
}

void normalize_fpoint(fpoint *fpt) {
	double	length;

	length = length_fpoint(fpt);
	divide_fpoint(fpt, length);
}

void perp_fpoint(fpoint *fpt) {
	fpt->x = -fpt->y;
	fpt->y = fpt->x;
}

void print_fpoint(fpoint *fpt) {
	printf("fpoint: %f %f\n", fpt->x, fpt->y);
}

void copy_fpoint(fpoint *copy, fpoint *fpt) {
	copy->x = fpt->x;
	copy->y = fpt->y;
}
