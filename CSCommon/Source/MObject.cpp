#include "stdafx.h"
#include "winsock2.h"
#include "MXml.h"
#include "MObject.h"

IMPLEMENT_RTTI(MObject)



MObjectCacheNode* MObjectCache::FindCacheNode(MObject* pObj)
{
	for(iterator i=begin(); i!=end(); i++){
		MObjectCacheNode* pNode = *i;
		if (pNode->m_pObject == pObj)
			return pNode;
	}
	return NULL;
}

void MObjectCache::Invalidate()
{
	m_nUpdateCount = 0;
	for(iterator i=begin(); i!=end(); i++){
		MObjectCacheNode* pNode = *i;
		pNode->m_CacheState = OBJECTCACHESTATE_EXPIRE;
		++m_nUpdateCount;
	}
}

void MObjectCache::Update(MObject* pObject)
{
	MObjectCacheNode* pFoundNode = NULL;
	for(iterator i=begin(); i!=end(); i++){
		MObjectCacheNode* pNode = *i;
		if (pNode->m_pObject == pObject)
			pFoundNode = pNode;
	}

	if (pFoundNode) {
		pFoundNode->m_CacheState = OBJECTCACHESTATE_KEEP;
		--m_nUpdateCount;
	} else {
		MObjectCacheNode* pNewNode = new MObjectCacheNode;
		pNewNode->m_pObject = pObject;
		pNewNode->m_CacheState = OBJECTCACHESTATE_NEW;
		++m_nUpdateCount;
		push_back(pNewNode);
	}
}

void MObjectCache::RemoveExpired()
{
	for(iterator i=begin(); i!=end();){
		MObjectCacheNode* pNode = *i;
		if (pNode->m_CacheState == OBJECTCACHESTATE_EXPIRE)
			i = erase(i);
		else
			++i;
	}
}
/*
rvector MObject::GetPos(unsigned long int nTime)
{
	MOBJECTPATH* pBegin = NULL;
	MOBJECTPATH* pEnd = NULL;
	for(vector<MOBJECTPATH>::iterator i=m_Path.begin(); i!=m_Path.end(); i++){
		MOBJECTPATH* pPath = &(*i);
		if(pPath->nTime<=nTime) pBegin = pPath;
		else if(pBegin!=NULL){
			pEnd = pPath;
			break;
		}
	}

	if(pBegin==NULL){
		if(m_Path.empty()==false) return m_Path[0].Pos;
		else return m_Pos;
	}
	else{
		if(pEnd==NULL){
			return pBegin->Pos;
		}
		else{
			_ASSERT(pBegin!=pEnd && pBegin->nTime!=pEnd->nTime);
			rvector Pos = pBegin->Pos + (pEnd->Pos - pBegin->Pos) * (float)(nTime - pBegin->nTime) / (float)(pEnd->nTime - pBegin->nTime);
			return Pos;
		}
	}
}
*/
MObject::MObject()
{
	m_UID = MUID::Invalid();
//	m_pMap = NULL;
//	m_Pos = rvector(0,0,0);
}
/*
MObject::MObject(MUID& uid, MMap* pMap, rvector& Pos)
{
	m_UID = uid;
	m_pMap = pMap;
	//m_StartPos = m_EndPos = m_Pos = Pos;
	//m_nStartTime = m_nEndTime = 0;
	m_Pos = Pos;
	if (m_pMap)
		m_hRef = m_pMap->Add(Pos.x, Pos.y, Pos.z, this);

	m_ObjectType = MOT_NONE;
}
*/
MObject::MObject(const MUID& uid)
{
	m_UID = uid;
//	m_pMap = NULL;
//	m_Pos = rvector(0,0,0);

//	m_ObjectType = MOT_NONE;
}
/*
MObject::~MObject(void)
{
	if (m_pMap)
		m_pMap->Del(m_hRef);
}
*/
const MUID MObject::GetUID(void) const
{
	return m_UID;
}
/*
void MObject::SetPos(unsigned long int nStartTime, unsigned long int nEndTime, rvector& StartPos, rvector& EndPos)
{
	_ASSERT(nStartTime<=nEndTime);

	m_Path.clear();
	//m_Path.reserve(2);
	m_Path.resize(2);
	m_Path[0].nTime = nStartTime;
	m_Path[0].Pos = StartPos;
	m_Path[1].nTime = nEndTime;
	m_Path[1].Pos = EndPos;
}*/
/*
void MObject::SetPath(MOBJECTPATH* p, int nCount)
{
	m_Path.clear();
	//m_Path.reserve(nCount);
	m_Path.resize(nCount);
	for(int i=0; i<nCount; i++){
		memcpy(&m_Path[i], &(p[i]), sizeof(MOBJECTPATH));
	}
}
*/
/*
void MObject::Tick(unsigned long int nTime)
{
	// 새로운 위치 계산
	m_Pos = GetPos(nTime);

	if (m_pMap)
		m_hRef = m_pMap->Move(m_Pos.x, m_Pos.y, m_Pos.z, this, m_hRef);
}
*/
void MObject::AddCommListener(MUID ListenerUID)
{
	if (IsCommListener(ListenerUID)) return;
	m_CommListener.push_back(ListenerUID);
}

void MObject::RemoveCommListener(MUID ListenerUID)
{
	for (list<MUID>::iterator i=m_CommListener.begin(); i!=m_CommListener.end(); i++) {
		MUID uid = *i;
		if (uid == ListenerUID) {
			m_CommListener.erase(i);
			return;
		}
	}
}

bool MObject::IsCommListener(MUID ListenerUID)
{
	for (list<MUID>::iterator i=m_CommListener.begin(); i!=m_CommListener.end(); i++) {
		MUID uid = *i;
		if (uid == ListenerUID)
			return true;
	}
	return false;
}