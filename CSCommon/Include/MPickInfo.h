#pragma once

#include "RBspObject.h"

struct MPICKINFO {
	class MMatchObject*	pObject;
	RPickInfo	info;

	bool bBspPicked;
	RBSPPICKINFO bpi;
};
