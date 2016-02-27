#include "stdafx.h"
#include "ZModule.h"
#include "crtdbg.h"

void ZModule::Active(bool bActive)	// (비)활성화 시킨다
{
	m_pContainer->ActiveModule(GetID(),bActive);
}


//////////////////////////////////////////////
//// ZModuleContainer

MImplementRootRTTI(ZModuleContainer);

ZModuleContainer::~ZModuleContainer()
{
	_ASSERT(m_Modules.empty());
}

ZModule *ZModuleContainer::GetModule(int nID)
{
	ZMODULEMAP::iterator i = m_Modules.find(nID);

	if(m_Modules.end()!=i) {
		return i->second;
	}

	return NULL;
}

void ZModuleContainer::AddModule(ZModule *pModule,bool bInitialActive)
{
	ZMODULEMAP::iterator i = m_Modules.find(pModule->GetID());
	if(i!=m_Modules.end()) {
		_ASSERT(0);	// 중복되었다
	}

	m_Modules.insert(ZMODULEMAP::value_type(pModule->GetID(),pModule));
	if(bInitialActive)
		ActiveModule(pModule->GetID());

	pModule->m_pContainer = this;
	pModule->OnAdd();
}

void ZModuleContainer::RemoveModule(ZModule *pModule)
{
	if (pModule == NULL) return;		// 널값이면 리턴
	ZMODULEMAP::iterator i = m_Modules.find(pModule->GetID());
	if(i==m_Modules.end()) return;	// 없는 모듈이다

	ActiveModule(pModule->GetID(),false);

	m_Modules.erase(i);
	pModule->OnRemove();
}

void ZModuleContainer::ActiveModule(int nID, bool bActive)	// 특정 모듈을 (비)활성화 시킨다
{
	// 활성화 시킨다
	if(bActive) {
		if(IsActiveModule(nID)) return;	// 이미 활성화 되어있다

		ZMODULEMAP::iterator i = m_Modules.find(nID);
		if(i!= m_Modules.end()) {
			i->second->OnActivate();
			m_ActiveModules.insert(ZMODULEMAP::value_type(nID,i->second));
		}
	}else{
	// 비활성화 시킨다
		ZMODULEMAP::iterator i = m_ActiveModules.find(nID);
		if(i==m_ActiveModules.end()) return; // 이미 비활성화 되어있다
		i->second->OnDeactivate();
		m_ActiveModules.erase(i);
	}
}

bool ZModuleContainer::IsActiveModule(int nID)
{
	ZMODULEMAP::iterator i = m_ActiveModules.find(nID);
	if(i!=m_ActiveModules.end()) return true;

	return false;
}


void ZModuleContainer::UpdateModules(float fElapsed)
{
	// 활성화 되어있는 모듈만 업데이트를 불러준다
	for(ZMODULEMAP::iterator i=m_ActiveModules.begin();i!=m_ActiveModules.end();i++) {
		ZModule *pModule = i->second;
		if(!pModule->Update(fElapsed)) {
			i = m_ActiveModules.erase(i);
		}
	}
}

void ZModuleContainer::InitModuleStatus(void)
{
	for(ZMODULEMAP::iterator i=m_Modules.begin();i!=m_Modules.end();i++) {
		ZModule *pModule = i->second;
		pModule->InitStatus();
	}
}