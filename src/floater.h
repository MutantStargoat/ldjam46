#ifndef FLOATER_H_
#define FLOATER_H_

#include <vector>
#include "gmath/gmath.h"
#include "particle.h"
#include "constr.h"

#define NUM_FLOATER_PARTICLES	4

class SimWorld;

class Floater {
private:
	float size;
	Particle part[NUM_FLOATER_PARTICLES];
	std::vector<Constraint*> constr;

public:
	Vec3 pos;
	Mat4 xform;

	Floater(const Vec3 &pos, float sz);

	void add_to_world(SimWorld *world);

	void constraint();

	bool is_flipped() const;
	void flip();

	void calc_xform();
	void draw() const;
};

#endif	// FLOATER_H_
