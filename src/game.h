#ifndef GAME_H_
#define GAME_H_

#include "opt.h"
#include "tpool.h"
#include "drawtext.h"

#ifdef __cplusplus
#include "gmath/gmath.h"

extern Mat4 world_matrix, view_matrix, proj_matrix;
extern bool keystate[256];
#endif

extern int win_width, win_height;
extern float win_aspect;

extern long time_msec;

extern struct thread_pool *tpool;
extern struct dtx_font *dbgfont;
extern int dbgfont_size;

enum {
	MOD_SHIFT	= 1,
	MOD_CTRL	= 2,
	MOD_ALT		= 4
};

/* special keys */
enum {
	KEY_DEL = 127,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_UP,
	KEY_DOWN,
	KEY_PGUP,
	KEY_PGDOWN,
	KEY_HOME,
	KEY_END,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12
};


#ifdef __cplusplus
extern "C" {
#endif

int game_init(int argc, char **argv);
void game_shutdown(void);

void game_draw(void);
void game_reshape(int x, int y);
void game_keyboard(int c, int press);
void game_mbutton(int bn, int press, int x, int y);
void game_mmotion(int x, int y);

/* implemented in main.c */
long game_timer(void);
void game_quit(void);
void game_fullscreen(int fs);
void game_toggle_fullscreen(void);
unsigned int game_modkeys(void);

#ifdef __cplusplus
}	/* extern "C" */
#endif

#endif	/* GAME_H_ */
