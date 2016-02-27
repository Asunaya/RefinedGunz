#ifndef __RQUATERNION_H
#define __RQUATERNION_H

#include "rutils.h"

struct rquaternion {
public:
	float x,y,z,w;

	rquaternion(float _x,float _y,float _z,float _w) { x=_x;y=_y;z=_z;w=_w;}
	rquaternion(const rquaternion &q) { x=q.x;y=q.y;z=q.z;w=q.w; }
	rquaternion(rvector &axis,float angle);
	rquaternion() {};

	rquaternion operator * (rquaternion& p);
	rvector v() { return rvector(x,y,z); }

	rquaternion Conjugate();

	rvector Transform(rvector& v);

};


#endif