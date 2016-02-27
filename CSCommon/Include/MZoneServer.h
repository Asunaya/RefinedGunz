#ifndef MZONESERVER_H
#define MZONESERVER_H

#include "winsock2.h"
#include "MCharacterDesc.h"
#include "MServer.h"
#include "MMap.h"
#include "MSpawnManager.h"
#include "RTypes.h"
#include "MObjectCharacter.h"
#include <vector>
using namespace std;

class MObject;
class MObjectCharacter;
class MObjectItem;

//class MCharacterList : public map<MUID, MObjectCharacter*>{};
//class MItemList : public map<MUID, MObjectItem*>{};
class MObjectList : public map<MUID, MObject*>{};

/*!
	Zone-Server:
	- 존 단위의 게임 오브젝트 관리 서버
	- 구동 순서
		-# MZoneServer::Create()
		-# MZoneServer::ConnectToMaster()
		-# MZoneServer::do_something()
		-# MZoneServer::DisconnectToMaster()
		-# MZoneServer::Destroy()
*/
class MZoneServer : public MServer{
private:
	static MZoneServer*	m_pInstance;		///< 전역 인스턴스
protected:
	MUID			m_Master;				///< 마스터 컨트롤러의 UID

	MMapManager		m_Maps;					///< 하나의 Zone안에 Map 리스트
	MSpawnManager	m_SpawnManager;			///< Spawn 관리

	bool			m_bRequestUIDToMaster;	///< UID 할당을 마스터 컨트롤러에게 요청했는가?

	//MCharacterList	m_Characters;
	//MItemList		m_Items;
	MObjectList		m_Objects;
	//MGridMap<MObjectCharacter*>	m_MapRefCharacter;
	//MGridMap<MObjectItem*>	m_MapRefItem;

	float			m_fSharedVisionRadius;	///< 시야 공유되는 영역

	MUIDRANGE		m_CommAllocRange;		///< 커뮤니케이터로 할당할수 있는 영역
	MUIDRANGE		m_NextCommAllocRange;	///< 커뮤니케이터로 다음에 할당할수 있는 영역

protected:
	/// Create()호출시에 불리는 함수
	virtual bool OnCreate(void);
	/// Destroy()호출시에 불리는 함수
	virtual void OnDestroy(void);
	/// 사용자 커맨드 등록
	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	/// 사용자 커맨드 처리
	virtual bool OnCommand(MCommand* pCommand);
	/// 사용자 루프
	virtual void OnRun(void);

	/// 초기화
	void OnInitialize();
	/// 로그인되었을때
	void OnLogin(MUID CommUID, MUID PlayerUID);
	/// 클라이언트에게 클럭을 맞추게 한다.
	void SetClientClockSynchronize(MUID& CommUID);
	void AssignPlayerCharacter(MUID& CommUID, MUID& PlayerUID);
	void ResponsePlayerCharacterAssign(MUID& CommUID, MObject* pObject);
	void ReleasePlayerCharacter(MUID& uid);
		
	/// Object들 사이의 상호인식 업데이트
	void ResponseObjectUpdate(MUID& TargetUID, MObject* pObject);
	void ResponseCharacterBasicAttr(MUID& TargetUID, MUID& RequestUID);

	/// Object의 Skill목록 전송
	void ResponseCharacterSkillList(const MUID& CommUID, const MUID& CharUID);

	/// Zone-Server의 세부 정보를 Target으로 전송
	void ResponseInfo(MUID& Target);
	void ResponseMapList(MUID& Target);

	/// DB에서 캐릭터 정보를 얻어서 로딩
	MObject* LoadCharacter(MUID& uid);
	/// 캐릭터 정보를 언로드. DB에 저장
	void UnloadCharacter(MUID& uid);

	/// UID로 캐릭터를 반환, 없으면 NULL을 반환한다.
	MObjectCharacter* FindCharacter(const MUID& uidChr);

	/// 캐릭터생성시 ActionPipe 구성
	void BuildActionPipe(MObjectCharacter* pCharacter);

	/// 대화 메시지
	int MessageSay(MUID& uid, char* pszSay);

	/// 오브젝트 생성
	int ObjectAdd(MUID* pAllocUID, int nObjID, int nMapID, int nNodeID, rvector& Pos);
	/// 오브젝트 제거
	int ObjectRemove(const MUID& uid, MObjectList::iterator* pNextItor);

	/// 오브젝트 상태변경
	int ObjectMoveMode(const MUID& uid, int nMoveMode);
	int ObjectMode(const MUID& uid, int nMode);
	int ObjectState(const MUID& uid, int nState);

