#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <GL/glut.h>
#include "game.h"

static void display(void);
static void idle(void);
static void reshape(int x, int y);
static void keydown(unsigned char key, int x, int y);
static void keyup(unsigned char key, int x, int y);
static void skeydown(int key, int x, int y);
static void skeyup(int key, int x, int y);
static void mouse(int bn, int st, int x, int y);

static int fullscr;

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(1280, 800);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);
	glutCreateWindow("ldjam46 keepalive");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutSpecialFunc(skeydown);
	glutSpecialUpFunc(skeyup);
	glutMouseFunc(mouse);
	glutMotionFunc(game_mmotion);

	if(game_init(argc, argv) == -1) {
		return 1;
	}
	atexit(game_shutdown);

	glutMainLoop();
	return 0;
}

void game_quit(void)
{
	exit(0);
}

void game_fullscreen(int fs)
{
	static int win_x, win_y;

	if(fs) {
		win_x = glutGet(GLUT_WINDOW_WIDTH);
		win_y = glutGet(GLUT_WINDOW_HEIGHT);
		glutFullScreen();
	} else {
		glutReshapeWindow(win_x, win_y);
	}
}

void game_toggle_fullscreen(void)
{
	fullscr ^= 1;
	game_fullscreen(fullscr);
}

static void display(void)
{
	time_msec = glutGet(GLUT_ELAPSED_TIME);

	game_draw();

	assert(glGetError() == GL_NO_ERROR);
	glutSwapBuffers();
}

static void idle(void)
{
	glutPostRedisplay();
}

static void reshape(int x, int y)
{
	win_width = x;
	win_height = y;
	win_aspect = (float)win_width / (float)win_height;
	game_reshape(x, y);
}

static void keydown(unsigned char key, int x, int y)
{
	game_keyboard(key, 1);
}

static void keyup(unsigned char key, int x, int y)
{
	game_keyboard(key, 0);
}

static void skeydown(int key, int x, int y)
{
	/* TODO */
}

static void skeyup(int key, int x, int y)
{
	/* TODO */
}

static void mouse(int bn, int st, int x, int y)
{
	game_mbutton(bn - GLUT_LEFT_BUTTON, st == GLUT_DOWN, x, y);
}
