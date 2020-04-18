#ifndef SCREEN_H_
#define SCREEN_H_

#include <vector>

class BaseScreen {
public:
	const char *name;
	BaseScreen *prev;

	BaseScreen();
	virtual ~BaseScreen();

	virtual bool init();
	virtual void destroy();

	virtual bool start();
	virtual void stop();

	virtual void draw() = 0;

	virtual void key(int key, bool press);
	virtual void mbutton(int bn, bool press, int x, int y);
	virtual void mmotion(int x, int y);
};

extern std::vector<BaseScreen*> screens;
extern BaseScreen *curscr;

bool init_screens();
void destroy_screens();
BaseScreen *get_screen(const char *name);
void push_screen(BaseScreen *scr);
void pop_screen();

#endif	// SCREEN_H_
