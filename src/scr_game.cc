#include <float.h>
#include <algorithm>
#include "opengl.h"
#include "game.h"
#include "scr_game.h"
#include "water.h"
#include "mesh.h"
#include "meshgen.h"
#include "skybox.h"
#include "sdr.h"
#include "simworld.h"
#include "floater.h"
#include "penguin.h"
#include "util.h"

#define WATER_SIZE	100.0f

#define SQ(x)	((x) * (x))
#define MAX_EXCITE_DIST_SQ	SQ(15.0f)
#define MAX_HOP_DIST_SQ		SQ(9.0f)

static void draw_ui();
static bool ground_intersect(const Ray &ray, Vec3 *pt);
static void disturb_water(const Vec3 &pt);
static float gen_icepatch_mesh(Mesh *mesh);
static Floater *find_nearest_floater(const Vec3 &p, Floater *ignore);

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
static Mesh *ice_mesh[NUM_FLOATERS];
static float floater_height[NUM_FLOATERS];
static SimWorld sim;

static bool pause;
static bool wireframe;
static int show_help;
static int dbgplonk, dbg_show_floaters;
static bool dbgjump;

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

	for(int i=0; i<NUM_FLOATERS; i++) {
		ice_mesh[i] = new Mesh;
		floater_height[i] = gen_icepatch_mesh(ice_mesh[i]);
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
	cam_theta = 180;
	cam_phi = 20;
	cam_dist = 8;
	cam_targ_pos = Vec3(0, 1, 0);
	cam_right_dir = Vec3(-1, 0, 0);
	cam_fwd_dir = Vec3(0, 0, -1);

	// reset water TODO
	sim.reset();

	for(int i=0; i<NUM_FLOATERS; i++) {
		float pos[2] = {0, 0};

		if(i > 0) {
			calc_sample_pos_rec(i, WATER_SIZE, WATER_SIZE, pos);
		}

		floater[i] = new Floater(Vec3(pos[0], 10.0, pos[1]), 3);
		floater[i]->add_to_world(&sim);
	}
	sim.set_bounds(-WATER_SIZE / 2.0, WATER_SIZE / 2.0, -WATER_SIZE / 2.0, WATER_SIZE / 2.0);

	// run a few simulation cycles to let the world stabilize
	long t0 = game_timer();
	for(int i=0; i<2048; i++) {
		update(1.0f / 120.0f);
	}
	printf("sim stabilization preroll: %ld msec\n", game_timer() - t0);

	for(int i=0; i<NUM_FLOATERS; i++) {
		floater[i]->calc_xform();
		if(floater[i]->is_flipped()) {
			floater[i]->flip();
		}
	}

	peng.reset();
	peng.parent = floater[0];

	return true;
}

void GameScreen::stop()
{
	for(int i=0; i<NUM_FLOATERS; i++) {
		delete floater[i];
	}
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
		floater[i]->calc_xform();
	}

	// see if we have nearby floaters to jump towards
	if(peng.parent) {
		if(peng.prev) {
			if(distance_sq(peng.prev->pos, peng.parent->pos) > MAX_EXCITE_DIST_SQ) {
				peng.prev = 0;
			}
		}

		Floater *nearest = find_nearest_floater(peng.parent->pos, peng.prev);
		if(nearest) {
			float dsq = distance_sq(peng.parent->pos, nearest->pos);
			if(dsq < MAX_EXCITE_DIST_SQ) {
				peng.restless = 1.0f - (dsq / MAX_EXCITE_DIST_SQ);
				peng.target(nearest);
			}
			if(dsq < MAX_HOP_DIST_SQ || dbgjump) {
				peng.hop(nearest);
				dbgjump = false;
			}
		}
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
		glPushMatrix();
		floater[i]->calc_xform();
		glMultMatrixf(floater[i]->xform.m[0]);

		if(dbg_show_floaters) {
			floater[i]->draw();
		} else {
			ice_mesh[i]->draw();
		}

		glPopMatrix();
	}

	if(dbg_show_floaters) {
		sim.draw_particles();
	}

	peng.draw();

	draw_ui();
}

