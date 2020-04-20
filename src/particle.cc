#include <vector>
#include "opengl.h"
#include "particle.h"
#include "simworld.h"

Particle::Particle()
{
	rad = 1.0;
	elast = 0.75;
	mass = 1.0;
	friction = 0.0;
}

void Particle::add_ignore(const Particle *p)
{
	ignorelist.insert(p);
}

void Particle::set_radius(float rad)
{
	this->rad = rad;
}

float Particle::get_radius() const
{
	return rad;
}

void Particle::set_mass(float m)
{
	mass = m;
}

float Particle::get_mass() const
{
	return mass;
}

void Particle::set_elasticity(float e)
{
	elast = e;
}

float Particle::get_elasticity() const
{
	return elast;
}

void Particle::set_position(const Vec3 &pos)
{
	this->pos = pos;
}

void Particle::set_velocity(const Vec3 &vel)
{
	velocity = vel;
}

Vec3 &Particle::get_position()
{
	return pos;
}

const Vec3 &Particle::get_position() const
{
	return pos;
}

Vec3 &Particle::get_velocity()
{
	return velocity;
}

const Vec3 &Particle::get_velocity() const
{
	return velocity;
}

void Particle::set_friction(float frict)
{
	friction = frict;
}

float Particle::get_friction() const
{
	return friction;
}

void Particle::add_force(const Vec3 &fvec)
{
	forces += fvec;
}

void Particle::step(SimWorld *world, float dt)
{
	Vec3 accel = forces / mass;
	forces.x = forces.y = forces.z = 0.0f;

	velocity = velocity * world->damping + accel * dt - velocity * friction * dt;

	Vec3 newpos = pos + velocity * dt;

	Ray ray(pos, newpos - pos);
	Collision col;

	if(world->collision(ray, rad, &col)) {
		pos = col.pos;
		velocity = -reflect(velocity, col.normal) * elast;
	} else {
		pos = newpos;
	}
}

bool Particle::collision(const Particle *p2, Collision *col) const
{
	if(ignorelist.find(p2) != ignorelist.end()) {
		return false;
	}

	Vec3 v = p2->pos - pos;
	float dist_sq = dot(v, v);
	float radsum = rad + p2->rad;

	if(dist_sq < radsum * radsum) {
		float rad_ratio = rad / p2->rad;
		Vec3 dir = p2->pos - pos;
		col->pos = pos + dir * rad_ratio;
		col->normal = -normalize(dir);
		col->elast = elast * p2->elast;
		return true;
	}
	return false;
}

void Particle::draw() const
{
	float color[] = {0.3, 0.9, 0.2, 1};

	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
	glutSolidSphere(rad * 0.2, 16, 8);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glLineWidth(2.0);

	glBegin(GL_LINES);
	glColor3f(0.3, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(velocity.x, velocity.y, velocity.z);

	glColor3f(0, 0, 0.8);
	glVertex3f(0, 0, 0);
	glVertex3f(forces.x, forces.y, forces.z);
	glEnd();

	glPopAttrib();

	glPopMatrix();
}
