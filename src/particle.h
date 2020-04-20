#ifndef PARTICLE_H_
#define PARTICLE_H_

#include <set>
#include "gmath/gmath.h"

class SimWorld;
struct Collision;

class Particle {
private:
	Vec3 forces;
	Vec3 pos, velocity;
	float rad;
	float elast;
	float mass;
	float friction;

	std::set<const Particle*> ignorelist;

public:
	Particle();

	void add_ignore(const Particle *p);

	void set_radius(float rad);
	float get_radius() const;

	void set_mass(float m);
	float get_mass() const;

	void set_elasticity(float e);
	float get_elasticity() const;

	void set_position(const Vec3 &pos);
	void set_velocity(const Vec3 &vel);

	Vec3 &get_position();
	const Vec3 &get_position() const;
	Vec3 &get_velocity();
	const Vec3 &get_velocity() const;

	void set_friction(float frict);
	float get_friction() const;

	void add_force(const Vec3 &fvec);

	void step(SimWorld *world, float dt);

	bool collision(const Particle *p2, Collision *col) const;

	void draw() const;

	friend class SimWorld;
};

#endif	// PARTICLE_H_
