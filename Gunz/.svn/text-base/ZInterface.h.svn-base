#ifndef _ZINTERFACE_H
#define _ZINTERFACE_H

#include "ZIDLResource.h"
#include <windows.h>
#include <io.h>
#include "ZGame.h"
#include "RealSpace2.h"
#include "RBaseTexture.h"
#include "Mint.h"
#include "Mint4R2.h"

_USING_NAMESPACE_REALSPACE2

// interface 들의 base 클래스

class ZInterface : public MWidget {
public:
	ZInterface(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZInterface();

	virtual bool OnCreate();										// 초기화
	virtual void OnDestroy();										// destroy 시 불림.

	virtual void OnDraw(MDrawContext* pDC);

	virtual bool IsDone();											// 다음 인터페이스로 갈때 false를 리턴한다.

	virtual bool OnEvent(MEvent* pEvent, MListener* pListener)=0;	// interface 는 항상 이벤트를 처리한다.

protected:
	bool m_bDone;
};

#endif