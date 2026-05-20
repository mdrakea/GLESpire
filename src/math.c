#include "lut.h"

GLfixed glSine(GLfixed x) {
    return sin_lut_lerp(sin_lut_phase_from_radians(x));
}

GLfixed glCos(GLfixed x) {
    return cos_lut_lerp(sin_lut_phase_from_radians(x));
}
