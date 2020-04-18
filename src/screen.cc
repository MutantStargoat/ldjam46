#include "screen.h"
#include "scr_game.h"
#include "game.h"

std::vector<BaseScreen*> screens;
BaseScreen *curscr;


bool init_screens()
{
	BaseScreen *scr;

	scr = new GameScreen;
	if(!scr->init()) {
		return false;
	}
	screens.push_back(new GameScreen);

	return true;
}

void destroy_screens()
{
	int num = screens.size();
	for(int i=0; i<num; i++) {
		screens[i]->destroy();
		delete screens[i];
	}
	curscr = 0;
}

BaseScreen *get_screen(const char *name)
{
	int num = screens.size();
	for(int i=0; i<num; i++) {
		if(strcmp(screens[i]->name, name) == 0) {
			return screens[i];
		}
	}
	return 0;
}

void push_screen(BaseScreen *scr)
{
	if(scr->prev) {
		scr->prev->stop();
	}
	scr->prev = curscr;
	curscr = scr;
	scr->start();
}

void pop_screen()
{
	if(!curscr || !curscr->prev) {
		game_quit();
		return;
	}

	curscr->stop();
	curscr = curscr->prev;
	curscr->start();
}

BaseScreen::BaseScreen()
{
	prev = 0;
}

BaseScreen::~BaseScreen()
{
}

bool BaseScreen::init()
{
	return true;
}

void BaseScreen::destroy()
{
}

bool BaseScreen::start()
{
	return true;
}

void BaseScreen::stop()
{
}

void BaseScreen::key(int key, bool press)
{
}

void BaseScreen::mbutton(int bn, bool press, int x, int y)
{
}

void BaseScreen::mmotion(int x, int y)
{
}
