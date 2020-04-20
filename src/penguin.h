#ifndef PENGUIN_H_
#define PENGUIN_H_

class Floater;

class Penguin {
public:
	Vec3 pos;
	Floater *parent;

	bool init();
	void destroy();

	void reset();

	void update(float dt);
	void draw();

	Floater *find_next_hop();
	void hop(Floater *to);
};

#endif	// PENGUIN_H_
