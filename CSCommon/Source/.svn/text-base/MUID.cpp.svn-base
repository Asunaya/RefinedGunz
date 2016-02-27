#include "stdafx.h"
#include "MUID.h"
#include <windows.h>


MUID MUID::Invalid(void)
{
	return MUID(0,0);
}

MUIDRefMap::MUIDRefMap(void)
{
	m_CurrentMUID.SetZero();
}
MUIDRefMap::~MUIDRefMap(void)
{
}

MUID MUIDRefMap::Generate(void* pRef)
{
	m_CurrentMUID.Increase();
	insert(value_type(m_CurrentMUID, pRef));

	return m_CurrentMUID;
}
void* MUIDRefMap::GetRef(MUID& uid)
{
	iterator i = find(uid);
	if(i==end()) return NULL;
	return (*i).second;
}

void* MUIDRefMap::Remove(MUID& uid)
{
	iterator i = find(uid);
	if(i==end()) return NULL;
	void* pRef = (*i).second;
	erase(i);
	return pRef;
}

MUIDRANGE MUIDRefMap::Reserve(int nSize)
{
	MUIDRANGE r;
	r.Start = m_CurrentMUID.Increase();
	r.End = m_CurrentMUID.Increase(nSize-1);
	return r;
}

MUIDRANGE MUIDRefMap::GetReservedCount(void)
{
	MUIDRANGE r;
	r.Start = MUID(0, 2);
	r.End = m_CurrentMUID;
	return r;
}

/*
MUIDRefArray::MUIDRefArray(void)
{
	m_CurrentMUID.SetZero();
	push_back(NULL);
}
MUIDRefArray::~MUIDRefArray(void)
{
}

MUID MUIDRefArray::Generate(void* pRef)
{
	_ASSERT(size()==m_CurrentMUID.Low);
	_ASSERT(1000*1000<m_CurrentMUID.Low);	// 1M 보다 작게 설정한다.

	m_CurrentMUID.Increase();
	push_back(pRef);

	return m_CurrentMUID;
}
void* MUIDRefArray::GetRef(MUID& uid)
{
	if(uid.Low<0 || uid.Low>=size()) return NULL;
	return at(uid.Low);
}
*/

MUIDRefCache::MUIDRefCache(void)
{
}
MUIDRefCache::~MUIDRefCache(void)
{
}


void MUIDRefCache::Insert(const MUID& uid, void* pRef)
{
#ifdef _DEBUG
	if (GetRef(uid)) {
		_ASSERT(0);
		OutputDebugString("MUIDRefCache DUPLICATED Data. \n");
	}
#endif
	insert(value_type(uid, pRef));
}

void* MUIDRefCache::GetRef(const MUID& uid)
{
	iterator i = find(uid);
	if(i==end()) return NULL;
	return (*i).second;
}

void* MUIDRefCache::Remove(const MUID& uid)
{
	iterator i = find(uid);
	if(i==end()) return NULL;
	void* pRef = (*i).second;
	erase(i);
	return pRef;
}
