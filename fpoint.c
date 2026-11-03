#include "fpoint.h"
#include <math.h>
#include <stdio.h>

void add_fpoint(FPoint *fpt, FPoint add) {
	fpt->x = fpt->x + add.x;
	fpt->y = fpt->y + add.y;
}

void minus_fpoint(FPoint *fpt, FPoint minus) {
	fpt->x = fpt->x - minus.x;
	fpt->y = fpt->y - minus.y;
}

void multiply_fpoint(FPoint *fpt, const double scalar) {
	fpt->x = fpt->x * scalar;
	fpt->y = fpt->y * scalar;
}

void divide_fpoint(FPoint *fpt, const double scalar) {
	if (scalar == 0)
		return;
	fpt->x = fpt->x / scalar;
	fpt->y = fpt->y / scalar;
}

double length_fpoint(FPoint *fpt) {
	return (sqrt(fpt->x * fpt->x + fpt->y * fpt->y));
}

void normalize_fpoint(FPoint *fpt) {
	double	length;

	length = length_fpoint(fpt);
	divide_fpoint(fpt, length);
}

void perp_fpoint(FPoint *fpt) {
	fpt->x = -fpt->y;
	fpt->y = fpt->x;
}

void print_fpoint(FPoint *fpt) {
	printf("FPoint: %f %f\n", fpt->x, fpt->y);
}

void copy_fpoint(FPoint *copy, FPoint *fpt) {
	copy->x = fpt->x;
	copy->y = fpt->y;
}
