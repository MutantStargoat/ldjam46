#include "opengl.h"
#include "game.h"
#include "screen.h"

#define START_SCR_NAME	"game"

Mat4 proj_matrix;

int win_width, win_height;
float win_aspect;

long time_msec;

extern "C" {

int game_init(int argc, char **argv)
{
	if(opengl_init() == -1) {
		return -1;
	}

	if(!init_screens()) {
		return -1;
	}
	BaseScreen *start_scr = get_screen(START_SCR_NAME);
	if(!start_scr) {
		fprintf(stderr, "failed to find starting screen: %s\n", START_SCR_NAME);
		return -1;
	}
	push_screen(start_scr);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glClearColor(0.02, 0.02, 0.02, 1);
	return 0;
}

void game_shutdown(void)
{
	destroy_screens();
}

void game_draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	curscr->draw();
}

void game_reshape(int x, int y)
{
	glViewport(0, 0, x, y);

	proj_matrix.perspective(deg_to_rad(50.0f), win_aspect, 0.5, 500.0);
}

void game_keyboard(int c, int press)
{
	curscr->key(c, press);
}

void game_mbutton(int bn, int press, int x, int y)
{
	curscr->mbutton(bn, press, x, y);
}

void game_mmotion(int x, int y)
{
	curscr->mmotion(x, y);
}

}	/* extern "C" */
