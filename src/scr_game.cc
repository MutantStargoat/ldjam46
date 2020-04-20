#include "opengl.h"
#include "game.h"
#include "scr_game.h"
#include "water.h"
#include "mesh.h"
#include "skybox.h"
#include "sdr.h"
#include "simworld.h"
#include "floater.h"
#include "penguin.h"
#include "util.h"

#define WATER_SIZE	100.0f

static bool ground_intersect(const Ray &ray, Vec3 *pt);
static void disturb_water(const Vec3 &pt);

static float cam_theta, cam_phi, cam_dist;
static Vec3 cam_targ_pos;
static Vec3 cam_right_dir, cam_fwd_dir;
static int mouse_x, mouse_y;
static bool bnstate[8];

// start-end point of user interaction with the water during the frame
static int plonkidx = -1;
static Vec2 plonkpt[2];

#define NUM_FLOATERS	10
static Floater *floater[NUM_FLOATERS];
static SimWorld sim;

static bool pause;
static bool wireframe;
static int dbgplonk;

static Penguin peng;


GameScreen::GameScreen()
{
	name = "game";
}

GameScreen::~GameScreen()
{
}

bool GameScreen::init()
{
	if(!init_water(200, 200, WATER_SIZE)) {
		fprintf(stderr, "failed to initialize water sim\n");
		return false;
	}

	if(!init_skybox()) {
		fprintf(stderr, "failed to initialize skybox\n");
		return false;
	}

	if(!peng.init()) {
		fprintf(stderr, "failed to initialize penguin\n");
		return false;
	}
	return true;
}

void GameScreen::destroy()
{
	peng.destroy();
	destroy_water();
}

bool GameScreen::start()
{
	cam_theta = 0;
	cam_phi = 25;
	cam_dist = 8;
	cam_targ_pos = Vec3(0, 1, 0);
	cam_right_dir = Vec3(1, 0, 0);
	cam_fwd_dir = Vec3(0, 0, 1);

	// reset water TODO
	sim.reset();

	for(int i=0; i<NUM_FLOATERS; i++) {
		float pos[2] = {0, 0};

		calc_sample_pos_rec(i, WATER_SIZE, WATER_SIZE, pos);

		floater[i] = new Floater(Vec3(pos[0], 10.0, pos[1]), 1.5);
		floater[i]->add_to_world(&sim);
	}
	sim.set_bounds(-WATER_SIZE / 2.0, WATER_SIZE / 2.0, -WATER_SIZE / 2.0, WATER_SIZE / 2.0);

	// run a few simulation cycles to let the world stabilize
	long t0 = game_timer();
	for(int i=0; i<2048; i++) {
		update(1.0f / 120.0f);
	}
	printf("sim stabilization preroll: %ld msec\n", game_timer() - t0);

	peng.reset();

	return true;
}

void GameScreen::stop()
{
}

void GameScreen::update(float dt)
{
	// camera manipulation should work even in pause, so do it before checking pause
	float walk_speed = 10.0f * dt;
	if(keystate['w']) {
		cam_targ_pos -= cam_fwd_dir * walk_speed;
	}
	if(keystate['s']) {
		cam_targ_pos += cam_fwd_dir * walk_speed;
	}
	if(keystate['d']) {
		cam_targ_pos += cam_right_dir * walk_speed;
	}
	if(keystate['a']) {
		cam_targ_pos -= cam_right_dir * walk_speed;
	}


	if(pause) return;

	// if we had any interaction this frame (plonkidx != -1) then add a series of plonks
	// equally spaced from start to finish (plonkpt[0] to plonkpt[1])
	if(plonkidx >= 0) {
		float len = 1;

		if(plonkidx == 0) {
			// had a single plonk, duplicate it to form a range
			plonkpt[1] = plonkpt[0];
		} else {
			len = distance(plonkpt[0], plonkpt[1]);
		}

		int num_plonks = 1 + (int)(10.0 * len);
		for(int i=0; i<num_plonks; i++) {
			Vec2 pt = lerp(plonkpt[0], plonkpt[1], (float)i / (float)(num_plonks - 1));
			//plonk_gauss(pt.x, pt.y, 0.05 * dt / (float)num_plonks, 0.2);
			plonk_gauss(pt.x, pt.y, (dbgplonk ? 0.1 : 0.05) * dt / (float)num_plonks, 0.085);
		}

		// reset the plonk index so that we'll start a new user interaction on click
		plonkidx = -1;
	}

	sim_water(dt);
	sim.step(dt);

	// satisfy the constraints for each floater
	for(int i=0; i<NUM_FLOATERS; i++) {
		floater[i]->constraint();
	}

	peng.update(dt);
}

