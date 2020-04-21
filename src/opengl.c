#include "opengl.h"

int opengl_init()
{
	glewInit();
	return 0;
}

void gl_mtl_diffuse(float r, float g, float b)
{
	float col[4];
	col[0] = r;
	col[1] = g;
	col[2] = b;
	col[3] = 1.0f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
}

void gl_mtl_specular(float r, float g, float b)
{
	float col[4];
	col[0] = r;
	col[1] = g;
	col[2] = b;
	col[3] = 1.0f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, col);
}
