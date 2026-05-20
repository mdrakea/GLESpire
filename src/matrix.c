#include "zgl.h"

static void print_fixed(FILE *f, GLfixed v)
{
   int ipart = tgl_fix_to_int(v);
   GLfixed frac = v - TGL_I(ipart);
   int frac4;
   if (frac < 0) frac = -frac;
   frac4 = (int)((frac * 10000) >> TGL_FIX_BITS);
   fiprintf(f, "%d.%04d", ipart, frac4);
}

void gl_print_matrix( const GLfixed *m)
{
   int i;

   for (i=0;i<4;i++) {
      print_fixed(stderr, m[i]);
      fputc(' ', stderr);
      print_fixed(stderr, m[4+i]);
      fputc(' ', stderr);
      print_fixed(stderr, m[8+i]);
      fputc(' ', stderr);
      print_fixed(stderr, m[12+i]);
      fputc('\n', stderr);
   }
}

static inline void gl_matrix_update(GLContext *c)
{
  c->matrix_model_projection_updated=(c->matrix_mode<=1);
}


void glopMatrixMode(GLContext *c,GLParam *p)
{
  int mode=p[1].i;
  switch(mode) {
  case GL_MODELVIEW:
    c->matrix_mode=0;
    break;
  case GL_PROJECTION:
    c->matrix_mode=1;
    break;
  case GL_TEXTURE:
    c->matrix_mode=2;
    break;
  default:
    assert(0);
  }
}

void glopLoadMatrix(GLContext *c,GLParam *p)
{
  M4 *m;
  int i;
  
  GLParam *q;

  m=c->matrix_stack_ptr[c->matrix_mode];
  q=p+1;

  for(i=0;i<4;i++) {
    m->m[0][i]=q[0].f;
    m->m[1][i]=q[1].f;
    m->m[2][i]=q[2].f;
    m->m[3][i]=q[3].f;
    q+=4;
  }

  gl_matrix_update(c);
}

void glopLoadIdentity(GLContext *c,GLParam *p)
{

  gl_M4_Id(c->matrix_stack_ptr[c->matrix_mode]);

  gl_matrix_update(c);
}

void glopMultMatrix(GLContext *c,GLParam *p)
{
  M4 m;
  int i;

  GLParam *q;
  q=p+1;

  for(i=0;i<4;i++) {
    m.m[0][i]=q[0].f;
    m.m[1][i]=q[1].f;
    m.m[2][i]=q[2].f;
    m.m[3][i]=q[3].f;
    q+=4;
  }

  gl_M4_MulLeft(c->matrix_stack_ptr[c->matrix_mode],&m);

  gl_matrix_update(c);
}


void glopPushMatrix(GLContext *c,GLParam *p)
{
  int n=c->matrix_mode;
  M4 *m;

  assert( (c->matrix_stack_ptr[n] - c->matrix_stack[n] + 1 )
	   < c->matrix_stack_depth_max[n] );

  m=++c->matrix_stack_ptr[n];
  
  gl_M4_Move(&m[0],&m[-1]);

  gl_matrix_update(c);
}

void glopPopMatrix(GLContext *c,GLParam *p)
{
  int n=c->matrix_mode;

  assert( c->matrix_stack_ptr[n] > c->matrix_stack[n] );
  c->matrix_stack_ptr[n]--;
  gl_matrix_update(c);
}


