#include "opengl.h"
#include "game.h"
#include "screen.h"
#include "mesh.h"
#include "sdr.h"

Mat4 view_matrix, proj_matrix;

int win_width, win_height;
float win_aspect;

long time_msec;

struct thread_pool *tpool;
struct dtx_font *dbgfont;
int dbgfont_size;

bool keystate[256];

extern "C" {

int game_init(int argc, char **argv)
{
	if(opengl_init() == -1) {
		return -1;
	}

	if(GLEW_ARB_framebuffer_sRGB || GLEW_EXT_framebuffer_sRGB) {
		if(opt.srgb) {
			int srgb_capable;
			glGetIntegerv(GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &srgb_capable);
			printf("Framebuffer %s sRGB-capable\n", srgb_capable ? "is" : "is not");
			if(srgb_capable) {
				glEnable(GL_FRAMEBUFFER_SRGB);
			} else {
				opt.srgb = 0;
			}
		}
	} else {
		opt.srgb = 0;
	}
	if(!opt.srgb) {
		add_shader_header(GL_FRAGMENT_SHADER, "#define FB_NOT_SRGB");
	}


	if(!(tpool = tpool_create(0))) {
		fprintf(stderr, "failed to create thread pool\n");
		return -1;
	}

	if(!(dbgfont = dtx_open_font_glyphmap("data/dbgfont.glyphmap"))) {
		fprintf(stderr, "failed to open dbgfont\n");
		return -1;
	}
	dbgfont_size = dtx_get_glyphmap_ptsize(dtx_get_glyphmap(dbgfont, 0));

	if(!init_screens()) {
		return -1;
	}
	BaseScreen *start_scr = get_screen(opt.startscr);
	if(!start_scr) {
		fprintf(stderr, "failed to find screen: %s\n", opt.startscr);
		return -1;
	}
	push_screen(start_scr);

	Mesh::use_custom_sdr_attr = false;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	if(opt.multisample && GLEW_ARB_multisample) {
		glEnable(GL_MULTISAMPLE);
	}

#ifdef __unix__
	if(GLXEW_EXT_swap_control) {
		int val = 0;
		if(opt.vsync) {
			val = GLXEW_EXT_swap_control_tear ? -1 : 1;
		}
		glXSwapIntervalEXT(glXGetCurrentDisplay(), glXGetCurrentDrawable(), val);
	} else if(GLXEW_SGI_swap_control) {
		glXSwapIntervalSGI(opt.vsync ? 1 : 0);
	}
#endif
#ifdef WIN32
	if(WGLEW_EXT_swap_control) {
		int val = 0;
		if(opt.vsync) {
			val = WGLEW_EXT_swap_control_tear ? -1 : 1;
		}
		wglSwapIntervalEXT(val);
	}
#endif

	glClearColor(0.02, 0.02, 0.02, 1);
	return 0;
}

void game_shutdown(void)
{
	dtx_close_font(dbgfont);
	destroy_screens();
	tpool_destroy(tpool);
}

void game_draw(void)
{
	static long prev_upd;

	long interv = time_msec - prev_upd;
	prev_upd = time_msec;
	curscr->update((float)interv / 1000.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	curscr->draw();

#ifdef DEV_BUILD
	/* fps counter */
	static char fpsbuf[16];
	static long frame, prev_fps_upd;
	static long fps_delta;

	frame++;
	fps_delta = time_msec - prev_fps_upd;
	if(fps_delta >= 1000) {
		float fps = (float)frame * 1000.0f / (float)fps_delta;
		sprintf(fpsbuf, "fps: %.1f", fps);
		frame = 0;
		prev_fps_upd = time_msec;
	}

	dtx_use_font(dbgfont, dbgfont_size);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, win_width, 0, win_height, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, win_height - dtx_line_height(), 0);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glColor3f(0, 0, 0);

	dtx_string(fpsbuf);
	dtx_flush();

	glPopAttrib();
#endif
}

void game_reshape(int x, int y)
{
	glViewport(0, 0, x, y);

	proj_matrix.perspective(deg_to_rad(50.0f), win_aspect, 0.5, 500.0);
}

void game_keyboard(int c, int press)
{
	if(c < 256) {
		keystate[c] = press;
	}

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
