#ifndef TENTACLE_H_
#define TENTACLE_H_

#include <vector>
#include <gmath/gmath.h>

class Tentacle {
private:
	std::vector<Vec3> cpoints;

	int find_point_segment(float t);
	float find_segment_t(float t, int start_idx);
	Vec3 find_point_pos(float t);

	void add_control_point(const Vec3 &point);

public:
	Vec3 pos;

	Tentacle();
	~Tentacle();

	bool init();
	void draw(long time);
	Vec3 get_point(float t);
};

#endif // TENTACLE_H_
