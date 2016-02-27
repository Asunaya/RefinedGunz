#ifndef _RDUMMYLIST_H
#define _RDUMMYLIST_H

#include <list>
#include <string>

#include "RTypes.h"
#include "RToken.h"

using namespace std;

class MXmlElement;

#include "RNameSpace.h"
_NAMESPACE_REALSPACE2_BEGIN



// 더미, 더미 리스트
struct RDummy {
	string	szName;
	rvector Position;
	rvector Direction;
};

class RDummyList : public list<RDummy*> 
{
private:
	void Clear();
public:
	virtual ~RDummyList();

	bool Open(MXmlElement *pElement);
	bool Save(MXmlElement *pElement);
};












_NAMESPACE_REALSPACE2_END













#endif