#include "opengl.h"
#include "skybox.h"
#include "mesh.h"
#include "meshgen.h"
#include "game.h"
#include "sdr.h"

Texture *skytex;
static Mesh *skybox;
static unsigned int sdr;

bool init_skybox()
{
	skytex = new Texture;
	if(!skytex->load_cube("data/skybox.jpg")) {
		return false;
	}

	if(!(sdr = create_program_load("sdr/skybox.v.glsl", "sdr/skybox.p.glsl"))) {
		return false;
	}

	skybox = new Mesh;
	gen_sphere(skybox, 2.0f, 12, 6);
	return true;
}

void destroy_skybox()
{
	free_program(sdr);
	delete skytex;
	delete skybox;
}

void draw_skybox()
{
	Mat4 mat = view_matrix.upper3x3();

	glPushMatrix();
	glLoadMatrixf(mat.m[0]);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	bind_program(sdr);
	bind_texture(skytex);

	skybox->draw();

	bind_texture(0);
	bind_program(0);

	glPopAttrib();

	glPopMatrix();
}
