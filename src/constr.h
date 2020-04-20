#ifndef CONSTR_H_
#define CONSTR_H_

#include "gmath/gmath.h"

class Constraint {
public:
	virtual ~Constraint();

	virtual void satisfy(float dt) = 0;
};

class DistConstraint : public Constraint {
protected:
	Vec3 *ppos, *panchor;
	float dist;

public:
	DistConstraint(Vec3 *p, Vec3 *anc, float dist);

	virtual void satisfy(float dt);
};

#endif	// CONSTR_H_
