#include <stdarg.h>
#include "zgl.h"

int vfiprintf(FILE *stream, const char *format, va_list ap);

void gl_fatal_error(char *format, ...)
{
  va_list ap;

  va_start(ap,format);

  fiprintf(stderr,"TinyGL: fatal error: ");
  vfiprintf(stderr,format,ap);
  fputc('\n', stderr);
  exit(1);

  va_end(ap);
}
