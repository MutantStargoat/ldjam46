#include "simworld.h"
#include "water.h"

#define AIR_FRICTION	0.001f
#define WATER_FRICTION	0.001f
#define BUOYANCY		60.0f
#define WAVE_PUSH		900.0f

SimWorld::SimWorld()
{
	reset();
}

void SimWorld::reset()
{
	part.clear();

	bbmin = Vec2(-1, -1);
	bbmax = Vec2(1, 1);
	grav = Vec3(0, -9.81, 0);
	damping = 0.99;
}

void SimWorld::set_bounds(float xmin, float xmax, float ymin, float ymax)
{
	bbmin.x = xmin;
	bbmin.y = ymin;
	bbmax.x = xmax;
	bbmax.y = ymax;
}

void SimWorld::add_particle(Particle *p)
{
	part.push_back(p);
}

bool SimWorld::collision(const Ray &ray, float rad, Collision *col) const
{
	bool found = false;

	Vec2 min = bbmin + Vec2(rad, rad);
	Vec2 max = bbmax - Vec2(rad, rad);

	// collision with the boundaries
	Vec3 npos = ray.origin + ray.dir;

	Vec3 col_pos, col_norm;
	float d, col_depth = 0;

	if((d = min.x - npos.x) > col_depth) {
		col->pos = npos;
		col->pos.x = min.x;
		col->normal = Vec3(1, 0, 0);
		col_depth = d;
		found = true;
	}
	if((d = min.y - npos.z) > col_depth) {
		col->pos = npos;
		col->pos.z = min.y;
		col->normal = Vec3(0, 0, 1);
		col_depth = d;
		found = true;
	}
	if((d = npos.x - max.x) > col_depth) {
		col->pos = npos;
		col->pos.x = max.x;
		col->normal = Vec3(-1, 0, 0);
		col_depth = d;
		found = true;
	}
	if((d = npos.z - max.y) > col_depth) {
		col->pos = npos;
		col->pos.z = max.y;
		col->normal = Vec3(0, 0, -1);
		col_depth = d;
		found = true;
	}

	return found;
}

void SimWorld::step(float dt)
{
	int count = part.size();
	for(int i=0; i<count; i++) {
		float rad = part[i]->get_radius();

		// add gravity
		part[i]->add_force(grav * part[i]->get_mass());

		// add buoyancy
		Vec3 pos = part[i]->get_position();
		Vec3 vel = part[i]->get_velocity();
		float water_level = get_water_level(pos.x, pos.z);
		float dy = water_level - pos.y + rad / 2.0;
		float bforce = 0.0;
		if(dy > 0.0) {
			float depth = dy < rad ? dy : rad;
			bforce = depth * depth * BUOYANCY;
		}

		Vec3 norm = get_water_normal(pos.x, pos.z);
		part[i]->add_force(Vec3(norm.x * WAVE_PUSH, bforce, norm.z * WAVE_PUSH));

		part[i]->set_friction(dy <= 0.0 ? AIR_FRICTION : WATER_FRICTION);
		part[i]->step(this, dt);

		// handle collisions with other particles
		for(int j=0; j<count; j++) {
			Collision col;
			if(i != j && part[i]->collision(part[j], &col)) {
				Vec3 rel_vel = part[j]->get_velocity() - part[i]->get_velocity();

				float kn = 1.0 / part[i]->get_mass() + 1.0 / part[j]->get_mass();
				float imp = dot(rel_vel, col.normal) * (col.elast + 1) / kn;

				if(imp < 0.0) imp = 0.0;

				Vec3 v = part[i]->get_position() - part[j]->get_position();
				float dist_sq = length_sq(v);
				float pen_depth_sq = part[i]->get_radius() + part[j]->get_radius() - dist_sq;
				if(pen_depth_sq < 0.0) pen_depth_sq = 0.0;

				part[i]->add_force(col.normal * imp * (1.0 / dt * pen_depth_sq + 1));
			}
		}

		Vec3 npos = part[i]->get_position();
		// if pos and npos are on either side of the water level we have a crossing
		if((pos.y - (water_level + rad)) * (npos.y - (water_level + rad)) < 0.0) {
			plonk_gauss(pos.x, pos.z, vel.y * dt * 0.005, 0.05);
		}
	}
}

void SimWorld::draw_particles() const
{
	for(size_t i=0; i<part.size(); i++) {
		part[i]->draw();
	}
}
