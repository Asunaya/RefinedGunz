#pragma once

#define	ENABLE_SHOWDOW
#include "zapplication.h"
#include "RTypes.h"
#include "RMeshUtil.h"
#include "RBaseTexture.h"

_USING_NAMESPACE_REALSPACE2

class ZShadow final
{
public:
	ZShadow(void);

private:	

	bool bLFShadow;
	bool bRFShadow;

	rmatrix mWorldMatLF;
	rmatrix mWorldMatRF;

	static rvector mNormal;

	float mfDistance;

public:
	bool setMatrix( class ZCharacterObject& char_, float size_ = 100.0f );
	bool setMatrix( RVisualMesh& vmesh, float size_ = 100.f, RBspObject* p_map  = 0 );
	void draw(bool bForced=false);

private:
	rmatrix setSize( float size_ ); // size_ should be bigger than 0
	rmatrix setDirection( rvector& dir_ );
};