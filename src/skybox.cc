#include "opengl.h"
#include "skybox.h"
#include "mesh.h"
#include "meshgen.h"
#include "game.h"

Texture *skytex;
static Mesh *skybox;

bool init_skybox()
{
	skytex = new Texture;
	if(!skytex->load_cube("data/skybox.jpg")) {
		delete skytex;
		return false;
	}

	skybox = new Mesh;
	gen_sphere(skybox, 2.0f, 12, 6);
	return true;
}

void destroy_skybox()
{
	delete skytex;
	delete skybox;
}

void draw_skybox()
{
	Mat4 mat = view_matrix.upper3x3();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(mat.m[0]);

	glMatrixMode(GL_TEXTURE);
	glLoadTransposeMatrixf(mat.m[0]);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);

	glEnable(GL_TEXTURE_CUBE_MAP);
	bind_texture(skytex);
	skybox->draw();
	bind_texture(0);

	glPopAttrib();

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
