#ifndef WATER_H_
#define WATER_H_

bool init_water(int width, int height, float size);
void destroy_water();

void sim_water(float dt);

float get_water_level(float x, float y);
Vec3 get_water_normal(float x, float y);

void plonk(float x, float y, float amt = 0.025);
void plonk_gauss(float x, float y, float amt = 0.01, float sdev = 1.0);

void draw_water();

#endif	// WATER_H_
