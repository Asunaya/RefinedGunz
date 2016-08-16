#ifndef MMATCHOBJCACHE_H
#define MMATCHOBJCACHE_H

#include "MMatchGlobal.h"
#include <list>
#include <map>
using namespace std;

#include "MMatchObject.h"
#include "MUID.h"


class MCommand;
class MCommandCommunicator;
class MMatchObject;


struct MMatchObjCacheCostume
{
	MMatchSex			nSex;
	unsigned char		nHair;
	unsigned char		nFace;
	unsigned long int	nEquipedItemID[MMCIP_END];
};

class MMatchObjCache {
protected:
	MUID					m_uidObject;
	char					m_szName[32];
	char					m_szClanName[CLAN_NAME_LENGTH];
	char					m_nLevel;
	MMatchUserGradeID		m_nUGrade;
	MMatchPremiumGradeID	m_nPGrade;
	unsigned char			m_nPlayerFlags;		// 플레이어 속성(운영자숨김등) - MTD_PlayerFlags 사용
	unsigned int			m_nCLID;			// ClanID
	unsigned int			m_nEmblemChecksum;	// Emblem Checksum
	MMatchObjCacheCostume	m_Costume;
	
public:
	MMatchObjCache()				{ 
		m_szName[0] = NULL;
		m_nLevel = 0;
		m_nUGrade = MMUG_FREE;
		m_nPGrade = MMPG_FREE;
		memset(&m_Costume, 0, sizeof(MMatchObjCacheCostume));
		ResetFlag();
	}
	virtual ~MMatchObjCache()		{}

	MUID GetUID()					{ return m_uidObject; }

	char* GetName()			{ return m_szName; }
	char* GetClanName()		{ return m_szClanName; }
	int GetLevel()					{ return m_nLevel; }
	MMatchUserGradeID		GetUGrade()	{ return m_nUGrade; }
	MMatchPremiumGradeID	GetPGrade()	{ return m_nPGrade; }
	
	void SetInfo(const MUID& uid, const char* szName, const char* szClanName, int nLevel, 
				 MMatchUserGradeID nUGrade, MMatchPremiumGradeID nPGrade)
	{
		m_uidObject = uid;
		strcpy_safe(m_szName, szName);
		strcpy_safe(m_szClanName, szClanName);
		m_nLevel = (char)nLevel;
		m_nUGrade = nUGrade;
		m_nPGrade = nPGrade;
		m_nPlayerFlags = 0;
	}

	unsigned char GetFlags()				{ return m_nPlayerFlags; }
	void SetFlags(unsigned char nFlags)		{ m_nPlayerFlags = nFlags; }
	bool CheckFlag(unsigned char nFlagIdx)	{ return (m_nPlayerFlags & nFlagIdx) ? true:false; }
	void ResetFlag()						{ m_nPlayerFlags = 0; }
	void SetFlag(unsigned char nFlagIdx, bool bSet)	
	{ 
		if (bSet) m_nPlayerFlags |= nFlagIdx; 
		else m_nPlayerFlags &= (0xff ^ nFlagIdx);
	}
	unsigned int GetCLID()					{ return m_nCLID; }
	void SetCLID(unsigned int nCLID)		{ m_nCLID = nCLID; }
	unsigned int GetEmblemChecksum()		{ return m_nEmblemChecksum; }
	void SetEmblemChecksum(unsigned int nChecksum)	{ m_nEmblemChecksum = nChecksum; }

	void AssignCostume(MMatchObjCacheCostume* pCostume) { memcpy(&m_Costume, pCostume, sizeof(MMatchObjCacheCostume)); }
	MMatchObjCacheCostume* GetCostume() { return &m_Costume; }
};

class MMatchObjCacheList : public list<MMatchObjCache*>{};
class MMatchObjCacheMap : public map<MUID, MMatchObjCache*>{
public:
	void Insert(const MUID& uid, MMatchObjCache* pCache)	{	
		insert(value_type(uid, pCache));	
	}
};


enum MATCHCACHEMODE {
	MATCHCACHEMODE_ADD = 0,
	MATCHCACHEMODE_REMOVE,
	MATCHCACHEMODE_UPDATE,		// 모두 업데이트
	MATCHCACHEMODE_REPLACE		// 새로운 내용으로 교체
};

class MMatchObjectCacheBuilder {
	MMatchObjCacheList	m_ObjectCacheList;

public:
	MMatchObjectCacheBuilder();
	virtual ~MMatchObjectCacheBuilder();

	void AddObject(MMatchObject* pObj);
	void Reset();
	MCommand* GetResultCmd(MATCHCACHEMODE nType, MCommandCommunicator* pCmdComm);
};


#endif