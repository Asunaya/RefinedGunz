#pragma	once

#include <string>
#include "RTypes.h"

#include "RNameSpace.h"

_NAMESPACE_REALSPACE2_BEGIN

class ROcclusion {
public:
	ROcclusion();
	~ROcclusion();

	inline void CalcPlane() { D3DXPlaneFromPoints(&plane,pVertices,pVertices+1,pVertices+2); }

	int nCount;
	rvector *pVertices;
	rplane	*pPlanes;
	rplane plane;
	string	Name;
};

// TODO: Change std::list<ROcclusion*> -> std::vector<ROcclusion>
class ROcclusionList final : public std::list<ROcclusion*> {
public:
	~ROcclusionList();

	bool Open(class MXmlElement *pElement);
	bool Save(MXmlElement *pElement);

	void UpdateCamera(const rmatrix &matWorld, const rvector &cameraPos);
	bool IsVisible(const rboundingbox &bb) const;

};

_NAMESPACE_REALSPACE2_END
