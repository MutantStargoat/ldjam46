#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gmath/gmath.h"
#include "water.h"
#include "opengl.h"

static void water_iter();
static inline float water(int x, int y);
static inline void water_vertex(int x, int y, float u, float v, float du, float dv);

static float *buf[2];
static float *dest_buf, *src_buf;
static int xsz, ysz;
static float scale;


bool init_water(int width, int height, float size)
{
	xsz = width;
	ysz = height;
	scale = size;

	int sz = xsz * ysz;

	for(int i=0; i<2; i++) {
		buf[i] = new float[sz];
		memset(buf[i], 0, sz * sizeof *buf[i]);
	}
	dest_buf = buf[0];
	src_buf = buf[1];

	return true;
}

void destroy_water()
{
	delete [] buf[0];
	delete [] buf[1];
}


#define INTERVAL	(1.0 / 40.0)
#define DAMPING		0.97

void sim_water(float dt)
{
	static float accum_dt;

	accum_dt += dt;
	while(accum_dt >= INTERVAL) {
		water_iter();
		accum_dt -= INTERVAL;
	}
}

static void water_iter()
{
	for(int i=1; i<xsz - 1; i++) {
		for(int j=1; j<ysz - 1; j++) {
			int idx = j * xsz + i;

			dest_buf[idx] = ((src_buf[idx - 1] + src_buf[idx + 1] +
						src_buf[idx - xsz] + src_buf[idx + xsz]) / 2.0 - dest_buf[idx]) * DAMPING;
		}
	}

	float *tmp = dest_buf;
	dest_buf = src_buf;
	src_buf = tmp;
}

float get_water_level(float x, float y)
{
	int px = ((x / scale) + 0.5) * xsz;
	int py = ((y / scale) + 0.5) * ysz;

	return water(px, py);
}

Vec3 get_water_normal(float x, float y)
{
	float du = scale / xsz;
	float dv = scale / ysz;

	float dfdx = get_water_level(x + du, y) - get_water_level(x - du, y);
	float dfdy = get_water_level(x, y + dv) - get_water_level(x, y - dv);

	Vec3 tang = Vec3(du, dfdx, 0);
	Vec3 bitan = Vec3(0, dfdy, dv);
	return normalize(cross(bitan, tang));
}

static void plonk_pixel(int x, int y, float amt)
{
	if(x > 0 && x < xsz - 1 && y > 0 && y < ysz - 1) {
		src_buf[y * xsz + x] += amt;
	}
}

static void water_pixel(float x, float y, int *px, int *py)
{
	x /= scale;
	y /= scale;

	*px = (x + 0.5) * xsz;
	*py = (y + 0.5) * ysz;
}

void plonk(float x, float y, float amt)
{
	int px, py;

	water_pixel(x, y, &px, &py);
	plonk_pixel(px, py, amt);
}

static float gaussian(float x, float mean, float sdev)
{
	float d = x - mean;
	float exponent = -(d * d) / (2.0 * (sdev * sdev));
	return 1.0 - -pow(M_E, exponent) / (sdev * sqrt(M_PI * 2.0));
}

void plonk_gauss(float x, float y, float amt, float sdev)
{
	int cx, cy;

	water_pixel(x, y, &cx, &cy);

	for(int i=0; i<5; i++) {
		for(int j=0; j<5; j++) {
			int px = cx + i - 2;
			int py = cy + j - 2;
			float dist = sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));

			plonk_pixel(px, py, amt * gaussian(dist, 0.0, sdev));
		}
	}
}

static inline float water(int x, int y)
{
	if(x >= 0 && x < xsz && y >= 0 && y < ysz) {
		return dest_buf[y * xsz + x];
	}
	return 0.0;
}

static inline void water_vertex(int x, int y, float u, float v, float du, float dv)
{
	float xpos = u - 0.5;
	float ypos = v - 0.5;
	float height = water(x, y);

	float dfdx = water(x + 1, y) - height;
	float dfdy = water(x, y + 1) - height;

	Vec3 tang(du, dfdx, 0.0);
	Vec3 bitan(0.0, dfdy, dv);
	Vec3 norm = -normalize(cross(tang, bitan));

	glColor3f(norm.x * 0.5 + 0.5, norm.y * 0.5 + 0.5, norm.z * 0.5 + 0.5);
	glNormal3f(norm.x, norm.y, norm.z);

	glTexCoord2f(u, v);
	glVertex3f(xpos * scale, height * scale, ypos * scale);
}

void draw_water()
{
	float du = 1.0 / (float)xsz;
	float dv = 1.0 / (float)ysz;

	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	float v = 0.0;
	for(int i=0; i<ysz-1; i++) {
		float u = 0.0;
		for(int j=0; j<xsz-1; j++) {
			water_vertex(j, i + 1, u, v + dv, du, dv);
			water_vertex(j + 1, i + 1, u + du, v + dv, du, dv);
			water_vertex(j + 1, i, u + du, v, du, dv);
			water_vertex(j, i, u, v, du, dv);

			u += du;
		}
		v += dv;
	}
	glEnd();
}
