#ifndef _MQUEST_MAP_H
#define _MQUEST_MAP_H


#include "MQuestConst.h"

/// 섹터 링크 정보
struct MQuestSectorLink
{
	char			szName[64];					///< 이름
	vector<int>		vecTargetSectors;			///< 연결될 수 있는 다음 섹터
	MQuestSectorLink() 
	{
		szName[0] = 0;
	}
};


/// 섹터 역링크 정보
struct MQuestSectorBacklink
{
	int nSectorID;
	int nLinkIndex;
};


/// 퀘스트 맵 섹터 정보
struct MQuestMapSectorInfo
{
	int								nID;								///< ID
	char							szTitle[64];						///< 섹터 이름
	bool							bBoss;								///< 보스방 여부
	int								nLinkCount;							///< 링크수
	MQuestSectorLink				Links[MAX_SECTOR_LINK];				///< 링크 정보
	vector<MQuestSectorBacklink>	VecBacklinks;						///< 역링크들
	int								nSpawnPointCount[MNST_END];			///< 스폰포인트 개수

	MQuestMapSectorInfo()
	{
		nID = -1;
		szTitle[0] = 0;
		nLinkCount = 0;
		bBoss = false;
		memset(nSpawnPointCount, 0, sizeof(nSpawnPointCount));
	}
};


/// 퀘스트 맵 세트 정보
struct MQuestMapsetInfo
{
	int				nID;									///< ID
	char			szTitle[64];							///< 세트 이름
	int				nLinkCount;								///< 링크수
	vector<int>		vecSectors;								///< 가지고 있는 섹터

	MQuestMapsetInfo()
	{
		nID = -1;
		szTitle[0] = 0;
		nLinkCount = 0;
	}
};

typedef	std::map<int, MQuestMapsetInfo*>		MQuestMapsetMap;
typedef	std::map<int, MQuestMapSectorInfo*>		MQuestMapSectorMap;

/// 퀘스트 맵 관리자
class MQuestMapCatalogue
{
private:
	// 멤버 변수
	MQuestMapsetMap			m_MapsetInfo;
	MQuestMapSectorMap		m_SectorInfo;

	// 함수
	void InsertMapset(MQuestMapsetInfo* pMapset);
	void InsertSector(MQuestMapSectorInfo* pSector);
	void ParseMapset(MXmlElement& element);
	void ParseMapsetSector1Pass(MXmlElement& elementMapset, MQuestMapsetInfo* pMapset);
	void ParseSector(MXmlElement& element, MQuestMapSectorInfo* pSector);
	void InitBackLinks();
public:
	MQuestMapCatalogue();													///< 생성자
	~MQuestMapCatalogue();													///< 소멸자

	void Clear();															///< 초기화
	bool ReadXml(const char* szFileName);									///< xml에서 맵정보를 읽는다. (서버용)
	bool ReadXml(MZFileSystem* pFileSystem,const char* szFileName);			///< xml에서 맵정보를 읽는다. (클라이언트용)
	void DebugReport();														///< 맵정보가 제대로 구성되었는지 확인한다.

	MQuestMapSectorInfo*	GetSectorInfo(int nSector);						///< 섹터 정보 반환
	MQuestMapsetInfo*		GetMapsetInfo(int nMapset);						///< 맵셋 정보 반환
	MQuestMapSectorInfo*	GetSectorInfoFromName(char* szSectorTitle);		///< 섹터 이름으로 섹터 정보 반환

	inline MQuestMapsetMap*		GetMapsetMap();
};


inline MQuestMapsetMap* MQuestMapCatalogue::GetMapsetMap()
{
	return &m_MapsetInfo;
}


#endif