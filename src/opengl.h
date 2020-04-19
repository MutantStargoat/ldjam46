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

#ifdef __cplusplus
}
#endif

#endif	/* OPENGL_H_ */
