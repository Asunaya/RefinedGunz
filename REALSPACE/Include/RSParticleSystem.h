#ifndef __RSPARTICLESYSTEM_H
#define __RSPARTICLESYSTEM_H

#include "rutils.h"

class rparticle {
public:
	rvector Position,Velocity,Acceleration;
	rvector Direction,Up;
	float lifetime;
};

class RSParticle_VerticalAcceleratedField : public rparticle {
public:
	RSParticle_VerticalAcceleratedField(float fAccelZ);
	void Move(float timevalue);

	float m_fAccelZ;
};

#define DEFAULT_GRAVITY -9.8f*30.f

class RSParticle_GravityField : public RSParticle_VerticalAcceleratedField {
public:
	RSParticle_GravityField();
};

class RSParticle_SpaceEnvironment : public rparticle {
public:
	void Move(float timevalue);
};

#endif