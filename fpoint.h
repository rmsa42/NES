#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FPoint {
	float x;
	float y;
} FPoint;

void add_fpoint(FPoint *fpt, FPoint add);
void minus_fpoint(FPoint *fpt, FPoint minus);
void multiply_fpoint(FPoint *fpt, const double scalar);
void divide_fpoint(FPoint *fpt, const double scalar);
double length_fpoint(FPoint *fpt);
void normalize_fpoint(FPoint *fpt);
void perp_fpoint(FPoint *fpt);
void print_fpoint(FPoint *fpt);
void copy_fpoint(FPoint *copy, FPoint *fpt);

#ifdef __cplusplus
}
#endif
