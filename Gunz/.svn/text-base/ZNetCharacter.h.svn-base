#ifndef _ZNETCHARACTER_H
#define _ZNETCHARACTER_H

#include "MRTTI.h"
#include "ZCharacter.h"


class ZNetCharacter : public ZCharacter
{
	MDeclareRTTI;
private:
protected:
	void SetNetPosition(rvector& pos);
	virtual void OnDraw();
	virtual void OnUpdate(float fDelta);
public:
	ZNetCharacter();
	virtual ~ZNetCharacter();

	//virtual void  Update(float fDelta);
	void SetNetPosition(rvector& position, rvector& velocity, rvector& dir);
};




#endif