#ifndef _ZMODULE_H
#define _ZMODULE_H

#include <map>
#include "MRTTI.h"

using namespace std;

class ZModuleContainer;

class ZModule {
	friend ZModuleContainer;
protected:
	ZModuleContainer*	m_pContainer;	// 이모듈을 담고있는 container

public:
	ZModule()				{}
	virtual ~ZModule()		{}

	void Active(bool bActive = true);	// (비)활성화 시킨다

	virtual int GetID() = 0;
	
	virtual bool Update(float fElapsed)	{ return true; }
	virtual void Destroy()	{}

	virtual void InitStatus() {}

protected:
	virtual void OnAdd()		{}
	virtual void OnRemove()		{}
	virtual void OnActivate()	{}
	virtual void OnDeactivate()	{}
};

typedef map <int, ZModule *> ZMODULEMAP;

class ZModuleContainer {
	friend ZModule;

	MDeclareRootRTTI;
	ZMODULEMAP m_Modules;
	ZMODULEMAP m_ActiveModules;

public:
	virtual ~ZModuleContainer();

	// bInitialActive : 활성화 초기치
	void AddModule(ZModule *pModule,bool bInitialActive = false);
	void RemoveModule(ZModule *pModule);

	// 등록된 모듈중 nID 모듈을 찾는다
	ZModule* GetModule(int nID);

	void ActiveModule(int nID, bool bActive = true);	// 특정 모듈을 (비)활성화 시킨다
	bool IsActiveModule(int nID);

	virtual void UpdateModules(float fElapsed);
	virtual void InitModuleStatus(void);
};

#define DECLARE_ID(_ID)		int GetID() { return _ID; }

// 모듈들의 선언
class ZModule_HPAP;
class ZModule_Movable;
class ZModule_Resistance;
class ZModule_ColdDamage;
class ZModule_FireDamage;
class ZModule_PoisonDamage;
class ZModule_LightningDamage;
class ZModule_Skills;

#endif	// of _ZOBJECTMODULE_H