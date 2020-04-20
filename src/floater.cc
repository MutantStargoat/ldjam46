#include <assert.h>
#include "opengl.h"
#include "floater.h"
#include "simworld.h"
#include "sdr.h"

#define CONSTR_ITER	8


Floater::Floater(const Vec3 &pos, float sz)
{
	static const Vec3 ppos[] = {
		Vec3(-1, 0, -1),
		Vec3(1, 0, -1),
		Vec3(1, 0, 1),
		Vec3(-1, 0, 1)
	};
	static int num_masses = sizeof ppos / sizeof *ppos;

	assert(num_masses == NUM_FLOATER_PARTICLES);

	size = sz;

	for(int i=0; i<NUM_FLOATER_PARTICLES; i++) {
		part[i].set_position(pos + ppos[i] * sz * 0.5);
		part[i].set_radius(sz * 1.2);
	}

	for(int i=0; i<NUM_FLOATER_PARTICLES; i++) {
		for(int j=0; j<NUM_FLOATER_PARTICLES; j++) {
			if(i == j)
				continue;

			Vec3 *p1 = &part[i].get_position();
			Vec3 *p2 = &part[j].get_position();
			float dist = distance(*p1, *p2);

			DistConstraint *c = new DistConstraint(p1, p2, dist);
			constr.push_back(c);

			/* don't collide with each-other */
			part[i].add_ignore(part + j);
		}
	}

	/* so what this does is add a random rotation to the floaters so
	 * they don't look the same until they hit the water
	 */
	float hoffs = (float)rand() / (float)RAND_MAX * 2.0f - 1.0f;
	for(int i=0; i<NUM_FLOATER_PARTICLES; i++) {
		float dy;
		if(i == 0) {
			dy = hoffs;
		} else if(i == 2) {
			dy = -hoffs;
		}

		Vec3 pos = part[i].get_position();
		pos.y += dy;
		part[i].set_position(pos);
	}
}

void Floater::add_to_world(SimWorld *world)
{
	for(int i=0; i<NUM_FLOATER_PARTICLES; i++) {
		world->add_particle(part + i);
	}
}

void Floater::constraint()
{
	float constr_step_sz = 1.0 / CONSTR_ITER;
	for(int i=0; i<CONSTR_ITER; i++) {
		for(size_t j=0; j<constr.size(); j++) {
			constr[j]->satisfy(constr_step_sz);
		}
	}
}

bool Floater::is_flipped() const
{
	return xform[1][1] < 0.0f;
}

void Floater::flip()
{
	Particle p = part[0];
	part[0] = part[3];
	part[3] = p;
	p = part[1];
	part[1] = part[2];
	part[2] = p;
}

void Floater::calc_xform()
{
	Vec3 p0 = part[0].get_position();
	Vec3 p1 = part[1].get_position();
	Vec3 p2 = part[2].get_position();
	Vec3 p3 = part[3].get_position();
	Vec3 org = (p0 + p1 + p2 + p3) / 4.0f;

	Vec3 vec_i = normalize(p1 - p0);
	Vec3 vec_k = normalize(p3 - p0);
	Vec3 vec_j = normalize(cross(vec_k, vec_i));
	vec_k = cross(vec_i, vec_j);

	xform = Mat4(vec_i, vec_j, vec_k);
	xform.translate(org.x, org.y - 0.3, org.z);
}

#define VLEN	1.0f
void Floater::draw() const
{
	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);

	glLineWidth(2);

	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(VLEN, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, VLEN, 0);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, VLEN);
	glEnd();

	glPopAttrib();
}
