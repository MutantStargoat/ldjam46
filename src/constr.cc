#include "constr.h"

Constraint::~Constraint()
{
}


DistConstraint::DistConstraint(Vec3 *p, Vec3 *anc, float d)
{
	ppos = p;
	panchor = anc;
	dist = d;
}

void DistConstraint::satisfy(float dt)
{
	Vec3 dir = *panchor - *ppos;
	float dx = length(dir) - dist;
	*ppos += dir * dx * dt;
}
