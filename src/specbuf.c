#include "zgl.h"
#include "msghandling.h"
#include <stdlib.h>

static void calc_buf(GLSpecBuf *buf, const GLfixed shininess)
{
  int i;
  GLfixed val, inc;
  int exp;
  val = 0;
  inc = TGL_FIX_ONE / SPECULAR_BUFFER_SIZE;
  exp = tgl_fix_to_int(shininess + TGL_FIX_HALF);
  for (i = 0; i <= SPECULAR_BUFFER_SIZE; i++) {
    buf->buf[i] = tgl_fix_powi(val, exp);
    val += inc;
  }
}

GLSpecBuf *
specbuf_get_buffer(GLContext *c, const int shininess_i, 
                   const GLfixed shininess)
{
  GLSpecBuf *found, *oldest;
  found = oldest = c->specbuf_first;
  while (found && found->shininess_i != shininess_i) {
    if (found->last_used < oldest->last_used) {
      oldest = found;
    }
    found = found->next; 
  }
  if (found) { /* hey, found one! */
    found->last_used = c->specbuf_used_counter++;
    return found;
  }
  if (oldest == NULL || c->specbuf_num_buffers < MAX_SPECULAR_BUFFERS) {
    /* create new buffer */
    GLSpecBuf *buf = gl_malloc(sizeof(GLSpecBuf));
    if (!buf) gl_fatal_error("could not allocate specular buffer");
    c->specbuf_num_buffers++;
    buf->next = c->specbuf_first;
    c->specbuf_first = buf;
    buf->last_used = c->specbuf_used_counter++;
    buf->shininess_i = shininess_i;
    calc_buf(buf, shininess);
    return buf;     
  }
  /* overwrite the lru buffer */
  /*tgl_trace("overwriting spec buffer :(\n");*/
  oldest->shininess_i = shininess_i;
  oldest->last_used = c->specbuf_used_counter++;
  calc_buf(oldest, shininess);
  return oldest;
}
