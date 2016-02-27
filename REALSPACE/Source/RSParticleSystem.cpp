#include "RSParticleSystem.h"

RSParticle_VerticalAcceleratedField::RSParticle_VerticalAcceleratedField(float fAccelZ)
{
	m_fAccelZ=fAccelZ;
}

void RSParticle_VerticalAcceleratedField::Move(float timevalue)
{
	Velocity.z+=m_fAccelZ*timevalue;
	Position+=Velocity*timevalue;
	lifetime+=timevalue;
}

RSParticle_GravityField::RSParticle_GravityField()
: RSParticle_VerticalAcceleratedField(DEFAULT_GRAVITY) {}

void RSParticle_SpaceEnvironment::Move(float timevalue)
{
	Position+=Velocity*timevalue;
	lifetime+=timevalue;
}