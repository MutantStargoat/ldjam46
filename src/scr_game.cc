#include "opengl.h"
#include "game.h"
#include "scr_game.h"
static float cam_theta, cam_phi = 25, cam_dist = 8;
static int mouse_x, mouse_y;
static bool bnstate[8];

GameScreen::GameScreen()
{
	name = "game";
}

GameScreen::~GameScreen()
{
}

bool GameScreen::init()
{
	return true;
}

void GameScreen::destroy()
{
}

bool GameScreen::start()
{
	return true;
}

void GameScreen::stop()
{
}

void GameScreen::draw()
{
	glMatrixMode(GL_PROJECTION);
	glLoadTransposeMatrixf(proj_matrix.m[0]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -cam_dist);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);

	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glVertex3f(-10, 0, 10);
	glVertex3f(10, 0, 10);
	glVertex3f(10, 0, -10);
	glVertex3f(-10, 0, -10);
	glEnd();

	glPushMatrix();
	glTranslatef(0, 0.72, 0);
	glFrontFace(GL_CW);
	glutSolidTeapot(1.0f);
	glFrontFace(GL_CCW);
	glPopMatrix();

}

void GameScreen::key(int key, bool press)
{
	if(press) {
		switch(key) {
		case 27:
			pop_screen();
			break;

		default:
			break;
		}
	}
}

void GameScreen::mbutton(int bn, bool press, int x, int y)
{
	if(bn < sizeof bnstate / sizeof *bnstate) {
		bnstate[bn] = press;
	}
	mouse_x = x;
	mouse_y = y;
}

void GameScreen::mmotion(int x, int y)
{
	int dx = x - mouse_x;
	int dy = y - mouse_y;
	mouse_x = x;
	mouse_y = y;

	if((dx | dy) == 0) return;

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
}
