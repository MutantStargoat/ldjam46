#ifndef SCR_GAME_H_
#define SCR_GAME_H_

#include "screen.h"
#include "scene_file.h"

class GameScreen : public BaseScreen {
	SceneFile scene_file;
public:
	GameScreen();
	~GameScreen();

	bool init();
	void destroy();

	bool start();
	void stop();

	void draw();

	void key(int key, bool press);
	void mbutton(int bn, bool press, int x, int y);
	void mmotion(int x, int y);
};

#endif	// SCR_GAME_H_
