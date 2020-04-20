#include <assert.h>
#include "opengl.h"
#include "floater.h"
#include "simworld.h"
#include "sdr.h"

#define CONSTR_ITER	8

Mesh *Floater::mesh = 0;
unsigned int Floater::sdr = 0;


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
		part[i].set_radius(sz * 0.7);
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

#define VLEN	1.0f
void Floater::draw() const
{
	Vec3 p0 = part[0].get_position();
	Vec3 p1 = part[1].get_position();
	Vec3 p2 = part[2].get_position();
	Vec3 p3 = part[3].get_position();

	Vec3 vec_i = normalize(p1 - p0);
	Vec3 vec_k = normalize(p3 - p0);
	Vec3 vec_j = normalize(cross(vec_k, vec_i));
	vec_k = cross(vec_i, vec_j);

	/*
	Matrix4x4 rot_matrix;
	rot_matrix.set_column_vector(vec_i, 0);
	rot_matrix.set_column_vector(vec_j, 1);
	rot_matrix.set_column_vector(vec_k, 2);
	rot_matrix.set_row_vector(Vec4(0, 0, 0, 1), 3);

	Matrix4x4 xform;
	xform.translate((p0 + p1 + p2 + p3) / 4.0);
	xform *= rot_matrix;
	xform.scale(Vec3(size / 2.0, size / 2.0, size / 2.0));
	*/

	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);

	glLineWidth(2);

	Vec3 org = (p0 + p1 + p2 + p3) / 4.0f;

	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(org.x, org.y, org.z);
	glVertex3f(org.x + vec_i.x * VLEN, p0.y + vec_i.y * VLEN, org.z + vec_i.z * VLEN);
	glColor3f(0, 1, 0);
	glVertex3f(org.x, org.y, org.z);
	glVertex3f(org.x + vec_j.x * VLEN, p0.y + vec_j.y * VLEN, org.z + vec_j.z * VLEN);
	glColor3f(0, 0, 1);
	glVertex3f(org.x, org.y, org.z);
	glVertex3f(org.x + vec_k.x * VLEN, p0.y + vec_k.y * VLEN, org.z + vec_k.z * VLEN);
	glEnd();

	glPopAttrib();

#if 0
	boattex->bind(0);
	bind_program(sdr);

	/*float view_matrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, view_matrix);
	set_uniform_matrix4_transposed(sdr, "view_matrix_trans", view_matrix);*/

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMultTransposeMatrixf(xform[0]);
	mesh->draw();
	glPopMatrix();

	bind_program(0);
#endif
}
