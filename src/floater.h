#ifndef FLOATER_H_
#define FLOATER_H_

#include <vector>
#include "gmath/gmath.h"
#include "particle.h"
#include "constr.h"
#include "mesh.h"

#define NUM_FLOATER_PARTICLES	4

class SimWorld;

class Floater {
private:
	float size;
	Particle part[NUM_FLOATER_PARTICLES];
	std::vector<Constraint*> constr;

	static Mesh *mesh;
	static unsigned int sdr;

public:
	Floater(const Vec3 &pos, float sz);

	void add_to_world(SimWorld *world);

	void constraint();
	void draw() const;
};

#endif	// FLOATER_H_
