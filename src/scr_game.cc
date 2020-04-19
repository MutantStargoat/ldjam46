#include "opengl.h"
#include "game.h"
#include "scr_game.h"
#include "water.h"
#include "mesh.h"
#include "scene_file.h"

static bool ground_intersect(const Ray &ray, Vec3 *pt);
static void disturb_water(const Vec3 &pt);

static float cam_theta, cam_phi = 25, cam_dist = 8;
static int mouse_x, mouse_y;
static bool bnstate[8];

// start-end point of user interaction with the water during the frame
static int plonkidx = -1;
static Vec2 plonkpt[2];

static Mesh *pengmesh;

static bool pause;
static bool wireframe;

GameScreen::GameScreen()
{
	name = "game";
}

GameScreen::~GameScreen()
{
}

bool GameScreen::init()
{
	if(!init_water(160, 160, 60.0f)) {
		fprintf(stderr, "failed to initialize water sim\n");
		return false;
	}

	SceneFile scn;
	if(!(scn.load("data/penguin.obj")) || scn.meshes.empty()) {
		return false;
	}
	pengmesh = scn.meshes[0];
	return true;
}

void GameScreen::destroy()
{
	delete pengmesh;
	destroy_water();
}

bool GameScreen::start()
{
	// reset water TODO
	glEnable(GL_NORMALIZE);
	return true;
}

void GameScreen::stop()
{
}

void GameScreen::update(float dt)
{
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
			plonk_gauss(pt.x, pt.y, 0.05 * dt / (float)num_plonks, 0.085);
		}

		// reset the plonk index so that we'll start a new user interaction on click
		plonkidx = -1;
	}

	sim_water(dt);
}

void GameScreen::draw()
{
	glMatrixMode(GL_PROJECTION);
	glLoadTransposeMatrixf(proj_matrix.m[0]);

	view_matrix = Mat4::identity;
	view_matrix.pre_translate(0, 0, -cam_dist);
	view_matrix.pre_rotate(deg_to_rad(cam_phi), 1, 0, 0);
	view_matrix.pre_rotate(deg_to_rad(cam_theta), 0, 1, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(view_matrix.m[0]);

	draw_water();

	pengmesh->draw();
}

void GameScreen::key(int key, bool press)
{
	if(press) {
		switch(key) {
		case 27:
			pop_screen();
			break;

		case 'w':
			wireframe = !wireframe;
			glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
			break;

		case ' ':
			pause = !pause;
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
