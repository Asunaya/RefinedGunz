#ifndef MMONITOR_H
#define MMONITOR_H

#include "MClient.h"
#include "MMap.h"

class MMonitorCommandProcessor;
class MCommand;
struct MOBJECTBASICINFO;
struct MOBJECTCACHEINFO;
struct MBLOB_BASICINFO_UPDATE;


/// 일반 커뮤니케이터와 의사 소통이 가능한 범용 Monitor
class MMonitor : public MClient{
protected:
	MMapManager	m_MapManager;	///< 모니터에서 참고할 맵

protected:
	// socket event
	virtual bool OnSockConnect(SOCKET sock);
	virtual bool OnSockDisconnect(SOCKET sock);
	virtual bool OnSockRecv(SOCKET sock, char* pPacket, DWORD dwSize);
	virtual void OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode);



	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	virtual bool OnCommand(MCommand* pCommand);
	virtual void BrowseMap(const char* szMapName) = 0;
	virtual void OnResponseObjectList(int nMapID, int nCellID, MOBJECTBASICINFO* pObjBIArray, int nObjBIArrayCount);
	virtual void OnResponseObjectList(int nMapID, MOBJECTBASICINFO* pObjBIArray, int nObjBIArrayCount);
	virtual void OnResponseObjectCacheUpdate(MOBJECTCACHEINFO* pObjBIArray, int nObjBIArrayCount);
	virtual void OnResponseObjectBasicInfoUpdate(MBLOB_BASICINFO_UPDATE* pObjBlArray, int nObjBlArrayCount);

	/// 로컬 정보를 출력한다.
	void OutputLocalInfo(void);
public:
	MMonitor(void);

	/*
	bool Create(int nPort);
	void Destroy(void);
	*/

	/// 커뮤니케이터 레벨의 로그 메세지를 모니터 출력으로 리다이렉션
	virtual void Log(const char* szLog){ OutputMessage(szLog, MZMOM_LOCALREPLY); }

	virtual int OnConnected(MUID* pTargetUID, MUID* pAllocUID, MCommObject* pCommObj);

	/// 연결되어 있는 서버
	MUID GetConnectedServer(void){ return m_Server; }
};



// 모니터와 서버가 분리되어 있지 않기 때문에 임시 정의된 함수들
class MCommandCommunicator;
void MTempSetZoneServer(MCommandCommunicator* pServer);
MCommandCommunicator* MTempGetZoneServer(void);
void MTempSetMaster(MCommandCommunicator* pMaster);
MCommandCommunicator* MTempSetMaster(void);


#endif
