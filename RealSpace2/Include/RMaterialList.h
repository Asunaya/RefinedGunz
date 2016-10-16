#pragma once

#include <list>
#include <string>
#include "RTypes.h"
#include "RToken.h"
#include "RNameSpace.h"

class MXmlElement;

_NAMESPACE_REALSPACE2_BEGIN

struct RMATERIAL {
	std::string Name;
	rvector Diffuse;
	rvector Ambient;
	rvector Specular;
	float Power;
	string DiffuseMap;
	DWORD dwFlags;
};

class RMaterialList : public std::list<RMATERIAL*> {
public:
	virtual ~RMaterialList();

	bool Open(MXmlElement *pElement);
	bool Save(MXmlElement *pElement);

private:
	bool Open_Material(MXmlElement *pElement);
};

_NAMESPACE_REALSPACE2_END