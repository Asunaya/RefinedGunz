#ifndef _RLIGHTLIST_H
#define _RLIGHTLIST_H

#include <string>
#include "RTypes.h"

#include "RNameSpace.h"

class MXmlElement;

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

class RLightList : public std::vector<RLIGHT> {
public:
	bool Open(MXmlElement *pElement);
	bool Save(MXmlElement *pElement);
};

_NAMESPACE_REALSPACE2_END


#endif