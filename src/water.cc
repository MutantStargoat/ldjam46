#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gmath/gmath.h"
#include "water.h"
#include "opengl.h"
#include "mesh.h"
#include "game.h"
#include "sdr.h"
#include "skybox.h"

#define VERTS_PER_SAMPLE	1
#define SURF_CUBIC

struct MeshJob {
	float du, dv;
	float v;
	float z;
	int row;

	Vec3 *vptr, *nptr, *tptr;
};

static void water_iter();
static inline float water(int x, int y);
static void updmesh_worker(void *wdata);

static float *buf[2];
static float *dest_buf, *src_buf;
static int xsz, ysz, vxsz, vysz;
static float scale;

static unsigned int sdr;

static Mesh *mesh;
static MeshJob *jobs;


bool init_water(int width, int height, float size)
{
	xsz = width;
	ysz = height;
	scale = size;

	vxsz = width * VERTS_PER_SAMPLE;
	vysz = height * VERTS_PER_SAMPLE;

	int sz = xsz * ysz;
	int vsz = vxsz * vysz;

	for(int i=0; i<2; i++) {
		buf[i] = new float[sz];
		memset(buf[i], 0, sz * sizeof *buf[i]);
	}
	dest_buf = buf[0];
	src_buf = buf[1];

	int num_faces = (vxsz - 1) * (vysz - 1);
	int num_idx = num_faces * 6;

	mesh = new Mesh;
	Vec3 *vptr = (Vec3*)mesh->set_attrib_data(MESH_ATTR_VERTEX, 3, vsz, 0);
	Vec3 *nptr = (Vec3*)mesh->set_attrib_data(MESH_ATTR_NORMAL, 3, vsz, 0);
	Vec3 *tptr = (Vec3*)mesh->set_attrib_data(MESH_ATTR_TANGENT, 3, vsz, 0);
	Vec2 *uvptr = (Vec2*)mesh->set_attrib_data(MESH_ATTR_TEXCOORD, 2, vsz, 0);
	unsigned int *iptr = (unsigned int*)mesh->set_index_data(num_idx, 0);

	float du = 1.0f / (float)vxsz;
	float dv = 1.0f / (float)vysz;

	jobs = new MeshJob[vysz];
	for(int i=0; i<vysz; i++) {
		float v = (float)i * dv;
		jobs[i].row = i;
		jobs[i].du = du;
		jobs[i].dv = dv;
		jobs[i].v = v;
		jobs[i].z = (v - 0.5f) * scale;
		int offs = i * vxsz;
		jobs[i].vptr = vptr + offs;
		jobs[i].nptr = nptr + offs;
		jobs[i].tptr = tptr + offs;
	}

	int vidx = 0;
	for(int i=0; i<vysz; i++) {
		float v = (float)i / (float)(vysz - 1);
		for(int j=0; j<vxsz; j++) {
			float u = (float)j / (float)(vxsz - 1);

			*vptr++ = Vec3(0, 0, 0);
			*nptr++ = Vec3(0, 1, 0);
			*tptr++ = Vec3(1, 0, 0);
			*uvptr++ = Vec2(u, v);

			if(i < vysz - 1 && j < vxsz - 1) {
				*iptr++ = vidx;
				*iptr++ = vidx + vxsz;
				*iptr++ = vidx + vxsz + 1;
				*iptr++ = vidx;
				*iptr++ = vidx + vxsz + 1;
				*iptr++ = vidx + 1;
			}
			vidx++;
		}
	}
	mesh->set_attrib_usage(MESH_ATTR_VERTEX, GL_STREAM_DRAW);
	mesh->set_attrib_usage(MESH_ATTR_NORMAL, GL_STREAM_DRAW);
	mesh->set_attrib_usage(MESH_ATTR_TANGENT, GL_STREAM_DRAW);

	if(!(sdr = create_program_load("sdr/water.v.glsl", "sdr/water.p.glsl"))) {
		return false;
	}

	return true;
}

