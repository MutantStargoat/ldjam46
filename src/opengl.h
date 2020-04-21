#ifndef OPENGL_H_
#define OPENGL_H_

#include <GL/glew.h>
#include <GL/glut.h>

#ifdef __unix__
#include <GL/glxew.h>
#endif
#ifdef WIN32
#include <GL/wglew.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int opengl_init(void);
void gl_mtl_diffuse(float r, float g, float b);
void gl_mtl_specular(float r, float g, float b);

#ifdef __cplusplus
}
#endif

#endif	/* OPENGL_H_ */
