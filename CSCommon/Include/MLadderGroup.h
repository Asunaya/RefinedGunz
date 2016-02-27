#pragma once


#include <list>
#include <map>
using namespace std;

#include "MUID.h"


class MLadderGroup {
protected:
	int				m_nType;
	int				m_nID;
	int				m_nScore;
	int				m_nTickCount;
	bool			m_bWantBalancedMatching;

	// 클랜전만 사용 -------
	int				m_nCLID;
	int				m_nTotalCharLevel;
	int				m_nTotalContPoint;
	// ---------------------


	list<MUID>		m_uidPlayerList;

	unsigned int	m_nRegTime;
public:
	MLadderGroup(unsigned int nRegTime)
	{ 
		m_nType=0; m_nID=0; m_nScore=0; m_nCLID=0; m_nRegTime=nRegTime; 
		m_nTickCount = 0;
		m_nTotalCharLevel=0; m_nTotalContPoint=0;
		m_bWantBalancedMatching = false;
	}
	int GetLadderType()				{ return m_nType; }
	void SetLadderType(int nType)	{ m_nType = nType; }
	int GetID()						{ return m_nID; }
	void SetID(int nID)				{ m_nID = nID; }
	int GetScore()					{ return m_nScore; }
	int GetCLID()					{ return m_nCLID; }
	void SetCLID(int nCLID)			{ m_nCLID = nCLID; }
	unsigned int GetRegTime()		{ return m_nRegTime; }
	inline int GetCharLevel();
	inline int GetContPoint();
	int GetTickCount()				{ return m_nTickCount; }
	void SetBalancedMatching(bool bValue)		{ m_bWantBalancedMatching = bValue; }

	size_t GetPlayerCount()						{ return m_uidPlayerList.size(); }
	list<MUID>::iterator GetPlayerListBegin()	{ return m_uidPlayerList.begin(); }
	list<MUID>::iterator GetPlayerListEnd()		{ return m_uidPlayerList.end(); }

	void AddPlayer(MMatchObject* pObj) { 
		_ASSERT(GetID());
		pObj->SetLadderGroupID(GetID());
		pObj->SetLadderChallenging(true);
		if (IsEnabledObject(pObj))
		{
			m_nTotalCharLevel += pObj->GetCharInfo()->m_nLevel;
			m_nTotalContPoint += pObj->GetCharInfo()->m_ClanInfo.m_nContPoint;
		}
		m_uidPlayerList.push_back(pObj->GetUID()); 
	}
	unsigned long GetChecksum() {
		return (unsigned long)(m_nType + m_nID);
	}
	void UpdateTick() { 
		if (!m_bWantBalancedMatching) m_nTickCount++; 
	}	// 비슷한 실력의 클랜과만 대전을 원하면 틱을 올리지 않는다.
	bool IsSameGroup(MLadderGroup* pTarGroup)
	{
		if (m_nID == pTarGroup->GetID()) return true;
		if ((m_nCLID != 0) && (pTarGroup->GetCLID() != 0) && (m_nCLID == pTarGroup->GetCLID())) return true;
		return false;
	}
};

class MLadderGroupMap : public map<int, MLadderGroup*> {
protected:
//	int		m_idGenerate;
//	int GenerateID()	{ return ++m_idGenerate; }

public:
	MLadderGroupMap() { 
//		m_idGenerate = 0; 
	}
	virtual ~MLadderGroupMap()	{	}

	void Add(MLadderGroup* pGroup) { 
//		int nID = GenerateID();
//		pGroup->SetID(nID);
		_ASSERT(pGroup->GetID());
		insert(value_type(pGroup->GetID(), pGroup));
	}
	void Remove(int nID) {	// NO Delete the Data
		iterator i = find(nID);
		if(i==end()) return;

		MLadderGroup* pGroup = (*i).second;
		erase(i);
	}
	MLadderGroup* Find(int nID) {
		iterator i = find(nID);
		if(i==end()) return NULL;

		MLadderGroup* pGroup = (*i).second;
		return pGroup;
	}
};



// inline functions ////////////////////////////////////////////////////////////////////////
inline int MLadderGroup::GetCharLevel()
{ 
	int nPlayerCount = (int)GetPlayerCount();
	if (nPlayerCount > 0) return (m_nTotalCharLevel / nPlayerCount); 
	return 1;
}
inline int MLadderGroup::GetContPoint()
{ 
	int nPlayerCount = (int)GetPlayerCount();
	if (nPlayerCount > 0) return (m_nTotalContPoint / nPlayerCount);
	return 0;
}