void destroy_water()
{
	delete [] jobs;

	delete [] buf[0];
	delete [] buf[1];

	delete mesh;
	free_program(sdr);
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
	return 0.0f;
}

/* cubic interpolation grid (SURF_CUBIC defined)
 * pp --- 0p --- 1p --- np
 *  |      |      |      |
 * p0 --- 00 --- 10 --- n0
 *  |      |      |      |
 * p1 --- 01 --- 11 --- n1
 *  |      |      |      |
 * pn --- 0n --- 1n --- nn
 */

static inline float nwater(float u, float v)
{
	float fxsz = (float)xsz;
	float fysz = (float)ysz;

	float floorx = floor(u * fxsz);
	float floory = floor(v * fysz);

	float x0 = floorx / fxsz;
	float y0 = floory / fysz;

	float s = (u - x0) * fxsz;
	float t = (v - y0) * fysz;

	int x = (int)floorx;
	int y = (int)floory;

	float h00 = water(x, y);
	float h10 = water(x + 1, y);
	float h01 = water(x, y + 1);
	float h11 = water(x + 1, y + 1);

#ifdef SURF_CUBIC
	float hpp = water(x - 1, y - 1);
	float h0p = water(x, y - 1);
	float h1p = water(x + 1, y - 1);
	float hnp = water(x + 2, y - 1);

	float hp0 = water(x - 1, y);
	float hn0 = water(x + 2, y);

	float hp1 = water(x - 1, y + 1);
	float hn1 = water(x + 2, y + 1);

	float hpn = water(x - 1, y + 2);
	float h0n = water(x, y + 2);
	float h1n = water(x + 1, y + 2);
	float hnn = water(x + 2, y + 2);

	float a = bspline(hpp, h0p, h1p, hnp, s);
	float b = bspline(hp0, h00, h10, hn0, s);
	float c = bspline(hp1, h01, h11, hn1, s);
	float d = bspline(hpn, h0n, h1n, hnn, s);

	return bspline(a, b, c, d, t);
#else	/* !SURF_CUBIC */
	float a = lerp(h00, h10, s);
	float b = lerp(h01, h11, s);
	return lerp(a, b, t);
#endif
}

void draw_water()
{
	/* mark the attribute arrays we're going to modify as dirty */
	mesh->get_attrib_data(MESH_ATTR_VERTEX);
	mesh->get_attrib_data(MESH_ATTR_NORMAL);
	mesh->get_attrib_data(MESH_ATTR_TANGENT);

	for(int i=0; i<vysz; i++) {
		tpool_enqueue(tpool, jobs + i, updmesh_worker, 0);
	}

	tpool_wait(tpool);

	bind_program(sdr);
	bind_texture(skytex);
	mesh->draw();
	bind_texture(0);
	bind_program(0);
}


static void updmesh_worker(void *wdata)
{
	MeshJob *job = (MeshJob*)wdata;

	Vec3 *vptr = job->vptr;
	Vec3 *nptr = job->nptr;
	Vec3 *tptr = job->tptr;

	float u = 0.0f;
	for(int i=0; i<vxsz; i++) {
		float x = (u - 0.5f) * scale;
#if VERTS_PER_SAMPLE == 1 && !defined(SURF_CUBIC)
		float y = water(i, job->row);
		float dfdx = water(i + 1, job->row) - y;
		float dfdy = water(i, job->row + 1) - y;
#else	/* supersampling */
		float y = nwater(u, job->v);
		float dfdx = nwater(u + job->du, job->v) - y;
		float dfdy = nwater(u, job->v + job->dv) - y;
#endif

		Vec3 tang = Vec3(job->du, dfdx, 0.0f);
		Vec3 bitan = Vec3(0.0f, dfdy, job->dv);
		tang.normalize();
		bitan.normalize();

		*vptr++ = Vec3(x, y * scale, job->z);
		*nptr++ = -cross(tang, bitan);
		*tptr++ = tang;

		u += job->du;
	}
}
