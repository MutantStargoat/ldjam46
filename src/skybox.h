#ifndef SKYBOX_H_
#define SKYBOX_H_

#include "texture.h"

extern Texture *skytex;

bool init_skybox();
void destroy_skybox();

void draw_skybox();

#endif	// SKYBOX_H_
