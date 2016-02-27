#ifndef _RLIGHTLIST_H
#define _RLIGHTLIST_H

class MXmlElement;

#include <string>
#include "RTypes.h"

using namespace std;

#include "RNameSpace.h"
_NAMESPACE_REALSPACE2_BEGIN

struct RLIGHT
{
	string	Name;
	rvector Color;
	rvector Position;
	float	fIntensity;
	float	fAttnStart,fAttnEnd;
	DWORD	dwFlags;
};

class RLightList : public list<RLIGHT*> {
public:
	virtual ~RLightList();
	bool Open(MXmlElement *pElement);
	bool Save(MXmlElement *pElement);
};

_NAMESPACE_REALSPACE2_END


#endif