void GameScreen::draw()
{
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj_matrix.m[0]);

	view_matrix = Mat4::identity;
	view_matrix.pre_translate(0, 0, -cam_dist);
	view_matrix.pre_rotate(deg_to_rad(cam_phi), 1, 0, 0);
	view_matrix.pre_rotate(deg_to_rad(cam_theta), 0, 1, 0);
	view_matrix.pre_translate(-cam_targ_pos.x, -cam_targ_pos.y, -cam_targ_pos.z);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(view_matrix.m[0]);

	draw_skybox();
	draw_water();

	for(int i=0; i<NUM_FLOATERS; i++) {
		floater[i]->draw();
	}
	sim.draw_particles();

	peng.draw();
}

void GameScreen::key(int key, bool press)
{
	if(press) {
		switch(key) {
		case 27:
			pop_screen();
			break;

		case 'W':
			wireframe = !wireframe;
			glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
			break;

		case ' ':
			pause = !pause;
			break;

		case KEY_F1:
			dbgplonk ^= 1;
			break;

		default:
			break;
		}
	}
}

void GameScreen::mbutton(int bn, bool press, int x, int y)
{
	if(bn < (int)(sizeof bnstate / sizeof *bnstate)) {
		bnstate[bn] = press;
	}
	mouse_x = x;
	mouse_y = y;

	if(bn == 0 && !game_modkeys() && press) {
		float nx = (float)x / (float)win_width;
		float ny = (float)(win_height - y) / (float)win_height;
		Ray pr = mouse_pick_ray(nx, ny, view_matrix, proj_matrix);
		Vec3 p;
		if(ground_intersect(pr, &p)) {
			disturb_water(p);
		}
	}
}

void GameScreen::mmotion(int x, int y)
{
	int dx = x - mouse_x;
	int dy = y - mouse_y;
	mouse_x = x;
	mouse_y = y;

	if((dx | dy) == 0) return;

	if(game_modkeys()) {
		if(bnstate[0]) {
			cam_theta += dx * 0.5f;
			cam_phi += dy * 0.5f;
			if(cam_phi < -90) cam_phi = 90;
			if(cam_phi > 90) cam_phi = 90;

			float theta = deg_to_rad(cam_theta);
			cam_right_dir = Vec3(cos(theta), 0, sin(theta));
			cam_fwd_dir = Vec3(-sin(theta), 0, cos(theta));
		}
		if(bnstate[1]) {
			cam_targ_pos -= cam_right_dir * dx * 0.025f;
			cam_targ_pos -= cam_fwd_dir * dy * 0.025f;
		}
		if(bnstate[2]) {
			cam_dist += dy * 0.1f;
			if(cam_dist < 0) cam_dist = 0;
		}
	} else {
		float nx = (float)x / (float)win_width;
		float ny = (float)(win_height - y) / (float)win_height;
		Ray pr = mouse_pick_ray(nx, ny, view_matrix, proj_matrix);
		Vec3 p;
		if(ground_intersect(pr, &p)) {
			disturb_water(p);
		}
	}
}


static bool ground_intersect(const Ray &ray, Vec3 *pt)
{
	float ndotdir = ray.dir.y;
	if(fabs(ndotdir) < 1e-6) {
		return false;
	}

	Vec3 ptvec = -ray.origin;
	float ndotpt = ptvec.y;

	float t = ndotpt / ndotdir;
	*pt = ray.origin + ray.dir * t;
	return true;
}


static void disturb_water(const Vec3 &pt)
{
	if(++plonkidx > 1) {
		plonkidx = 1;
	}
	plonkpt[plonkidx] = Vec2(pt.x, pt.z);
}
