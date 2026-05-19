#ifndef TINYGL_FASTMATH
#define TINYGL_FASTMATH
#include <math.h>

/* Replacement, faster math functions. Either include just this file or math.h and then this file */

double glSine(double x);
double glCos(double x);

#ifndef NO_MATH_H_FASTMATH_PATCH
#warning "Using fast math routines!"
#define sin(x) glSine(x)
#define cos(x) glCos(x)
#endif // NO_MATH_H_FASTMATH_PATCH

#endif // TINYGL_FASTMATH