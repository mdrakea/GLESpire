#ifndef TINYGL_FASTMATH
#define TINYGL_FASTMATH
#include <GL/fixed.h>
#include <math.h>

/* Fixed-point math functions. */

GLfixed glSine(GLfixed x);
GLfixed glCos(GLfixed x);

#ifndef NO_MATH_H_FASTMATH_PATCH
#define sin(x) glSine(x)
#define cos(x) glCos(x)
#define sqrt(x) tgl_fix_sqrt(x)
#endif // NO_MATH_H_FASTMATH_PATCH

#endif // TINYGL_FASTMATH
