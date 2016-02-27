#ifndef _MSMARTREFRESHIMPL_H
#define _MSMARTREFRESHIMPL_H

//#pragma once

#include "MSmartRefresh.h"

class MMatchObject;
class MMatchChannel;
class MMatchClan;


//// MRefreshCategoryChannel ////
class MRefreshCategoryChannelImpl : public MRefreshCategory {
protected:
	MMatchChannel*	m_pChannel;

protected:
	virtual bool OnUpdateChecksum(unsigned long nTick);

public:
	MRefreshCategoryChannelImpl(MMatchChannel* pChannel, int nCategory) : MRefreshCategory(nCategory) 
	{
		m_pChannel = pChannel;
	}
	~MRefreshCategoryChannelImpl()	{}

	MMatchChannel* GetMatchChannel()				{ return m_pChannel; }
};


//// MRefreshClientChannel ////
class MRefreshClientChannelImpl : public MRefreshClient {
protected:
	MMatchObject*	m_pObject;

protected:
	virtual bool OnSync(unsigned long nChecksum);

public:
	void SetMatchObject(MMatchObject* pObj)	{ m_pObject = pObj; }
	MMatchObject* GetMatchObject()			{ return m_pObject; }
};


//// MRefreshCategoryClanMember ////
class MRefreshCategoryClanMemberImpl : public MRefreshCategory {
protected:
	MMatchClan*		m_pClan;

protected:
	virtual bool OnUpdateChecksum(unsigned long nTick);

public:
	MRefreshCategoryClanMemberImpl(MMatchClan* pClan, int nCategory) : MRefreshCategory(nCategory) 
	{
		m_pClan = pClan;
	}
	~MRefreshCategoryClanMemberImpl()	{}

	MMatchClan* GetMatchClan()		{ return m_pClan; }
};


//// MRefreshClientClanMemberImpl ////
class MRefreshClientClanMemberImpl : public MRefreshClient {
protected:
	MMatchObject*	m_pObject;

protected:
	virtual bool OnSync(unsigned long nChecksum);

public:
	void SetMatchObject(MMatchObject* pObj)	{ m_pObject = pObj; }
	MMatchObject* GetMatchObject()			{ return m_pObject; }
};


#endif