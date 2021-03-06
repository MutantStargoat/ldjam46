#ifndef TENTACLE_H_
#define TENTACLE_H_

#include <vector>
#include "gmath/gmath.h"
#include "mesh.h"

class Tentacle {
public:
	float anm;
	Mesh *mesh;

	Vec3 pos;

	bool init();
	void destroy();

	void update(float dt);
	void draw();
};

#endif // TENTACLE_H_
