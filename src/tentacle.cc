#include <stdlib.h>
#include <string.h>
#include "opengl.h"
#include "tentacle.h"
#include "meshgen.h"
#include "water.h"

#define NUM_DRAW_SEG 32
#define HEIGHT		4.0f

static Vec2 revol_shape(float u, float v, void *cls)
{
	float t = v * v;
	float x = cos(t * M_PI / 2.0f) * 0.2;
	return Vec2(x, v * HEIGHT);
}

static Vec3 *origv;

bool
Tentacle::init()
{
	mesh = new Mesh;
	gen_revol(mesh, 8, NUM_DRAW_SEG, revol_shape);
	mesh->flip();

	if(!origv) {
		int vcount = mesh->get_attrib_count(MESH_ATTR_VERTEX);
		origv = new Vec3[vcount];
		memcpy(origv, (void*)mesh->get_attrib_data(MESH_ATTR_VERTEX), vcount * sizeof *origv);
	}

	anm = (float)rand() / (float)RAND_MAX * 100.0f;
	pos.y = -100;

	return true;
}

void Tentacle::destroy()
{
	delete mesh;
}

void Tentacle::update(float dt)
{
	Vec3 surfpt = Vec3(0, -100, 0);

	Vec3 *vptr = (Vec3*)mesh->get_attrib_data(MESH_ATTR_VERTEX);
	int vcount = mesh->get_attrib_count(MESH_ATTR_VERTEX);

	anm += dt;
	float tm = anm;

	for(int i=0; i<vcount; i++) {
		float v = vptr->y / HEIGHT;

		float xoffs = gph::noise(vptr->y, tm + vptr->y);
		float zoffs = gph::noise(vptr->y + 12, tm + vptr->y);

		float s = (sin(v * M_PI) * 0.5 + 0.5) * 0.5 + 0.5;

		vptr->x = origv[i].x + xoffs * s * 0.5;
		vptr->z = origv[i].z + zoffs * s * 0.5;

		if(fabs(vptr->y + pos.y) < fabs(surfpt.y)) {
			surfpt = Vec3(pos.x + xoffs, vptr->y + pos.y, pos.z + zoffs);
		}
		vptr++;
	}

	if(fabs(surfpt.y) < 0.2) {
		//plonk_gauss(surfpt.x, surfpt.z, sin(anm * 20.0f) * 0.001 * dt, 0.005);
		plonk(surfpt.x, surfpt.z, sin(anm * 25.0f) * 0.1 * dt);
	}

	mesh->calc_face_normals();
}

void
Tentacle::draw()
{
	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);

	gl_mtl_diffuse(0.08, 0.01, 0.1);
	mesh->draw();

	glPopMatrix();
}
