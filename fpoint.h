#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fpoint {
	float x;
	float y;
} fpoint;

void add_fpoint(fpoint *fpt, fpoint add);
void minus_fpoint(fpoint *fpt, fpoint minus);
void multiply_fpoint(fpoint *fpt, const double scalar);
void divide_fpoint(fpoint *fpt, const double scalar);
double length_fpoint(fpoint *fpt);
void normalize_fpoint(fpoint *fpt);
void perp_fpoint(fpoint *fpt);
void print_fpoint(fpoint *fpt);
void copy_fpoint(fpoint *copy, fpoint *fpt);

#ifdef __cplusplus
}
#endif
