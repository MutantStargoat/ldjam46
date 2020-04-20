#include "opengl.h"
#include "gmath/gmath.h"
#include "penguin.h"
#include "floater.h"
#include "mesh.h"
#include "scene_file.h"
#include "texture.h"

static Mesh *mesh;
static Texture *tex;

bool Penguin::init()
{
	SceneFile scn;
	if(!(scn.load("data/penguin.obj")) || scn.meshes.empty()) {
		return false;
	}
	mesh = scn.meshes[0];

	tex = new Texture;
	if(!tex->load("data/peng_texture.jpg")) {
		return false;
	}

	reset();
	return true;
}

void Penguin::destroy()
{
	delete mesh;
	delete tex;
}

void Penguin::reset()
{
	parent = 0;
	pos = Vec3(0, 0, 0);
}

void Penguin::update(float dt)
{
}

void Penguin::draw()
{
	float col[] = {1.0, 1.0, 1.0, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);

	bind_texture(tex);
	glEnable(GL_TEXTURE_2D);
	mesh->draw();
	bind_texture(0);
}

Floater *Penguin::find_next_hop()
{
	return 0;
}

void Penguin::hop(Floater *to)
{
}
