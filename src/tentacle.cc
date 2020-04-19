
#include <GL/gl.h>
#include "tentacle.h"

#define NUM_DRAW_SEG 16

Tentacle::Tentacle()
{
}

Tentacle::~Tentacle()
{
}

bool
Tentacle::init()
{
	for (int i=0; i<5; i++) {
		add_control_point(Vec3(0.5, i / 2.0, 0.0));
	}

	return true;
}

int
Tentacle::find_point_segment(float t)
{
	float segment_len = 1.0 / (cpoints.size() - 1);
	return floor(t / segment_len);
}

float
Tentacle::find_segment_t(float t, int start_idx)
{
	float range1 = t - (float)start_idx / (float)(cpoints.size() - 1);
	float range2 = 1.0 / (float)(cpoints.size() - 1);

	return range1 / range2;
}

Vec3
Tentacle::find_point_pos(float t)
{
	int segment = find_point_segment(t);
	int last_segment = cpoints.size() - 1;

	Vec3 start_cp = cpoints[segment];
	Vec3 end_cp = segment == last_segment ?
				  cpoints[segment] : cpoints[segment + 1];
	Vec3 prev_cp = segment == 0 ? cpoints[segment] :
				   cpoints[segment - 1];
	Vec3 next_cp = segment >= last_segment - 1 ?
				   cpoints[segment] : cpoints[segment + 2];

	float t_seg = find_segment_t(t, segment);
//	return spline(prev_cp, start_cp, end_cp, next_cp, t_seg);
	return lerp(start_cp, end_cp, t_seg);
}

void
Tentacle::draw(long time)
{
	std::vector<Vec3> tpoints;
	glLineWidth(2.0);

	glBegin(GL_LINE_STRIP);
	glColor3f(0.0, 0.0, 1.0);
	for(size_t i = 0; i < NUM_DRAW_SEG; i++) {
		float t = i / (float)(NUM_DRAW_SEG - 1);
		Vec3 pt = get_point(t);

		pt.x += gph::noise(pt.y, time * 0.0005 * pt.y);
		pt.z += gph::noise(pt.y + 12, time * 0.0005 * pt.y);

		glVertex3f(pt.x, pt.y, pt.z);
		tpoints.push_back(pt);
	}
	glEnd();

	//XXX Debug: points
	glDisable(GL_DEPTH_TEST);
	glPointSize(4.0);
	glBegin(GL_POINTS);
	glColor3f(1.0, 0.0, 0.0);
	for (size_t i = 0; i < tpoints.size(); i++) {
		glVertex3f(tpoints[i].x, tpoints[i].y, tpoints[i].z);
	}
	glEnd();
	glEnable(GL_DEPTH_TEST);
}

void
Tentacle::add_control_point(const Vec3 &point)
{
	cpoints.push_back(point);
}

Vec3
Tentacle::get_point(float t)
{
	Vec3 pos = find_point_pos(t);
	printf("pos: x: %f y: %f z: %f\n", pos.x, pos.y, pos.z);
	return pos;
}