void glopRotate(GLContext *c,GLParam *p)
{
  M4 m;
  GLfixed u[3];
  GLfixed angle;
  int dir_code;

  angle = tgl_fix_div_int(tgl_fix_mul(p[1].f, TGL_FIX_PI), 180);
  u[0]=p[2].f;
  u[1]=p[3].f;
  u[2]=p[4].f;

  /* simple case detection */
  dir_code = ((u[0] != 0)<<2) | ((u[1] != 0)<<1) | (u[2] != 0);

  switch(dir_code) {
  case 0:
    gl_M4_Id(&m);
    break;
  case 4:
    if (u[0] < 0) angle=-angle;
    gl_M4_Rotate(&m,angle,0);
    break;
  case 2:
    if (u[1] < 0) angle=-angle;
    gl_M4_Rotate(&m,angle,1);
    break;
  case 1:
    if (u[2] < 0) angle=-angle;
    gl_M4_Rotate(&m,angle,2);
    break;
  default:
    {
      GLfixed cost, sint;

      /* normalize vector */
      GLfixed len = tgl_fix_mul(u[0],u[0])+tgl_fix_mul(u[1],u[1])+tgl_fix_mul(u[2],u[2]);
      if (len == 0) return;
      len = sqrt(len);
      u[0] = tgl_fix_div(u[0], len);
      u[1] = tgl_fix_div(u[1], len);
      u[2] = tgl_fix_div(u[2], len);

      /* store cos and sin values */
      cost=cos(angle);
      sint=sin(angle);

      /* fill in the values */
      m.m[3][0]=m.m[3][1]=m.m[3][2]=
        m.m[0][3]=m.m[1][3]=m.m[2][3]=0;
      m.m[3][3]=TGL_FIX_ONE;

      /* do the math */
      {
        GLfixed ux2=tgl_fix_mul(u[0],u[0]);
        GLfixed uy2=tgl_fix_mul(u[1],u[1]);
        GLfixed uz2=tgl_fix_mul(u[2],u[2]);
        GLfixed one_minus_cost=TGL_FIX_ONE-cost;
        m.m[0][0]=ux2+tgl_fix_mul(cost,TGL_FIX_ONE-ux2);
        m.m[1][0]=tgl_fix_mul(tgl_fix_mul(u[0],u[1]),one_minus_cost)-tgl_fix_mul(u[2],sint);
        m.m[2][0]=tgl_fix_mul(tgl_fix_mul(u[2],u[0]),one_minus_cost)+tgl_fix_mul(u[1],sint);
        m.m[0][1]=tgl_fix_mul(tgl_fix_mul(u[0],u[1]),one_minus_cost)+tgl_fix_mul(u[2],sint);
        m.m[1][1]=uy2+tgl_fix_mul(cost,TGL_FIX_ONE-uy2);
        m.m[2][1]=tgl_fix_mul(tgl_fix_mul(u[1],u[2]),one_minus_cost)-tgl_fix_mul(u[0],sint);
        m.m[0][2]=tgl_fix_mul(tgl_fix_mul(u[2],u[0]),one_minus_cost)-tgl_fix_mul(u[1],sint);
        m.m[1][2]=tgl_fix_mul(tgl_fix_mul(u[1],u[2]),one_minus_cost)+tgl_fix_mul(u[0],sint);
        m.m[2][2]=uz2+tgl_fix_mul(cost,TGL_FIX_ONE-uz2);
      }
    }
  }

  gl_M4_MulLeft(c->matrix_stack_ptr[c->matrix_mode],&m);

  gl_matrix_update(c);
}

void glopScale(GLContext *c,GLParam *p)
{
  GLfixed *m;
  GLfixed x=p[1].f,y=p[2].f,z=p[3].f;

  m=&c->matrix_stack_ptr[c->matrix_mode]->m[0][0];

  m[0] = tgl_fix_mul(m[0], x);     m[1] = tgl_fix_mul(m[1], y);     m[2] = tgl_fix_mul(m[2], z);
  m[4] = tgl_fix_mul(m[4], x);     m[5] = tgl_fix_mul(m[5], y);     m[6] = tgl_fix_mul(m[6], z);
  m[8] = tgl_fix_mul(m[8], x);     m[9] = tgl_fix_mul(m[9], y);     m[10] = tgl_fix_mul(m[10], z);
  m[12] = tgl_fix_mul(m[12], x);   m[13] = tgl_fix_mul(m[13], y);   m[14] = tgl_fix_mul(m[14], z);
  gl_matrix_update(c);
}

void glopTranslate(GLContext *c,GLParam *p)
{
  GLfixed *m;
  GLfixed x=p[1].f,y=p[2].f,z=p[3].f;

  m=&c->matrix_stack_ptr[c->matrix_mode]->m[0][0];

  m[3] = tgl_fix_mul(m[0], x) + tgl_fix_mul(m[1], y) + tgl_fix_mul(m[2], z) + m[3];
  m[7] = tgl_fix_mul(m[4], x) + tgl_fix_mul(m[5], y) + tgl_fix_mul(m[6], z) + m[7];
  m[11] = tgl_fix_mul(m[8], x) + tgl_fix_mul(m[9], y) + tgl_fix_mul(m[10], z) + m[11];
  m[15] = tgl_fix_mul(m[12], x) + tgl_fix_mul(m[13], y) + tgl_fix_mul(m[14], z) + m[15];

  gl_matrix_update(c);
}


void glopFrustum(GLContext *c,GLParam *p)
{
  GLfixed *r;
  M4 m;
  GLfixed left=p[1].f;
  GLfixed right=p[2].f;
  GLfixed bottom=p[3].f;
  GLfixed top=p[4].f;
  GLfixed near=p[5].f;
  GLfixed farp=p[6].f;
  GLfixed x,y,A,B,C,D;

  x = tgl_fix_div(near * 2, right-left);
  y = tgl_fix_div(near * 2, top-bottom);
  A = tgl_fix_div(right+left, right-left);
  B = tgl_fix_div(top+bottom, top-bottom);
  C = -tgl_fix_div(farp+near, farp-near);
  D = -tgl_fix_div(tgl_fix_mul(farp, near) * 2, farp-near);

  r=&m.m[0][0];
  r[0]= x; r[1]=0; r[2]=A; r[3]=0;
  r[4]= 0; r[5]=y; r[6]=B; r[7]=0;
  r[8]= 0; r[9]=0; r[10]=C; r[11]=D;
  r[12]= 0; r[13]=0; r[14]=-TGL_FIX_ONE; r[15]=0;

  gl_M4_MulLeft(c->matrix_stack_ptr[c->matrix_mode],&m);

  gl_matrix_update(c);
}
  
