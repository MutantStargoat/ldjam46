#include "opengl.h"
#include "gmath/gmath.h"
#include "penguin.h"
#include "floater.h"
#include "mesh.h"
#include "scene_file.h"
#include "texture.h"
#include "game.h"

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
	parent = next = prev = 0;

	restless = 0;
	pos = Vec3(0, 0, 0);
	angle = 0;
}

static void hop_anim(const Vec3 &pfrom, const Vec3 &pto, float t, Mat4 *hopmat)
{
	float ease = smoothstep(0, 1, t);
	ease *= ease;

	//float s = 1.0f - std::max(sin(t * M_PI * 6.0f), 0.0) * 0.5f;
	float s;
	if(t < 0.33f) {
		s = 1.0f - sin(t * M_PI * 6.0f) * 0.25f;
	} else {
		s = 1.0f;
	}
	float h = sin(ease * M_PI) * 2.0f;

	Vec3 p = lerp(pfrom, pto, ease);
	hopmat->scale(1.0f / s, s, 1.0f / s);
	hopmat->translate(p.x, p.y + h, p.z);
}

#define HOP_DUR	1.0f

void Penguin::update(float dt)
{
	if(parent) {
		xform = parent->xform;
	} else {
		xform = Mat4::identity;

		// no parent, if we have prev and next, we're hopping
		if(prev && next) {
			hop_t += dt / HOP_DUR;

			if(hop_t >= 1.0f) {
				parent = next;
				next = 0;
			} else {
				hop_anim(prev->pos, next->pos, hop_t, &xform);
			}
		}
	}

	xform.translate(pos.x, pos.y, pos.z);

	xform.pre_rotate(angle, 0, 1, 0);
	if(parent && restless > 0.0f) {
		float tm = time_msec / 60.0f;
		xform.pre_rotate(sin(tm) * restless * 0.2, 0, 0, 1);
	}
}

void Penguin::draw()
{
	world_matrix = xform;

	glPushMatrix();
	glMultMatrixf(xform.m[0]);

	float col[] = {1.0, 1.0, 1.0, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);

	bind_texture(tex);
	glEnable(GL_TEXTURE_2D);
	mesh->draw();
	bind_texture(0);

	glPopMatrix();

	/*
	if(next) {
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		Vec3 p = xform * Vec3(0, 0, 0);
		glColor3f(1, 0, 0);
		glVertex3f(p.x, p.y, p.z);
		glVertex3f(next->pos.x, p.y, next->pos.z);
		glEnd();
		glPopAttrib();
	}
	*/
}

void Penguin::target(Floater *to)
{
	next = to;

	Vec3 targ = to->pos;

	if(parent) {
		targ = inverse(parent->xform) * targ;
	}

	Vec3 dir = normalize(targ - pos);

	angle = atan2(dir.x, dir.z);
}

void Penguin::hop(Floater *to)
{
	prev = parent;
	parent = 0;
	hop_t = 0.0f;
	restless = 0.0f;
}
