#ifndef PENGUIN_H_
#define PENGUIN_H_

#include "gmath/gmath.h"

class Floater;

class Penguin {
public:
	Vec3 pos;
	float angle;
	Mat4 xform;

	float hop_t;

	Floater *parent, *next, *prev;
	float restless;

	bool init();
	void destroy();

	void reset();

	void update(float dt);
	void draw();

	void target(Floater *to);
	void hop(Floater *to);
};

#endif	// PENGUIN_H_
