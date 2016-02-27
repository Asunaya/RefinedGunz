#include "rquaternion.h"

rquaternion::rquaternion(rvector &axis,float angle)
{
	float s=(float)sin(angle/2);
	x=s*axis.x;
	y=s*axis.y;
	z=s*axis.z;
	w=(float)cos(angle/2);
}

rquaternion rquaternion::operator * (rquaternion& p)
{
	rquaternion ret;
	ret.w=w*p.w-DotProduct(v(),p.v());
	rvector t=CrossProduct(v(),p.v())+w*p.v()+v()*p.w;
	ret.x=t.x;
	ret.y=t.y;
	ret.z=t.z;
	return ret;
}

rquaternion rquaternion::Conjugate()
{
	rquaternion ret;
	ret.x=-x;
	ret.y=-y;
	ret.z=-z;
	ret.w=w;
	return ret;
}

rvector rquaternion::Transform(rvector &v)
{
	rquaternion q=rquaternion(v.x,v.y,v.z,0);
	return ((*this)*q*(this->Conjugate())).v();
}