	/// 오브젝트 이동
	//int ObjectMove(MUID& uid, unsigned long int nStartTime, unsigned long int nEndTime, rvector& StartPos, rvector& EndPos);
	/// 패스 리스트로 오브젝트 이동
	int ObjectMove(MUID& uid, void* pPathArray);
	/// 오브젝트 공격
	int ObjectAttack(MUID& uid, MUID& uidTarget);
	/// 오브젝트 스킬
	int ObjectSkill(const MUID& uid, const MUID& uidTarget, int nSkillID);
	/// Map의 Cell안에 있는 Object List 전송
	void ResponseObjectList(MUID& Target, int nMapID, int nNodeID);
	/// Map의 Object List 전송
	void ResponseObjectList(MUID& Target, int nMapID);

	/// 오브젝트가 갖고 있던 아이템을 장착 or 해제
	int ObjectEquipItem(MUID& TargetUID, MUID& ChrUID, MUID& ItemUID, MEquipmentSlot slot);

	/// Item 생성
	MObjectItem* CreateObjectItem(int nItemDescID);
	MObjectItem* FindItem(MUID& ItemUID);		///< UID를 이용하여 Item을 찾는다.

	void ResponseEquipItem(MUID& TargetUID, MUID& ItemUID, MEquipmentSlot slot);

	/// uid에게 아이템을 만들어준다.
	int AdminCreateItem(MUID uid, unsigned int nItemIndex);
	/// 캐릭터 정보를 알려준다.
	void ResponseAdminCharacterInfo(MUID& TargetUID, MUID& RequestUID);

	/// 새로운 UID 얻어내기
	virtual MUID UseUID(void);
public:
	MZoneServer(void);
	virtual ~MZoneServer(void);

	/// 전역 인스턴스 얻기
	static MZoneServer* GetInstance(void);

	/// 현재 클럭 얻어내기
	unsigned long int GetGlobalClockCount(void);

	/// 초기화
	bool Create(int nPort);
	/// 해제
	void Destroy(void);

	/// 커넥션을 받아들인 경우
	virtual int OnAccept(MCommObject* pCommObj);
	/// 커넥션이 이루어진 경우
	virtual int OnConnected(MUID* pTargetUID, MUID* pAllocUID, MCommObject* pCommObj);
	/// 커넥션이 종료된 경우
	virtual int OnDisconnect(MCommObject* pCommObj);

	/// Master와 연결한다.
	int ConnectToMaster(MCommObject* pCommObj);
	/// Master와 연결을 해제한다.
	void DisconnectToMaster(void);

	/// Master에게 UID할당을 요청한다.
	void RequestUIDToMaster(int nSize);

	void DebugTest();

public:
	/// UID로 오브젝트 얻어내기
	MObject* GetObject(const MUID& uid);
	/// UID로 캐릭터 오브젝트 얻어내기
	MObjectCharacter* GetObjectCharacter(const MUID& uid);
	/// UID로 아이템 오브젝트 얻어내기
	MObjectItem* GetObjectItem(const MUID& uid);
	/// CommUID로 캐릭터 오브젝트 얻어내기
	MObjectCharacter* GetPlayerByCommUID(const MUID& uid);

    /// Object들 사이의 상호인식 업데이트
	void UpdateObjectRecognition(MObject* pObject);

	/// Command를 Object의 Listener에게 전송
	void RouteToListener(MObject* pObject, MCommand* pCommand);
	/// Command를 전체 클라이언트로 전송
	void RouteToAllClient(MCommand* pCommand);
	/// Command를 Sender를 제외한 전체 클라이언트로 전송
	void RouteToAllExcludeSender(MUID& uidSender, MCommand* pCommand);
	/// Command를 지역으로 전송
	void RouteToRegion(int nMapID, const rvector& Pos, float fRange, MCommand* pCommand);

	int SpawnObject(int nSpawnID, int nObjID, int nMapID, int nNodeID, rvector& Pos);
	
};

#define DEFAULT_REQUEST_UID_SIZE	10000	///< UID 할당 요청 기본 단위


/// 맵 한개의 설명
struct MMAPDESCRIPTION{
	char	szName[MMAP_NAME_LENGTH];
};

/// 맵 리스트
struct MMAPLIST{
	int					nMapCount;	///< 맵 갯수
	MMAPDESCRIPTION*	pMaps;		///< 맵
};

/// Local Clock을 Global Clock으로 변환
unsigned long int ConvertLocalClockToGlobalClock(unsigned long int nLocalClock, unsigned long int nLocalClockDistance);
/// Global Clock을 Local Clock으로 변환
unsigned long int ConvertGlobalClockToLocalClock(unsigned long int nGlobalClock, unsigned long int nLocalClockDistance);


#endif