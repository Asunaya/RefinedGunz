#ifndef MGROUP_H
#define MGROUP_H

#include "MWidget.h"
#include "MLookNFeel.h"

class MGroup;
class MGroupLook{
public:
	virtual void OnDraw(MGroup* pGroup, MDrawContext* pDC);
	virtual MRECT GetClientRect(MGroup* pGroup, MRECT& r);
};

class MGroup : public MWidget{
	DECLARE_LOOK(MGroupLook)
	DECLARE_LOOK_CLIENT()
public:
	MGroup(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MGroup(void);

#define MINT_GROUP	"Group"
	virtual const char* GetClassName(void){ return MINT_GROUP; }
};

#endif