void GameScreen::key(int key, bool press)
{
	if(press) {
		switch(key) {
		case 27:
			if(show_help) {
				show_help = 0;
			} else {
				pop_screen();
			}
			break;

		case 'W':
			wireframe = !wireframe;
			glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
			break;

		case ' ':
			pause = !pause;
			break;

		case KEY_F1:
			show_help ^= 1;
			break;

		case KEY_F2:
			dbg_show_floaters ^= 1;
			break;

		case KEY_F3:
			dbgplonk ^= 1;
			break;

		case '`':
			dbgjump = true;
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
		if(bnstate[2]) {
			cam_dist += dy * 0.1f;
			if(cam_dist < 0) cam_dist = 0;
		}
	} else {
		if(bnstate[0]) {
			float nx = (float)x / (float)win_width;
			float ny = (float)(win_height - y) / (float)win_height;
			Ray pr = mouse_pick_ray(nx, ny, view_matrix, proj_matrix);
			Vec3 p;
			if(ground_intersect(pr, &p)) {
				disturb_water(p);
			}
		}
		if(bnstate[2]) {
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
	}
}

static const char *helpstr[] = {
	"Keep the penguin alive!",
	"Try to push floating ice chunks into the penguin's path to safety",
	"Make waves by dragging the mouse with the left button pressed",
	"Rotate the view with the right mouse button",
	"Move around with the middle mouse button or WASD",
	0
};
static const char *pressf1_str = "Press F1 for help";

static void draw_ui()
{
	static const float textcol[][3] = {{0, 0, 0}, {0.2, 0.5, 1}};

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-win_width / 2, win_width / 2, -win_height / 2, win_height / 2, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float line_height = dtx_line_height();
	float soffs = win_height * 0.002;
	if(soffs < 1) soffs = 1.0f;

	glColor3f(0, 0, 0);
	for(int j=0; j<2; j++) {
		glPushMatrix();
		if(!show_help) {
			glTranslatef(-dtx_string_width(pressf1_str) / 2, win_height / 2 - line_height * 1.2 + soffs, 0);
			glColor3fv(textcol[j]);
			dtx_string(pressf1_str);
		} else {
			glTranslatef(0, win_height / 4, 0);

			float y = line_height * 1.8;
			float height = line_height * (sizeof helpstr / sizeof *helpstr + 1);
			glBegin(GL_QUADS);
			glColor4f(0, 0, 0, 0.8);
			glVertex2f(-win_width / 2, y);
			glVertex2f(-win_width / 2, y - height);
			glVertex2f(win_width / 2, y - height);
			glVertex2f(win_width / 2, y);
			glEnd();

			glColor3fv(textcol[j]);
			for(int i=0; helpstr[i]; i++) {
				float w = dtx_string_width(helpstr[i]);
				glPushMatrix();
				glTranslatef(-w / 2, -line_height * i, 0);
				dtx_string(helpstr[i]);
				glPopMatrix();
			}
		}
		glPopMatrix();
		dtx_flush();

		glTranslatef(-soffs, soffs, 0);
	}

	glPopAttrib();

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

static float gen_icepatch_mesh(Mesh *mesh)
{
	float cheight = 0.0f;
	float sz = 5.0f;
	gen_plane(mesh, sz, sz, 16, 16);

	Vec3 offs;
	offs.x = (float)rand() / (float)RAND_MAX * 128.0f;
	offs.y = (float)rand() / (float)RAND_MAX * 128.0f;
	offs.z = (float)rand() / (float)RAND_MAX * 128.0f;

	Mat4 xform;
	xform.rotation(deg_to_rad(-90), 1, 0, 0);
	mesh->apply_xform(xform);

	Vec3 *vptr = (Vec3*)mesh->get_attrib_data(MESH_ATTR_VERTEX);
	int nverts = mesh->get_attrib_count(MESH_ATTR_VERTEX);

	for(int i=0; i<nverts; i++) {
		float theta = atan2(vptr->z, vptr->x) + M_PI;
		float u = theta / (2.0 * M_PI);
		float qa = fmod(theta, M_PI / 2.0);
		float s = fabs(std::max(cos(qa), sin(qa)));

		Vec3 vpos = *vptr;
		vpos.x *= s;
		vpos.z *= s;

		float r = sqrt(vpos.x * vpos.x + vpos.z * vpos.z);

		if(r >= sz / 2.0f - 1e-5) {
			vpos.y -= 2.0f;
		} else {
			vpos.y += noise((offs.x + vpos.x) * 0.8, (offs.z + vpos.z) * 0.8) * 0.5 - r * r  * 0.09;
		}

		s = 1.0f + fbm((offs.y + u) * 10.0f, 2) * 0.5;
		vpos.x *= s;
		vpos.z *= s;

		if(r <= 1e-5) {
			cheight = vpos.y;
		}

		*vptr++ = vpos;
	}

	mesh->calc_face_normals();
	return cheight;
}

static Floater *find_nearest_floater(const Vec3 &p, Floater *ignore)
{
	Floater *best = 0;
	float best_dsq = FLT_MAX;

	for(int i=0; i<NUM_FLOATERS; i++) {
		if(floater[i] == peng.parent || floater[i] == ignore) continue;

		float dsq = distance_sq(p, floater[i]->pos);
		if(dsq < best_dsq) {
			best = floater[i];
			best_dsq = dsq;
		}
	}
	return best;
}
