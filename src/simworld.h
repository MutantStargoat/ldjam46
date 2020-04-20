#ifndef SIMWORLD_H_
#define SIMWORLD_H_

#include <vector>
#include "particle.h"
#include "floater.h"
#include "gmath/gmath.h"

struct Collision {
	Vec3 pos;
	Vec3 normal;
	float elast;
};

class SimWorld {
private:
	Vec2 bbmin, bbmax;
	std::vector<Particle*> part;
	Vec3 grav;
	float damping;

public:
	SimWorld();

	void reset();

	void set_bounds(float xmin, float xmax, float ymin, float ymax);

	void set_gravity(const Vec3 &f);

	void add_particle(Particle *p);

	bool collision(const Ray &ray, float rad, Collision *col) const;

	void step(float dt);

	void draw_particles() const;

	friend class Particle;
};

#endif	// SIMWORLD_H_
