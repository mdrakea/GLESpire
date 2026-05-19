#include "lut.h"

double glSine(double x) {
    return sin_lut_lerp(sin_lut_phase_from_radians(x));
}

double glCos(double x) {
    return cos_lut_lerp(sin_lut_phase_from_radians(x));
}