#include "winsock2.h"
#include "MObject.h"
#include "MStageAgent.h"
#include "MMatchAgent.h"


bool MStageAgent::Create(const MUID& uid)
{
	m_uidStage = uid;

	m_tmCreateTime = MMatchAgent::GetGlobalClockCount();

	return true;
}

void MStageAgent::Destroy()
{
	m_ObjUIDCaches.clear();
}

void MStageAgent::AddObject(const MUID& uid, const MAgentClient* pObj)
{
	m_ObjUIDCaches.Insert(uid, (void*)pObj);
}

void MStageAgent::RemoveObject(const MUID& uid)
{
	MUIDRefCache::iterator i = m_ObjUIDCaches.find(uid);
	if (i==m_ObjUIDCaches.end())
		return;

	m_ObjUIDCaches.erase(i);
}

bool MStageAgent::IsExistClient(const MUID& uid) {
	MUIDRefCache::iterator i = m_ObjUIDCaches.find(uid);
	if (i==m_ObjUIDCaches.end())
		return false;
	else
		return true;
}

bool MStageAgent::CheckDestroy()
{
	#define TIME_STAGE_CREATE_WAIT	(10*60*1000)	// 30 min
	if (MMatchAgent::GetGlobalClockCount() - m_tmCreateTime < TIME_STAGE_CREATE_WAIT)
		return false;

	if (m_ObjUIDCaches.size() <= 0)
		return true;
	else
		return false;
}

void MStageAgent::Tick(unsigned long nClock)
{
}
