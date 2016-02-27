#ifndef MOBJECT_H
#define MOBJECT_H

#include "MUID.h"
//#include "RTypes.h"
#include "MGridMap.h"
#include "MObjectTypes.h"

using namespace std;
#include <algorithm>

class MMap;
class MObject;

typedef MGridMap<MObject*>	MObjectGridMap;


/// 변수를 이용한 RTTI 구현. Class Declaration
#define DECLARE_RTTI()								public: static const char* _m_RTTI_szClassName; virtual const char* GetClassName(void){ return _m_RTTI_szClassName; }
/// 변수를 이용한 RTTI 구현. Class Implementation
#define IMPLEMENT_RTTI(_Class)						const char* _Class::_m_RTTI_szClassName = #_Class;
/// 같은 클래스타입인가?
#define ISEQUALCLASS(_Class, _InstancePointer)		(_Class::_m_RTTI_szClassName==(_InstancePointer)->GetClassName())	// 포인터 비교

//#define SAFECAST(_Class, _InstancePointer)			_InstancePointer-
/*
/// 이동에 관한 정보
struct MOBJECTPATH{
	unsigned long int	nTime;		///< 해당 시간
	rvector				Pos;		///< 해당 위치
	int					nPathNode;	///< Path Node ID
};
#define MOBJECTMOVE	MOBJECTPATH	// 임시, 나중에 이름을 다 바꿔준다.
*/
class MObject;
enum OBJECTCACHESTATE {	OBJECTCACHESTATE_KEEP, OBJECTCACHESTATE_NEW, OBJECTCACHESTATE_EXPIRE };
class MObjectCacheNode {
public:
	MObject*			m_pObject;
	OBJECTCACHESTATE	m_CacheState;
};
class MObjectCache : public list<MObjectCacheNode*> {
	int		m_nUpdateCount;	// New, Expire만 카운트
public:
	int GetUpdateCount() { return m_nUpdateCount; }
	MObjectCacheNode* FindCacheNode(MObject* pObj);
	void Invalidate();
	void Update(MObject* pObject);
	void RemoveExpired();
};


/// 서버내에 존재하는 모든 오브젝트의 Abstract Class
class MObject{
protected:
	MUID			m_UID;

	MObjectType		m_ObjectType;		///< 우선 이것을 가지고 PC, NPC를 판별한다.
private:
	//rvector			m_Pos;		///< 위치
	//rvector				m_StartPos, m_EndPos, m_Pos;
	//unsigned long int	m_nStartTime, m_nEndTime;

//	vector<MOBJECTPATH>	m_Path;		///< 패스
//	rvector				m_Pos;		///< 현재 위치
//	int					m_nSpawnID;

//protected:
//	MMap*					m_pMap;		///< 현재 속해있는 맵
//	MObjectGridMap::HREF	m_hRef;		///< 맵에서의 위치 핸들


public:
	MObjectCache	m_ObjectCache;
	list<MUID>		m_CommListener;

	/*
protected:
	void UpdateObjectCache(void);
	*/

protected:
	/// 시간에 따른 위치 얻어내기
//	rvector GetPos(unsigned long int nTime);

public:
	MObject();
//	MObject(MUID& uid, MMap* pMap, rvector& Pos);	// Map Navication Object
	MObject(const MUID& uid);	// No Map Object
	virtual ~MObject(void)	{};

	const MUID GetUID(void) const;

	/// 현재 위치 설정
//	void SetPos(rvector& Pos){ SetPos(0, 0, Pos, Pos); }
	/// 시간에 따른 위치 지정
//	void SetPos(unsigned long int nStartTime, unsigned long int nEndTime, rvector& StartPos, rvector& EndPos);
	/// 패스로 지정
//	void SetPath(MOBJECTPATH* p, int nCount);
	/// 현재 시간의 오브젝트 위치 얻기
//	const rvector GetPos(void){ return m_Pos; }
	/// SpawnID 얻기 (Spawn Object 아니면 0)
//	int GetSpawnID() { return m_nSpawnID; }
//	void SetSpawnID(int nSpawnID) { m_nSpawnID = nSpawnID; }

	void SetObjectType(MObjectType type) { m_ObjectType = type; }
	MObjectType GetObjectType()	{ return m_ObjectType; }

	virtual bool CheckDestroy(int nTime) { return false; }

	/// 틱 처리
	virtual void Tick(unsigned long int nTime)	{}

	/// 현재 맵 설정
//	void SetMap(MMap* pMap){ m_pMap = pMap; }
	/// 현재 속해있는 맵 얻기
//	MMap* GetMap(void){ return m_pMap; }
	

	void AddCommListener(MUID ListenerUID);
	void RemoveCommListener(MUID ListenerUID);
	bool IsCommListener(MUID ListenerUID);
	bool HasCommListener() { if (m_CommListener.size() > 0) return true; else return false; }
	const MUID GetCommListener() { 
		if (HasCommListener())
			return *m_CommListener.begin(); 
		else
			return MUID(0,0);
	}

	void InvalidateObjectCache() { m_ObjectCache.Invalidate(); } 
	void UpdateObjectCache(MObject* pObject) { m_ObjectCache.Update(pObject); }
	void ExpireObjectCache(MObject* pObject) { 
		MObjectCacheNode* pNode = m_ObjectCache.FindCacheNode(pObject);
		if (pNode == NULL) return;
		pNode->m_CacheState = OBJECTCACHESTATE_EXPIRE;
	}
	void RemoveObjectCacheExpired() { m_ObjectCache.RemoveExpired(); }
	int GetObjectCacheUpdateCount() { return m_ObjectCache.GetUpdateCount(); }

	DECLARE_RTTI()
};

/*
/// 보이는 모든 오브젝트. 맵에 위치시킬수 있는 위치와 크기를 가진다.
class MObjectVisible : public MObject{
public:
	MObjectVisible(MUID& uid) : MObject(uid) {}
	virtual ~MObjectVisible(void){}

	DECLARE_RTTI()
};
*/


/*
/// 오브젝트의 기본 정보
struct MOBJECTBASICINFO{
	MUID	uid;
	float	x, y, z;
};

/// 오브젝트의 캐쉬 정보
struct MOBJECTCACHEINFO{
	MUID				uid;
	float				x, y, z;

	MObjectType		nObjectType;		// 오브젝트 타입 - Character, Item
//	union {								// 오브젝트 타입에 따라서 달라질 수 있다.
//		MCharacterType	nChrType;			// 캐릭터 타입 - Player, NPC
//	};

	OBJECTCACHESTATE	nState;
};

/// 캐릭터 속성 기본 정보
struct MCharacterAttrBasicInfo{
	MUID		uid;
	int			nHP;
	int			nEN;
};


/// 걸어가는데 걸리는 시간 얻기(나중에 캐릭터에 대한 정보가 파라미터로 들어간다.)
unsigned long int MGetTimeToWalk(rvector& Distance);
/// 턴하는데 걸리는 시간 얻기
unsigned long int MGetTimeToTurn(rvector& Dir1, rvector& Dir2);
*/
#endif