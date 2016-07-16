#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MMatchNotify.h"
#include "Msg.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MAsyncDBJob_FriendList.h"
#include "MAsyncDBJob_UpdateCharInfoData.h"
#include "MAsyncDBJob_GetLoginInfo.h"
#include "MMatchWorldItemDesc.h"
#include "MMatchQuestMonsterGroup.h"
#include "RTypes.h"
#include "MMatchChatRoom.h"
#include "MMatchUtil.h"
#include "MLadderStatistics.h"
#include "MMatchSchedule.h"
#include <winbase.h>
#include "MMatchGameType.h"
#include "MQuestFormula.h"
#include "MQuestItem.h"
#include "MSacrificeQItemTable.h"
#include "MMatchPremiumIPCache.h"
#include "MCommandBuilder.h"
#include "MMatchLocale.h"
#include "MMatchEvent.h"
#include "MMatchEventManager.h"
#include "MMatchEventFactory.h"
#include "../../MatchServer/HSHIELD/AntiCpSvrFunc.h"
#include "HitRegistration.h"
#include "MUtil.h"

#define DEFAULT_REQUEST_UID_SIZE		4200000000	///< UID 할당 요청 기본 단위
#define DEFAULT_REQUEST_UID_SPARE_SIZE	10000		///< UID 남은 갯수
#define DEFAULT_ASYNCPROXY_THREADPOOL	6
#define MAXUSER_WEIGHT					30

#define MAX_DB_QUERY_COUNT_OUT			5		// 쿼리가 5번이상 실패하면 Shutdown


#define MATCHSERVER_DEFAULT_UDP_PORT	7777

#define FILENAME_ITEM_DESC				"zitem.xml"
#define FILENAME_SHOP					"shop.xml"
#define FILENAME_CHANNEL				"channel.xml"
#define FILENAME_SHUTDOWN_NOTIFY		"shutdown.xml"
#define FILENAME_WORLDITEM_DESC			"worlditem.xml"
#define FILENAME_MONSTERGROUP_DESC		"monstergroup.xml"
#define FILENAME_CHANNELRULE			"channelrule.xml"

MMatchServer* MMatchServer::m_pInstance = NULL;
//extern void RcpLog(const char *pFormat,...);

////////////////////////////////////
void RcpLog(const char *pFormat,...)
{
	char szBuf[256];

	va_list args;

	va_start(args,pFormat);
	vsprintf_safe(szBuf, pFormat, args);
	va_end(args);

	int nEnd = (int)strlen(szBuf)-1;
	if ((nEnd >= 0) && (szBuf[nEnd] == '\n')) {
		szBuf[nEnd] = NULL;
		strcat_s(szBuf, "\n");
	}
	OutputDebugString(szBuf);
}
////////////////////////////////////


/////////////////////////////////////////////////////////
class MPointerChecker
{
private:
	void* m_pPointer;
	bool	m_bPrinted;
public:
	MPointerChecker() : m_bPrinted(false), m_pPointer(0) {  }
	void Init(void* pPointer) { m_pPointer = pPointer; }
	void Check(void* pPointer, int nState, int nValue)
	{
		if ((pPointer != m_pPointer) && (!m_bPrinted))
		{
			m_bPrinted = true;
			mlog("### Invalid Pointer(%x, %x) - State(%d) , Value(%d) ###\n", m_pPointer, pPointer, nState, nValue);
		}
	}
};

#define NUM_CHECKPOINTER	3
static MPointerChecker g_PointerChecker[NUM_CHECKPOINTER];


void _CheckValidPointer(void* pPointer1, void* pPointer2, void* pPointer3, int nState, int nValue)
{
	if (pPointer1 != NULL) g_PointerChecker[0].Check(pPointer1, nState, nValue);
	if (pPointer2 != NULL) g_PointerChecker[1].Check(pPointer2, nState, nValue);
	if (pPointer3 != NULL) g_PointerChecker[2].Check(pPointer3, nState, nValue);
}



/////////////////////////////////////////////////////////

void CopyCharInfoForTrans(MTD_CharInfo* pDest, MMatchCharInfo* pSrcCharInfo, MMatchObject* pSrcObject)
{
	memset(pDest, 0, sizeof(MTD_CharInfo));

	if (pSrcCharInfo)
	{
		strcpy_safe(pDest->szName, pSrcCharInfo->m_szName);
		strcpy_safe(pDest->szClanName, pSrcCharInfo->m_ClanInfo.m_szClanName);
		pDest->nClanGrade = pSrcCharInfo->m_ClanInfo.m_nGrade;
		pDest->nClanContPoint = pSrcCharInfo->m_ClanInfo.m_nContPoint;

		pDest->nCharNum = (char)pSrcCharInfo->m_nCharNum;
		pDest->nLevel = (unsigned short)pSrcCharInfo->m_nLevel;
		pDest->nSex = (char)pSrcCharInfo->m_nSex;
		pDest->nFace = (char)pSrcCharInfo->m_nFace;
		pDest->nHair = (char)pSrcCharInfo->m_nHair;

		pDest->nXP = pSrcCharInfo->m_nXP;
		pDest->nBP = pSrcCharInfo->m_nBP;
		pDest->fBonusRate = pSrcCharInfo->m_fBonusRate;
		pDest->nPrize = (unsigned short)pSrcCharInfo->m_nPrize;
		pDest->nHP = (unsigned short)pSrcCharInfo->m_nHP;
		pDest->nAP = (unsigned short)pSrcCharInfo->m_nAP;
		pDest->nMaxWeight = (unsigned short)pSrcCharInfo->m_nMaxWeight;
		pDest->nSafeFalls = (unsigned short)pSrcCharInfo->m_nSafeFalls;
		pDest->nFR = (unsigned short)pSrcCharInfo->m_nFR;
		pDest->nCR = (unsigned short)pSrcCharInfo->m_nCR;
		pDest->nER = (unsigned short)pSrcCharInfo->m_nER;
		pDest->nWR = (unsigned short)pSrcCharInfo->m_nWR;

		for (int i = 0; i < MMCIP_END; i++)
		{
			if (pSrcCharInfo->m_EquipedItem.IsEmpty(MMatchCharItemParts(i)))
			{
				pDest->nEquipedItemDesc[i] = 0;
			}
			else
			{
				MMatchItem* pItem = pSrcCharInfo->m_EquipedItem.GetItem(MMatchCharItemParts(i));
				MMatchItemDesc* pItemDesc = pItem->GetDesc();
				if (pItemDesc)
				{
					pDest->nEquipedItemDesc[i] = pItemDesc->m_nID;
				}
			}

		}
	}


	if (pSrcObject)
	{
		pDest->nUGradeID = pSrcObject->GetAccountInfo()->m_nUGrade;
	}
	else
	{
		pDest->nUGradeID = MMUG_FREE;
	}

	pDest->nClanCLID = pSrcCharInfo->m_ClanInfo.m_nClanID;
}

void CopyCharInfoDetailForTrans(MTD_CharInfo_Detail* pDest, MMatchCharInfo* pSrcCharInfo, MMatchObject* pSrcObject)
{
	memset(pDest, 0, sizeof(MTD_CharInfo_Detail));

	if (pSrcCharInfo)
	{
		strcpy_safe(pDest->szName, pSrcCharInfo->m_szName);
		strcpy_safe(pDest->szClanName, pSrcCharInfo->m_ClanInfo.m_szClanName);
		pDest->nClanGrade = pSrcCharInfo->m_ClanInfo.m_nGrade;
		pDest->nClanContPoint = pSrcCharInfo->m_ClanInfo.m_nContPoint;

		pDest->nLevel = (unsigned short)pSrcCharInfo->m_nLevel;
		pDest->nSex = (char)pSrcCharInfo->m_nSex;
		pDest->nFace = (char)pSrcCharInfo->m_nFace;
		pDest->nHair = (char)pSrcCharInfo->m_nHair;
		pDest->nXP = pSrcCharInfo->m_nXP;
		pDest->nBP = pSrcCharInfo->m_nBP;

		pDest->nKillCount = pSrcCharInfo->m_nTotalKillCount;
		pDest->nDeathCount = pSrcCharInfo->m_nTotalDeathCount;


		unsigned long int nNowTime = MMatchServer::GetInstance()->GetTickTime();

		// 접속시간
		pDest->nConnPlayTimeSec = MGetTimeDistance(pSrcCharInfo->m_nConnTime, nNowTime) / 1000;
		pDest->nTotalPlayTimeSec = pDest->nConnPlayTimeSec + pSrcCharInfo->m_nTotalPlayTimeSec;




		//
		// 아이템셋
		for (int i = 0; i < MMCIP_END; i++)
		{
			if (pSrcCharInfo->m_EquipedItem.IsEmpty(MMatchCharItemParts(i)))
			{
				pDest->nEquipedItemDesc[i] = 0;
			}
			else
			{
				pDest->nEquipedItemDesc[i] = pSrcCharInfo->m_EquipedItem.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nID;
			}
		}
	}


	if (pSrcObject)
	{
		pDest->nUGradeID = pSrcObject->GetAccountInfo()->m_nUGrade;
	}
	else
	{
		pDest->nUGradeID = MMUG_FREE;
	}

	pDest->nClanCLID = pSrcCharInfo->m_ClanInfo.m_nClanID;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsExpiredBlockEndTime( const SYSTEMTIME& st )
{
	SYSTEMTIME stLocal;
	GetLocalTime( &stLocal );


	if( st.wYear < stLocal.wYear )
		return true;
	else if( st.wYear > stLocal.wYear )
		return false;

	if( st.wMonth < stLocal.wMonth )
		return true;
	else if( st.wMonth > stLocal.wMonth )
		return false;

	if( st.wDay < stLocal.wDay )
		return true;
	else if( st.wDay > stLocal.wDay )
		return false;

	if( st.wHour < stLocal.wHour )
		return true;
	else if( st.wHour > stLocal.wHour )
		return false;

	if( st.wMinute < stLocal.wMinute )
		return true;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MMatchServer::MMatchServer(void) : m_pScheduler( 0 )
{
	_ASSERT(m_pInstance==NULL);
	m_pInstance = this;
	m_nTickTime = 0;
	m_dwBlockCount = 0;
	m_dwNonBlockCount = 0;

	m_This = (MATCHSERVER_UID);

	SetName("MATCHSERVER");	// For Debug
	SetDefaultChannelName("PUBLIC-");

	m_bCreated = false;

	m_pAuthBuilder = NULL;

	// m_pScheduler = 0;

	// 스트링 리소스는 제일 먼저 인스턴스를 생성해놔야 한다.
	MMatchStringResManager::MakeInstance();

	// 포인터 침범 디버그코드
	m_checkMemory1 = m_checkMemory2 = m_checkMemory3 = m_checkMemory4 = m_checkMemory5 = m_checkMemory6 =
	m_checkMemory7 = m_checkMemory8 = m_checkMemory9 = m_checkMemory10 = m_checkMemory11 = m_checkMemory12 =
	m_checkMemory13 = m_checkMemory14 = m_checkMemory15 = m_checkMemory16 = m_checkMemory17 = m_checkMemory18 =
	m_checkMemory19 = m_checkMemory20 = m_checkMemory21 = CHECKMEMORYNUMBER;

#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
		m_HSCheckCounter = 0L;
#endif
}

static bool g_bPrintedInvalidMemory = false;

void MMatchServer::CheckMemoryTest(int nState, int nValue)
{
#define CHECK(n) if(m_checkMemory##n!=CHECKMEMORYNUMBER) { g_bPrintedInvalidMemory=true; mlog("***WARNING !! m_checkMemory" #n " is corrupted. State(%d), Value(%d)\n", nState, nValue); }

	if (g_bPrintedInvalidMemory) return;

	CHECK(1);    
	CHECK(2);    
	CHECK(3);    
	CHECK(4);    
	CHECK(5);    
	CHECK(6);    
	CHECK(7);    
	CHECK(8);    
	CHECK(9);    
	CHECK(10);    
	CHECK(11);    
	CHECK(12);    
	CHECK(13);    
	CHECK(14);    
	CHECK(15);    
	CHECK(16);    
	CHECK(17);    
	CHECK(18);    
	CHECK(19);    
	CHECK(20);    
	CHECK(21);    
}

MMatchServer::~MMatchServer(void)
{
	CheckMemoryTest();

	if (m_pAuthBuilder) {
		delete m_pAuthBuilder;
		m_pAuthBuilder = NULL;
	}

	Destroy();

}


bool MMatchServer::LoadInitFile()
{
	if (!MGetServerConfig()->Create())
	{
		LOG(LOG_ALL, "Load Config File Failed");
		return false;
	}

	if( !InitLocale() ){
		LOG(LOG_ALL, "Locale 설정 실패." );
		return false;
	}


	// 제한맵서버일 경우 플레이가능한 맵 화면에 출력
	if (MGetServerConfig()->IsResMap())
	{
		char szText[512];
		sprintf_safe(szText, "Enable Maps: ");
		for (int i = 0; i < MMATCH_MAP_MAX; i++)
		{
			if (MGetServerConfig()->IsEnableMap(MMATCH_MAP(i)))
			{
				strcat_s(szText, g_MapDesc[i].szMapName);
				strcat_s(szText, ", ");
			}
		}
		LOG(LOG_ALL, szText);
	}

	if (!MMatchFormula::Create()) 
	{
		LOG(LOG_ALL, "Open Formula Table FAILED");
		return false;
	}
	if (!MQuestFormula::Create()) 
	{
		LOG(LOG_ALL, "Open Quest Formula Table FAILED");
		return false;
	}

	if (!MGetMatchWorldItemDescMgr()->ReadXml(FILENAME_WORLDITEM_DESC))
	{
		Log(LOG_ALL, "Read World Item Desc Failed");
		return false;
	}
/*
	if (!MGetNPCGroupMgr()->ReadXml(FILENAME_MONSTERGROUP_DESC))
	{
		Log(LOG_ALL, "Read Monster Group Desc Failed");
		return false;
	}
*/
#ifdef _QUEST_ITEM
	if( !GetQuestItemDescMgr().ReadXml(QUEST_ITEM_FILE_NAME) )
	{
		Log( LOG_ALL, "Load quest item xml file failed." );
		return false;
	}
	if( !MSacrificeQItemTable::GetInst().ReadXML(SACRIFICE_TABLE_XML) )
	{
		Log( LOG_ALL, "Load sacrifice quest item table failed." );
		return false;
	}
#endif
	// 클랜전 서버일 경우만 실행하는 초기화
	if (MGetServerConfig()->GetServerMode() == MSM_CLAN)
	{
		GetLadderMgr()->Init();

#ifdef _DEBUG
		//GetLadderMgr()->GetStatistics()->PrintDebug();
		
#endif
	}

	if (!MGetMapsWorldItemSpawnInfo()->Read())
	{
		Log(LOG_ALL, "Read World Item Spawn Failed");
		return false;
	}

	if (!MGetMatchItemDescMgr()->ReadXml(FILENAME_ITEM_DESC))
	{
		Log(LOG_ALL, "Read Item Descriptor Failed");
		return false;
	}

	if(!GetQuest()->Create())
	{
		Log(LOG_ALL, "Read Quest Desc Failed");
		return false;
	}

	if (!MGetMatchShop()->Create(FILENAME_SHOP))
	{
		Log(LOG_ALL, "Read Shop Item Failed");
		return false;
	}
	if (!LoadChannelPreset()) 
	{
		Log(LOG_ALL, "Load Channel preset Failed");
		return false;
	}
	if (!m_MatchShutdown.LoadXML_ShutdownNotify(FILENAME_SHUTDOWN_NOTIFY))
	{
		Log(LOG_ALL, "Load Shutdown Notify Failed");
		return false;
	}
	if (!MGetChannelRuleMgr()->ReadXml(FILENAME_CHANNELRULE))
	{
		Log(LOG_ALL, "Load ChannelRule.xml Failed");
		return false;
	}

	unsigned long nItemChecksum = MGetMZFileChecksum(FILENAME_ITEM_DESC);
	SetItemFileChecksum(nItemChecksum);


	if( !InitEvent() )
	{
		Log(LOG_ALL, "init event failed.\n");
		return false;
	}
#ifdef _DEBUG
	CheckItemXML();
	CheckUpdateItemXML();	
#endif

	return true;
}

bool MMatchServer::LoadChannelPreset()
{
	#define MTOK_DEFAULTCHANNELNAME		"DEFAULTCHANNELNAME"
	#define MTOK_DEFAULTRULENAME		"DEFAULTRULENAME"
	#define MTOK_CHANNEL				"CHANNEL"

	MXmlDocument	xmlIniData;
	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(FILENAME_CHANNEL))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, childElement;
	char szTagName[256];
	char szBuf[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		childElement = rootElement.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, MTOK_CHANNEL))
		{
			char szRuleName[256]="";
			int nMaxPlayers = 0;
			int nLevelMin = -1;
			int nLevelMax = -1;

			childElement.GetAttribute(szBuf, "name");
			if (childElement.GetAttribute(szRuleName, "rule") == false)
				strcpy_safe(szRuleName, GetDefaultChannelRuleName());
			childElement.GetAttribute(&nMaxPlayers, "maxplayers");
			childElement.GetAttribute(&nLevelMin, "levelmin");
			childElement.GetAttribute(&nLevelMax, "levelmax");

			MUID uidChannel;
			ChannelAdd(MGetStringResManager()->GetStringFromXml(szBuf), 
				szRuleName, &uidChannel, MCHANNEL_TYPE_PRESET, nMaxPlayers, nLevelMin, nLevelMax);
		} else if (!strcmp(szTagName, MTOK_DEFAULTCHANNELNAME)) 
		{
			childElement.GetAttribute(szBuf, "name");
			SetDefaultChannelName(MGetStringResManager()->GetStringFromXml(szBuf));
		} else if (!strcmp(szTagName, MTOK_DEFAULTRULENAME)) 
		{
			childElement.GetAttribute(szBuf, "name");
			SetDefaultChannelRuleName(szBuf);
		}
	}

	xmlIniData.Destroy();
	return true;
}

bool MMatchServer::InitDB()
{
	CString str = m_MatchDBMgr.BuildDSNString(MGetServerConfig()->GetDB_DNS(), 
		                                      MGetServerConfig()->GetDB_UserName(), 
											  MGetServerConfig()->GetDB_Password());

	if (m_MatchDBMgr.Connect(str))
	{
		LOG(LOG_ALL, "DBMS connected");
	}
	else
	{
		LOG(LOG_ALL, "Can't Connect To DBMS");
		return false;
	}
	if( MGetServerConfig()->IsUseFilter() )
	{
		if( InitCountryFilterDB() )
			LOG(LOG_ALL, "InitCountryFilterDB.\n");
		else
		{
			LOG( LOG_ALL, "Fail to init country filter DB.\n" );
			return false;
		}
	}
	
	return true;
}

#include "MLadderMgr.h"
#include "MTeamGameStrategy.h"

bool MMatchServer::Create(int nPort)
{
	// set buffer overrun error handler /GS
	//SetSecurityErrorHandler(ReportBufferOverrun);

	srand(timeGetTime());

	m_NextUseUID.SetZero();
	m_NextUseUID.Increase(10);	// 10 아래의 UID는 사용안함

	SetupRCPLog(RcpLog);
#ifdef _DEBUG
	m_RealCPNet.SetLogLevel(0);
#else
	m_RealCPNet.SetLogLevel(0);
#endif

	if (m_SafeUDP.Create(true, MATCHSERVER_DEFAULT_UDP_PORT)==false) {
		LOG(LOG_ALL, "Match Server SafeUDP Create FAILED (Port:%d)", MATCHSERVER_DEFAULT_UDP_PORT);
		return false;
	}

	m_SafeUDP.SetCustomRecvCallback(UDPSocketRecvEvent);

	if (!LoadInitFile()) return false;

	if (!InitDB()) return false;

	m_AsyncProxy.Create(DEFAULT_ASYNCPROXY_THREADPOOL);

	m_Admin.Create(this);

	if(MServer::Create(nPort)==false) return false;

	// 디비에 최대 접속인원 업데이트
	m_MatchDBMgr.UpdateServerInfo(MGetServerConfig()->GetServerID(), MGetServerConfig()->GetMaxUser(),
								  MGetServerConfig()->GetServerName());


	// 서버 상태 보여주는 클래스 초기화
	MGetServerStatusSingleton()->Create(this);

	// 스케쥴러 초기화.
	if( !InitScheduler() ){
		LOG(LOG_ALL, "Match Server Scheduler Create FAILED" );
		return false;
	}

	MMatchAntiHack::InitClientFileList();

#ifdef _XTRAP
	// XTrap Init
	if( MGetServerConfig()->IsUseXTrap() )
		MMatchAntiHack::InitHashMap();
#endif

	if(OnCreate()==false){
//		Destroy();
		LOG(LOG_ALL, "Match Server create FAILED (Port:%d)", nPort);

		return false;
	}

	m_bCreated = true;
	
	LOG(LOG_ALL, "Match Server Created (Port:%d)", nPort);

/*////////////////// RAONDEBUG DELETE THIS
	for (int i=0; i<10; i++) {
		MAsyncDBJob_Test* pJob=new MAsyncDBJob_Test();
		PostAsyncJob(pJob);
		Sleep(10);
	}
/////////////////*/

	// 디버그용
	g_PointerChecker[0].Init(NULL);
	g_PointerChecker[1].Init(m_pScheduler);
	g_PointerChecker[2].Init(m_pAuthBuilder);

	LagComp.Create();

	return true;
}

void MMatchServer::Destroy(void)
{
	m_bCreated = false;

	OnDestroy();

	GetQuest()->Destroy();

	for (MMatchObjectList::iterator ObjItor = m_Objects.begin(); 
		ObjItor != m_Objects.end(); ++ObjItor)
	{
		MMatchObject* pObj = (*ObjItor).second;
		if (pObj)
		{
			CharFinalize(pObj->GetUID());
		}
	}

	m_ClanMap.Destroy();


	m_ChannelMap.Destroy();
/*
	MMatchChannelMap::iterator itorChannel = m_ChannelMap.begin();
	while(itorChannel != m_ChannelMap.end()) {
		MUID uid = (*itorChannel).first;
		ChannelRemove(uid, &itorChannel);
	}
*/


	m_Admin.Destroy();
	m_AsyncProxy.Destroy();
	MGetMatchShop()->Destroy();
	m_MatchDBMgr.Disconnect();
	m_SafeUDP.Destroy();
	MServer::Destroy();

	MMatchStringResManager::FreeInstance();
}

void MMatchServer::Shutdown()
{
	Log(LOG_ALL, "MatchServer Shutting down...\n");
}

bool MMatchServer::OnCreate(void)
{
	return true;
}
void MMatchServer::OnDestroy(void)
{
	if( 0 != m_pScheduler ){
		m_pScheduler->Release();
		delete m_pScheduler;
		m_pScheduler = 0;
	}
}

void MMatchServer::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MCommandCommunicator::OnRegisterCommand(pCommandManager);
	MAddSharedCommandTable(pCommandManager, MSCT_MATCHSERVER);
	Log(LOG_ALL, "Command registeration completed");

}


void MMatchServer::OnPrepareRun()
{
	MServer::OnPrepareRun();

	MGetServerStatusSingleton()->AddCmdCount(m_CommandManager.GetCommandQueueCount());
}

template <typename T>
MCommandParameterBlob* MakeBlobArrayParameter(uint32_t NumBlobs)
{
	uint32_t OneBlobSize = sizeof(T);
	size_t TotalSize = MGetBlobArrayInfoSize() + OneBlobSize * NumBlobs;
	auto Param = new MCmdParamBlob(TotalSize);
	memcpy(Param->GetPointer(), &OneBlobSize, sizeof(OneBlobSize));
	memcpy(((uint8_t*)Param->GetPointer()) + sizeof(uint32_t), &NumBlobs, sizeof(NumBlobs));
	return Param;
}

void MMatchServer::OnRun(void)
{
//	MGetLocale()->PostLoginInfoToDBAgent(MUID(1,1), "JM0000726991", "skarlfyd", 1);

#ifdef _DEBUG
//	Sleep(2000);
#endif
	MGetServerStatusSingleton()->SetRunStatus(100);
	// tick count
	SetTickTime(timeGetTime());

	// 스케쥴러 목록 업데이트.
	if (m_pScheduler)
		m_pScheduler->Update();

	// PC방 IP캐쉬 업데이트
	MPremiumIPCache()->Update();

	MGetServerStatusSingleton()->SetRunStatus(101);

	// Update Objects
	unsigned long int nGlobalClock = GetGlobalClockCount();
	unsigned long int nHShieldClock = GetGlobalClockCount();
	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end();){
		MMatchObject* pObj = (*i).second;
		pObj->Tick(nGlobalClock);
		
		if( pObj->GetDisconnStatusInfo().IsSendDisconnMsg() )
		{
			MCommand* pCmd = CreateCommand( MC_MATCH_DISCONNMSG, pObj->GetUID() );
			pCmd->AddParameter( new MCmdParamUInt(pObj->GetDisconnStatusInfo().GetMsgID()) );
			Post( pCmd );

			pObj->GetDisconnStatusInfo().SendCompleted();
		}
		else if( pObj->GetDisconnStatusInfo().IsDisconnectable(nGlobalClock) )
		{
			// 이전에 BlockType을 설정했다면 DB update flag가 설정된다.
			if( pObj->GetDisconnStatusInfo().IsUpdateDB() ) 
			{
				MAsyncDBJob_SetBlockAccount* pJob = new MAsyncDBJob_SetBlockAccount;

				pJob->Input( pObj->GetAccountInfo()->m_nAID, 
					(0 != pObj->GetCharInfo()) ? pObj->GetCharInfo()->m_nCID : 0, 
					pObj->GetDisconnStatusInfo().GetBlockType(), 
					pObj->GetDisconnStatusInfo().GetBlockLevel(),
					pObj->GetDisconnStatusInfo().GetComment(), 
					pObj->GetIPString(),
					pObj->GetDisconnStatusInfo().GetEndDate() );

				PostAsyncJob( pJob );

				// 업데이트하면 다시 DB update flag를 무효화 함.
				pObj->GetDisconnStatusInfo().UpdateDataBaseCompleted();
			}

			DisconnectObject( pObj->GetUID() );
		}

		if (pObj->CheckDestroy(nGlobalClock) == true) {
			ObjectRemove(pObj->GetUID(), &i);
			continue;
		} else {
			i++;
		}
	}

	MGetServerStatusSingleton()->SetRunStatus(102);

	// Update Stages
	for(MMatchStageMap::iterator iStage=m_StageMap.begin(); iStage!=m_StageMap.end();){
		MMatchStage* pStage = (*iStage).second;

		pStage->Tick(nGlobalClock);

		if (pStage->GetState() == STAGE_STATE_CLOSE) {
			
			StageRemove(pStage->GetUID(), &iStage);
			continue;
		}else {
			iStage++;
		}
	}

	MGetServerStatusSingleton()->SetRunStatus(103);

	// Update Channels
	m_ChannelMap.Update(nGlobalClock);
/*
	unsigned long nChannelListChecksum = 0;
	for(MMatchChannelMap::iterator iChannel=m_ChannelMap.begin(); iChannel!=m_ChannelMap.end();)
	{
		MMatchChannel* pChannel = (*iChannel).second;
		pChannel->Tick(nGlobalClock);
		if (pChannel->CheckLifePeriod() == false) {
			ChannelRemove(pChannel->GetUID(), &iChannel);
		}else {
			iChannel++;
		}
		nChannelListChecksum += pChannel->GetChecksum();
	}
	UpdateChannelListChecksum(nChannelListChecksum);
*/

	MGetServerStatusSingleton()->SetRunStatus(104);

	// Update Clans
	m_ClanMap.Tick(nGlobalClock);

	MGetServerStatusSingleton()->SetRunStatus(105);

	// Update Ladders - 클랜전서버일 경우에만 실행한다.
	if (MGetServerConfig()->GetServerMode() == MSM_CLAN)
	{
		GetLadderMgr()->Tick(nGlobalClock);
	}

	MGetServerStatusSingleton()->SetRunStatus(106);

	if (nGlobalClock - LastPingTime > 500) // Ping every half second
	{
		// Send ping list
		for (auto Stage : MakePairValueAdapter(m_StageMap))
		{
			if (Stage->GetState() == STAGE_STATE_RUN)
			{
				auto Command = CreateCommand(MC_MATCH_PING_LIST, MUID(0, 0));

				size_t NumPlayers = Stage->GetObjCount();

				auto Param = MakeBlobArrayParameter<MTD_PingInfo>(NumPlayers);
				MTD_PingInfo* PingInfos = static_cast<MTD_PingInfo*>(MGetBlobArrayPointer(Param->GetPointer()));

				auto ObjList = Stage->GetObjectList();
				auto it = ObjList.begin();
				auto end = ObjList.end();
				for (size_t i = 0; i < Stage->GetObjCount() && it != end; i++, it++)
				{
					PingInfos[i] = MTD_PingInfo{ it->GetUID(), (uint16_t)it->GetPing() };
				}

				Command->AddParameter(Param);

				RouteToBattle(Stage->GetUID(), Command);
			}
		}

		// Ping all in-game clients
		MCommand* pNew = CreateCommand(MC_NET_PING, MUID(0, 0));
		pNew->AddParameter(new MCmdParamUInt(GetGlobalClockCount()));
		RouteToAllClientIf(pNew, [](MMatchObject& Obj) {
			return Obj.GetPlace() == MMP_BATTLE; });
		LastPingTime = nGlobalClock;
	}

	// Garbage Session Cleaning
#define MINTERVAL_GARBAGE_SESSION_PING	(5 * 60 * 1000)	// 3 min
	static unsigned long tmLastGarbageSessionCleaning = nGlobalClock;
	if (nGlobalClock - tmLastGarbageSessionCleaning > MINTERVAL_GARBAGE_SESSION_PING){
		tmLastGarbageSessionCleaning = nGlobalClock;

		LOG(LOG_ALL, "ClientCount=%d, SessionCount=%d, AgentCount=%d", 
			GetClientCount(), GetCommObjCount(), GetAgentCount());
		MCommand* pNew = CreateCommand(MC_NET_PING, MUID(0,0));
		pNew->AddParameter(new MCmdParamUInt(GetGlobalClockCount()));
		RouteToAllConnection(pNew);
	}

	// Garbage MatchObject Cleaning
#define MINTERVAL_GARBAGE_SESSION_CLEANING	10*60*1000		// 10 min
	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++){
		MMatchObject* pObj = (MMatchObject*)((*i).second);
		if (pObj->GetUID() < MUID(0,3)) continue;	// MUID로 Client인지 판별할수 있는 코드 필요함
		if (GetTickTime() - pObj->GetTickLastPacketRecved() >= MINTERVAL_GARBAGE_SESSION_CLEANING) {
			LOG(LOG_PROG, "TIMEOUT CLIENT CLEANING : %s(%u%u, %s) (ClientCnt=%d, SessionCnt=%d)", 
			pObj->GetName(), pObj->GetUID().High, pObj->GetUID().Low, pObj->GetIPString(), GetClientCount(), GetCommObjCount());
			
			MUID uid = pObj->GetUID();
			ObjectRemove(uid, &i);
			Disconnect(uid);			
		}
	}

#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
	{
	// HShield Data Check
#define MINTERVAL_HSHIELD_CHECK		1*60*1000
		static unsigned long tmLastHShieldCleaning = nHShieldClock;
		if (nHShieldClock - tmLastHShieldCleaning > MINTERVAL_HSHIELD_CHECK) 
		{
			tmLastHShieldCleaning = nHShieldClock;

			SendHShieldReqMsg();
		}


		// 게임중일 때 즉, m_bEnterBattle이 true일 때는 최대지연시간이 2분
		// 맵로딩중(난입 또는 게임시작)이나 로비에 있을 때는 최대지연시간이 3분
const int MINTERVAL_HSHIELD_CLEANING_IN_BATTLE	= 2*60*1000;		// 2min 내로 HShield Ack 메세지가 안 오면 클라이언트 접속을 끊자
const int MINTERVAL_HSHIELD_CLEANING_IN_LOBBY	= 3*60*1000;		// 3min 내로 HShield Ack 메세지가 안 오면 클라이언트 접속을 끊자

		for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++)
		{
			MMatchObject* pObj = (MMatchObject*)((*i).second);
			if (pObj->GetUID() < MUID(0,3)) continue;	// MUID로 Client인지 판별할수 있는 코드 필요함
			
			// 최근에 받은 메세지가 2분이 넘었을 경우
			unsigned long int tmp = GetTickTime() - pObj->GetLastHShieldMsgRecved();
			
			int nInterval = pObj->GetEnterBattle() ? MINTERVAL_HSHIELD_CLEANING_IN_BATTLE : MINTERVAL_HSHIELD_CLEANING_IN_LOBBY;
			if ( tmp >= nInterval) 
			{
#ifndef _DEBUG
				MUID uid = pObj->GetUID();
				ObjectRemove(uid, &i);
				Disconnect(uid);			
#endif
				LOG(LOG_FILE, "HShield Ack Msg (%d ms) delayed. (%s)", tmp, pObj->GetAccountName());

			}
		}
	}
#endif

	MGetServerStatusSingleton()->SetRunStatus(107);
	/*
	#ifdef _DEBUG
	#define MINTERVAL_GARBAGE_SESSION_CLEANING	(3 * 60 * 1000)		// 3 min
	#else
	#define MINTERVAL_GARBAGE_SESSION_CLEANING	(10 * 60 * 1000)		// 10 min
	#endif
	// Garbage MatchObject Cleaning
	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++){
	MMatchObject* pObj = (MMatchObject*)((*i).second);
	if (pObj->GetUID() < MUID(0,3)) continue;	// MUID로 Client인지 판별할수 있는 코드 필요함
	if( GetTickTime() - pObj->GetTickLastPacketRecved() >= MINTERVAL_GARBAGE_SESSION_CLEANING )
	{	// 30 min
	LOG(LOG_ALL, "TIMEOUT CLIENT CLEANING : %s(%u%u) (ClientCount=%d, SessionCount=%d)", 
	pObj->GetName(), pObj->GetUID().High, pObj->GetUID().Low, GetClientCount(), GetCommObjCount());

	DisconnectObject(pObj->GetUID());
	}
	}
	*/
	MGetServerStatusSingleton()->SetRunStatus(108);

	// Process Async Jobs
	ProcessAsyncJob();

	MGetServerStatusSingleton()->SetRunStatus(109);

	// Update Logs
	UpdateServerLog();
	UpdateServerStatusDB();

	MGetServerStatusSingleton()->SetRunStatus(110);

	// update custom ip list.
	if( 3600000 < (nGlobalClock - m_CountryFilter.GetLastUpdatedTime()) )
	{
		UpdateCustomIPList();
		m_CountryFilter.SetLastUpdatedTime( nGlobalClock );
	}

	MGetServerStatusSingleton()->SetRunStatus(111);

	// Shutdown...
	m_MatchShutdown.OnRun(nGlobalClock);

	MGetServerStatusSingleton()->SetRunStatus(112);
}

void MMatchServer::UpdateServerLog()
{
	if (!IsCreated()) return;

#define SERVER_LOG_TICK		(60000)	// 1분 (1000 * 60)

	static unsigned long int st_nElapsedTime = 0;
	static unsigned long int nLastTime = timeGetTime();
	unsigned long int nNowTime = timeGetTime();

	st_nElapsedTime += (nNowTime - nLastTime);

	if (st_nElapsedTime > SERVER_LOG_TICK)
	{
		st_nElapsedTime = 0;

		// 여기서 디비 업데이트
		m_MatchDBMgr.InsertServerLog(MGetServerConfig()->GetServerID(), 
									 (int)m_Objects.size(), (int)m_StageMap.size(), 
									 GetBlockCount(), GetNonBlockCount() );
		ResetBlockCount();
		ResetNonBlockCount();
	}

	nLastTime = nNowTime;
}

void MMatchServer::UpdateServerStatusDB()
{
	if (!IsCreated()) return;

#define SERVER_STATUS_TICK		(30000)	// 30초 (1000 * 30)

	static unsigned long int st_nElapsedTime = 0;
	static unsigned long int nLastTime = timeGetTime();
	unsigned long int nNowTime = timeGetTime();

	st_nElapsedTime += (nNowTime - nLastTime);

	if (st_nElapsedTime > SERVER_STATUS_TICK)
	{
		st_nElapsedTime = 0;

		// 여기서 디비 업데이트
		int nObjSize = (int)m_Objects.size();
		if (nObjSize > MGetServerConfig()->GetMaxUser()) nObjSize = MGetServerConfig()->GetMaxUser();

		static int st_ErrCounter = 0;
		if (m_MatchDBMgr.UpdateServerStatus(MGetServerConfig()->GetServerID(), nObjSize) == false) 
		{
			LOG(LOG_ALL, "[CRITICAL ERROR] DB Connection Lost. ");
			//Shutdown();

			m_MatchDBMgr.Disconnect();
			InitDB();
			st_ErrCounter++;
			if (st_ErrCounter > MAX_DB_QUERY_COUNT_OUT) 
			{
				LOG(LOG_ALL, "[CRITICAL ERROR] UpdateServerStatusDB - Shutdown");
				Shutdown();
			}
		}
		else
		{
			st_ErrCounter = 0;
		}
	}

	nLastTime = nNowTime;
}

inline void MMatchServer::RouteToListener(MObject* pObject, MCommand* pCommand)
{
	if (pObject == NULL) return;

	size_t nListenerCount = pObject->m_CommListener.size();
	if (nListenerCount <= 0) {
		delete pCommand;
		return;
	} else if (nListenerCount == 1) {
		MUID TargetUID = *pObject->m_CommListener.begin();
		pCommand->m_Receiver = TargetUID;
		Post(pCommand);
	} else {
		int nCount = 0;
		for (list<MUID>::iterator itorUID=pObject->m_CommListener.begin(); itorUID!=pObject->m_CommListener.end(); itorUID++) {
			MUID TargetUID = *itorUID;

			MCommand* pSendCmd;
			if (nCount<=0)
				pSendCmd = pCommand;
			else
				pSendCmd = pCommand->Clone();
			pSendCmd->m_Receiver = TargetUID;
			Post(pSendCmd);
			nCount++;
		}
	}
}

void MMatchServer::RouteResponseToListener(MObject* pObject, const int nCmdID, int nResult)
{
	MCommand* pNew = CreateCommand(nCmdID, MUID(0,0));
	pNew->AddParameter(new MCmdParamInt(nResult));
	RouteToListener(pObject, pNew);
}

void MMatchServer::OnVoiceChat(const MUID & Player, unsigned char* EncodedFrame, int Length)
{
	auto Obj = GetObject(Player);

	if (!Obj)
		return;

	auto Stage = Obj->GetStageUID();

	auto Command = CreateCommand(MC_MATCH_RECEIVE_VOICE_CHAT, MUID(0, 0));
	Command->AddParameter(new MCommandParameterUID(Player));
	Command->AddParameter(new MCommandParameterBlob(EncodedFrame, Length));

	RouteToBattleExcept(Stage, Command, Player);
}

static int GetBlobCmdID(const char* Data)
{
	return *(u16*)(Data + 2);
}

#pragma pack(push, 1)
struct ZPACKEDSHOTINFO {
	float	fTime;
	short	posx, posy, posz;
	short	tox, toy, toz;
	BYTE	sel_type;
};
#pragma pack(pop)

void MMatchServer::OnTunnelledP2PCommand(const MUID & Sender, const MUID & Receiver, const char * Blob, size_t BlobSize)
{
	auto SenderObj = GetObjectA(Sender);
	if (!SenderObj)
		return;

	auto CommandID = GetBlobCmdID(Blob);
	auto uidStage = SenderObj->GetStageUID();
	auto Stage = FindStage(uidStage);
	if (!Stage)
		return;

	auto Netcode = Stage->GetStageSetting()->GetNetcode();

	[&]()
	{
		if (Netcode != NetcodeType::ServerBased)
			return;

		switch (CommandID)
		{
		case MC_PEER_BASICINFO:
		{
			BasicInfoItem bi;
			ZPACKEDBASICINFO& pbi = *(ZPACKEDBASICINFO*)(Blob + 2 + 2 + 1 + 4);
			pbi.Unpack(bi);
			bi.SentTime = pbi.fTime;
			bi.RecvTime = double(MGetMatchServer()->GetGlobalClockCount()) / 1000;
			SenderObj->BasicInfoHistory.AddBasicInfo(bi);

			//mlog("BasicInfo %d, %d, %d\n", pbi.posx, pbi.posy, pbi.posz);
		}
		break;
		case MC_PEER_SHOT:
		{
			ZPACKEDSHOTINFO& psi = *(ZPACKEDSHOTINFO*)(Blob + 2 + 2 + 1 + 4);

			v3 src = v3(psi.posx, psi.posy, psi.posz);
			v3 dest = v3(psi.tox, psi.toy, psi.toz);
			v3 orig_dir = dest - src;
			Normalize(orig_dir);

			//AnnounceF(Sender, "Shot! %f, %f, %f -> %f, %f, %f", src.x, src.y, src.z, dest.x, dest.y, dest.z);

			auto Time = double(GetGlobalClockCount() - SenderObj->GetPing()) / 1000;

			auto Slot = SenderObj->GetSelectedSlot();

			auto Item = SenderObj->GetCharInfo()->m_EquipedItem.GetItem(Slot);
			if (!Item)
				return;

			auto ItemDesc = Item->GetDesc();
			if (!ItemDesc)
				return;

			auto Damage = ItemDesc->m_nDamage;

			auto SendDamage = [&](auto& UID, auto Damage, auto PiercingRatio, auto DamageType, auto WeaponType)
			{
				MCommand* pCmd = CreateCommand(MC_MATCH_DAMAGE, UID);
				pCmd->AddParameter(new MCmdParamUID(Sender));
				pCmd->AddParameter(new MCmdParamUShort(Damage));
				pCmd->AddParameter(new MCmdParamFloat(PiercingRatio));
				pCmd->AddParameter(new MCmdParamUChar(DamageType));
				pCmd->AddParameter(new MCmdParamUChar(WeaponType));
				Post(pCmd);
			};

			if (ItemDesc->m_nWeaponType == MWT_SHOTGUN)
			{
				struct DamageInfo
				{
					int Damage = 0;
					float PiercingRatio = 0;
					ZDAMAGETYPE DamageType;
					MMatchWeaponType WeaponType;
				};

				auto DirGen = GetShotgunPelletDirGenerator(orig_dir, reinterpret<u32>(psi.fTime));

				std::unordered_map<MUID, DamageInfo> DamageMap;

				for (int i = 0; i < SHOTGUN_BULLET_COUNT; i++)
				{
					auto dir = DirGen();
					auto dest = src + dir * 10000;

					const u32 PassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;

					MPICKINFO pickinfo;
					PickHistory(*SenderObj, src, dest, Stage->BspObject,
						pickinfo, MakePairValueAdapter(Stage->m_ObjUIDCaches), Time, PassFlag);

					if (pickinfo.bBspPicked)
					{
						/*AnnounceF(Sender, "Server: Hit wall at %d, %d, %d",
							(int)pickinfo.bpi.PickPos.x, (int)pickinfo.bpi.PickPos.y,
							(int)pickinfo.bpi.PickPos.z);*/
						continue;
					}

					if (!pickinfo.pObject)
					{
						if (SenderObj->ClientSettings.DebugOutput)
							AnnounceF(Sender, "Server: No wall, no object");
						continue;
					}

					float PiercingRatio = GetPiercingRatio(ItemDesc->m_nWeaponType, pickinfo.info.parts);

					auto& item = DamageMap[pickinfo.pObject->GetUID()];

					int NewDamage = item.Damage + Damage;
					if (PiercingRatio != item.PiercingRatio)
						item.PiercingRatio = (item.Damage * item.PiercingRatio + Damage * PiercingRatio)
							/ NewDamage;
					item.Damage += Damage;
					auto DamageType = (pickinfo.info.parts == eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;
					static_assert(ZD_BULLET_HEADSHOT > ZD_BULLET, "Fix me");
					item.DamageType = max(item.DamageType, DamageType);
					item.WeaponType = ItemDesc->m_nWeaponType;
				}

				for (auto& item : DamageMap)
				{
					SendDamage(item.first,
						item.second.Damage, item.second.PiercingRatio,
						item.second.DamageType, item.second.WeaponType);

					if (SenderObj->ClientSettings.DebugOutput)
					{
						AnnounceF(Sender, "Server: Hit %s for %d damage",
							GetObject(item.first)->GetName(), item.second.Damage);
						v3 Head, Origin;
						SenderObj->GetPositions(Head, Origin, Time);
						AnnounceF(Sender, "Server: Head: %d, %d, %d; origin: %d, %d, %d",
							int(Head.x), int(Head.y), int(Head.z),
							int(Origin.x), int(Origin.y), int(Origin.z));
					}
				}
			}
			else
			{
				/*AnnounceF(Sender, "%s: ping = %d, abs time = %X\nHead: %f, %f, %f; foot: %f, %f, %f",
				Obj.GetName(), Obj.GetPing(), GetGlobalClockCount() - Obj.GetPing(),
					Head.x, Head.y, Head.z, Root.x, Root.y, Root.z);*/

				const u32 PassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;

				MPICKINFO pickinfo;
				PickHistory(*SenderObj, src, dest, Stage->BspObject, pickinfo,
					MakePairValueAdapter(Stage->m_ObjUIDCaches), Time, PassFlag);

				if (pickinfo.bBspPicked)
				{
					/*AnnounceF(Sender, "Server: Hit wall at %d, %d, %d",
						(int)pickinfo.bpi.PickPos.x, (int)pickinfo.bpi.PickPos.y,
						(int)pickinfo.bpi.PickPos.z);*/
					return;
				}

				if (!pickinfo.pObject)
				{
					if (SenderObj->ClientSettings.DebugOutput)
						AnnounceF(Sender, "Server: No wall, no object");
					return;
				}

				float PiercingRatio = GetPiercingRatio(ItemDesc->m_nWeaponType, pickinfo.info.parts);
				auto DamageType = (pickinfo.info.parts == eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;
				auto WeaponType = ItemDesc->m_nWeaponType;

				if (SenderObj->ClientSettings.DebugOutput)
				{
					AnnounceF(Sender, "Server: Hit %s for %d damage", pickinfo.pObject->GetName(), Damage);
					v3 Head, Origin;
					SenderObj->GetPositions(Head, Origin, Time);
					AnnounceF(Sender, "Server: Head: %d, %d, %d; origin: %d, %d, %d",
						int(Head.x), int(Head.y), int(Head.z),
						int(Origin.x), int(Origin.y), int(Origin.z));
				}

				SendDamage(pickinfo.pObject->GetUID(), Damage, PiercingRatio, DamageType, WeaponType);
			}
		}
		break;
		};
	}();

	MCommand* pCmd = CreateCommand(MC_MATCH_P2P_COMMAND, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUID(Sender));
	pCmd->AddParameter(new MCmdParamBlob(Blob, BlobSize));

	RouteToBattleExcept(uidStage, pCmd, Sender);

	//LogF(LOG_ALL, "P2P Command! Sender = %X:%X, receiver = %X:%X, command ID = %X", Sender.High, Sender.Low, Receiver.High, Receiver.Low, ID);
}

struct stRouteListenerNode
{
	DWORD				nUserContext;
	MPacketCrypterKey	CryptKey;
	//SEED_ALG_INFO	CryptAlgInfo;
};

void MMatchServer::RouteToAllConnection(MCommand* pCommand)
{
	queue<stRouteListenerNode*>	ListenerList;

	// Queueing for SafeSend
	LockCommList();
		for(auto i=m_CommRefCache.begin(); i!=m_CommRefCache.end(); i++){
			MCommObject* pCommObj = i->second;
			if (pCommObj->GetUID() < MUID(0,3)) continue;	// MUID로 Client인지 판별할수 있는 코드 필요함

			stRouteListenerNode* pNewNode = new stRouteListenerNode;
			pNewNode->nUserContext = pCommObj->GetUserContext();
			memcpy(&pNewNode->CryptKey, pCommObj->GetCrypter()->GetKey(), sizeof(MPacketCrypterKey));
			ListenerList.push(pNewNode);
		}
	UnlockCommList();

	// Send the queue (each all session)
	int nCmdSize = pCommand->GetSize();

	if (nCmdSize <= 0)
	{
		while (!ListenerList.empty())
		{
			stRouteListenerNode* pNode = ListenerList.front();
			ListenerList.pop();
			delete pNode;
		}
		return;
	}

	char* pCmdData = new char[nCmdSize];
	int nSize = pCommand->GetData(pCmdData, nCmdSize);
	_ASSERT(nSize < MAX_PACKET_SIZE && nSize==nCmdSize);


	if (pCommand->m_pCommandDesc->IsFlag(MCCT_NON_ENCRYPTED))
	{
		while (!ListenerList.empty())
		{
			stRouteListenerNode* pNode = ListenerList.front();
			ListenerList.pop();

			SendMsgCommand(pNode->nUserContext, pCmdData, nSize, MSGID_RAWCOMMAND, NULL);

			delete pNode;
		}
	}
	else
	{
		while (!ListenerList.empty())
		{
			stRouteListenerNode* pNode = ListenerList.front();
			ListenerList.pop();

			SendMsgCommand(pNode->nUserContext, pCmdData, nSize, MSGID_COMMAND, &pNode->CryptKey);

			delete pNode;
		}
	}

	delete [] pCmdData;
	delete pCommand;
}

void MMatchServer::RouteToAllClient(MCommand* pCommand)
{
	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++){
		MMatchObject* pObj = (MMatchObject*)((*i).second);
		if (pObj->GetUID() < MUID(0,3)) continue;	// MUID로 Client인지 판별할수 있는 코드 필요함
		
		MCommand* pSendCmd = pCommand->Clone();
		pSendCmd->m_Receiver = pObj->GetUID();
		Post(pSendCmd);
	}	
	delete pCommand;
}

void MMatchServer::RouteToChannel(const MUID& uidChannel, MCommand* pCommand)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) 
	{
		delete pCommand;
		return;
	}

	for (auto i=pChannel->GetObjBegin(); i!=pChannel->GetObjEnd(); i++) {
		MObject* pObj = i->second;

		MCommand* pSendCmd = pCommand->Clone();
		RouteToListener(pObj, pSendCmd);
	}
	delete pCommand;
}

void MMatchServer::RouteToChannelLobby(const MUID& uidChannel, MCommand* pCommand)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) 
	{
		delete pCommand;
		return;
	}

	for (auto i=pChannel->GetLobbyObjBegin(); i!=pChannel->GetLobbyObjEnd(); i++)
	{
		MObject* pObj = i->second;

		MCommand* pSendCmd = pCommand->Clone();
		RouteToListener(pObj, pSendCmd);
	}
	delete pCommand;
}

void MMatchServer::RouteToStage(const MUID& uidStage, MCommand* pCommand)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) 
	{
		delete pCommand;
		return;
	}

	for (auto i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MUID uidObj = i->first;
		MObject* pObj = (MObject*)GetObject(uidObj);
		if (pObj) {
			MCommand* pSendCmd = pCommand->Clone();
			RouteToListener(pObj, pSendCmd);
		} else {
			LOG(LOG_ALL, "WARNING(RouteToStage) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i=pStage->RemoveObject(uidObj);	// RAONHAJE : 방에 쓰레기UID 남는것 발견시 로그&청소
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToStageWaitRoom(const MUID& uidStage, MCommand* pCommand)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) 
	{
		delete pCommand;
		return;
	}

	for (auto i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {

		MUID uidObj = i->first;
		MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
		if (pObj) {
			if (! pObj->GetEnterBattle())
			{
				MCommand* pSendCmd = pCommand->Clone();
				RouteToListener(pObj, pSendCmd);
			} 
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToBattle(const MUID& uidStage, MCommand* pCommand)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == nullptr) 
	{
		delete pCommand;
		return;
	}

	for (auto i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		//MMatchObject* pObj = (MMatchObject*)(*i).second;

		MUID uidObj = i->first;
		MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
		if (pObj) {
			if (pObj->GetEnterBattle())
			{
				MCommand* pSendCmd = pCommand->Clone();
				RouteToListener(pObj, pSendCmd);
			} 
		}else {
			LOG(LOG_ALL, "WARNING(RouteToBattle) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i=pStage->RemoveObject(uidObj);	// RAONHAJE : 방에 쓰레기UID 남는것 발견시 로그&청소
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToBattleExcept(const MUID& uidStage, MCommand* pCommand, const MUID& uidExceptedPlayer)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == nullptr)
	{
		delete pCommand;
		return;
	}

	for (auto i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++) {
		MUID uidObj = i->first;

		if (uidObj == uidExceptedPlayer)
			continue;

		MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
		if (pObj) {
			if (pObj->GetEnterBattle())
			{
				MCommand* pSendCmd = pCommand->Clone();
				RouteToListener(pObj, pSendCmd);
			}
		}
		else {
			LOG(LOG_ALL, "WARNING(RouteToBattle) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i = pStage->RemoveObject(uidObj);	// RAONHAJE : 방에 쓰레기UID 남는것 발견시 로그&청소
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToClan(const int nCLID, MCommand* pCommand)
{
	MMatchClan* pClan = FindClan(nCLID);
	if (pClan == NULL) 
	{
		delete pCommand;
		return;
	}

	for (auto i=pClan->GetMemberBegin(); i!=pClan->GetMemberEnd(); i++) {
		MObject* pObj = i->second;

		MCommand* pSendCmd = pCommand->Clone();
		RouteToListener(pObj, pSendCmd);
	}
	delete pCommand;
}

void MMatchServer::ResponseRoundState(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchRule* pRule = pStage->GetRule();
	if (pRule == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_ROUNDSTATE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundCount()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundState()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundArg()));

	// 게임 안에 있는 플레이어에게만 전송
	RouteToBattle(uidStage, pCmd);
}

void MMatchServer::ResponseRoundState(MMatchObject* pObj, const MUID& uidStage)
{
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchRule* pRule = pStage->GetRule();
	if (pRule == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_ROUNDSTATE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundCount()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundState()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundArg()));

	RouteToListener(pObj, pCmd);
}

void MMatchServer::NotifyMessage(const MUID& uidChar, int nMsgID)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;
	
	MCommand* pNew=new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_NOTIFY), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUInt(nMsgID));
	RouteToListener(pObj, pNew);
}

int MMatchServer::ObjectAdd(const MUID& uidComm)
{
	MMatchObject* pObj = new MMatchObject(uidComm);
	pObj->UpdateTickLastPacketRecved();

	m_Objects.insert(MMatchObjectList::value_type(pObj->GetUID(), pObj));
//	*pAllocUID = pObj->GetUID();

	//LOG("Character Added (UID:%d%d)", pObj->GetUID().High, pObj->GetUID().Low);

	return MOK;
}

int MMatchServer::ObjectRemove(const MUID& uid, MMatchObjectList::iterator* pNextItor)
{
	MMatchObjectList::iterator i = m_Objects.find(uid);
	if(i==m_Objects.end()) return MERR_OBJECT_INVALID;

	MMatchObject* pObj = (*i).second;

	// Clear up the Object
	if (pObj->GetChatRoomUID() != MUID(0,0)) {
		MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
		MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoom(pObj->GetChatRoomUID());
		if (pRoom)
			pRoom->RemovePlayer(pObj->GetUID());
	}
	if (pObj->GetStageUID() != MUID(0,0)) {
		StageLeaveBattle(pObj->GetUID(), pObj->GetStageUID());
	}
	if (pObj->GetStageUID() != MUID(0,0)) {
		StageLeave(pObj->GetUID(), pObj->GetStageUID());
	}
	if (pObj->GetChannelUID() != MUID(0,0)) {
		ChannelLeave(pObj->GetUID(), pObj->GetChannelUID());
	}
	
	// m_ClanMap에서도 삭제
	m_ClanMap.RemoveObject(pObj->GetUID(), pObj);

	delete pObj;
	pObj = NULL;

	MMatchObjectList::iterator itorTemp = m_Objects.erase(i);
	if (pNextItor)
		*pNextItor = itorTemp;

	return MOK;
}

MMatchObject* MMatchServer::GetObject(const MUID& uid)
{
	MMatchObjectList::iterator i = m_Objects.find(uid);
	if(i==m_Objects.end()) return NULL;
	return (*i).second;
}

MMatchObject* MMatchServer::GetPlayerByCommUID(const MUID& uid)
{
	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++){
		MMatchObject* pObj = ((*i).second);
		for (list<MUID>::iterator j=pObj->m_CommListener.begin(); j!=pObj->m_CommListener.end(); j++){
			MUID TargetUID = *j;
			if (TargetUID == uid)
				return pObj;
		}
	}
	return NULL;
}

MMatchObject* MMatchServer::GetPlayerByName(const char* pszName)
{
	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++){
		MMatchObject* pObj = ((*i).second);
		if (_stricmp(pObj->GetName(), pszName) == 0)
			return pObj;
	}
	return NULL;
}

MMatchObject* MMatchServer::GetPlayerByAID(unsigned long int nAID)
{
	if (nAID == 0) return NULL;

	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++)
	{
		MMatchObject* pObj = ((*i).second);
		if (pObj->GetAccountInfo()->m_nAID == nAID)
			return pObj;
	}
	return NULL;
}





MUID MMatchServer::UseUID(void)
{
	LockUIDGenerate();
		MUID ret = m_NextUseUID;
		m_NextUseUID.Increase();
	UnlockUIDGenerate();
	return ret;
}

void MMatchServer::SetClientClockSynchronize(const MUID& CommUID)
{
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_CLOCK_SYNCHRONIZE), CommUID, m_This);
	pNew->AddParameter(new MCommandParameterUInt(GetGlobalClockCount()));
	Post(pNew);
}

void MMatchServer::Announce(const MUID& CommUID, char* pszMsg)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, CommUID);
	pCmd->AddParameter(new MCmdParamUInt(0));
	pCmd->AddParameter(new MCmdParamStr(pszMsg));
	Post(pCmd);
}

void MMatchServer::Announce(MObject* pObj, char* pszMsg)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUInt(0));
	pCmd->AddParameter(new MCmdParamStr(pszMsg));
	RouteToListener(pObj, pCmd);
}

void MMatchServer::AnnounceErrorMsg(const MUID& CommUID, const int nErrorCode)
{
	// 다음 서버 패치때 Announce대신 ErrorCode로 클라이언트에 메세지를 전송할 수 있도록 만들 예정
}

void MMatchServer::AnnounceErrorMsg(MObject* pObj, const int nErrorCode)
{

}




void MMatchServer::OnBridgePeer(const MUID& uidChar, DWORD dwIP, DWORD nPort)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;
// 임시 Debug코드 ///////
#ifdef _DEBUG
if (strcmp(pObj->GetAccountName(), "라온하제4")==0)
return;
if (strcmp(pObj->GetAccountName(), "라온하제3")==0)
return;
#endif
/////////////////////////

/*	if (pObj->GetBridgePeer() == false) {
		char szMsg[128];
		sprintf_safe(szMsg, "BridgePeer : Player[%d%d] Local(%s:%u) , Routed(%s:%u)", 
			uidChar.High, uidChar.Low, szLocalIP, nLocalPort, szIP, nPort);
		LOG(LOG_DEBUG, szMsg);
	}*/

	in_addr addr;
	addr.s_addr = dwIP;
	char* pszIP = inet_ntoa(addr);

	pObj->SetPeerAddr(dwIP, pszIP, (unsigned short)nPort);
	pObj->SetBridgePeer(true);
	pObj->SetPlayerFlag(MTD_PlayerFlags_BridgePeer, true);

	ResponseBridgePeer(uidChar, 0);
}

MMatchServer* MMatchServer::GetInstance(void)
{
	return m_pInstance;
}

unsigned long int MMatchServer::GetGlobalClockCount(void)
{
	unsigned long int i = timeGetTime();
	return i;
}

unsigned long int MMatchServer::ConvertLocalClockToGlobalClock(unsigned long int nLocalClock, unsigned long int nLocalClockDistance)
{
	return (nLocalClock + nLocalClockDistance);
}

unsigned long int MMatchServer::ConvertGlobalClockToLocalClock(unsigned long int nGlobalClock, unsigned long int nLocalClockDistance)
{
	return (nGlobalClock - nLocalClockDistance);
}

void MMatchServer::DebugTest()
{
#ifndef _DEBUG
	return;
#endif

///////////
	LOG(LOG_DEBUG, "DebugTest: Object List");
	for(MMatchObjectList::iterator it=m_Objects.begin(); it!=m_Objects.end(); it++){
		MMatchObject* pObj = (*it).second;
		LOG(LOG_DEBUG, "DebugTest: Obj(%d%d)", pObj->GetUID().High, pObj->GetUID().Low);
	}
///////////
}

void MMatchServer::SendCommandByUDP(MCommand* pCommand, char* szIP, int nPort)
{
	_ASSERT(0);	// 사용하지 않음
	// 현재는 1024 size이상 보낼 수 없다.
	const int BUF_SIZE = 1024;

	char* szBuf = new char[BUF_SIZE];
	int iMaxPacketSize = BUF_SIZE;

	MPacketHeader a_PacketHeader;
	int iHeaderSize = sizeof(a_PacketHeader);
	int size = pCommand->GetData(szBuf + iHeaderSize, iMaxPacketSize - iHeaderSize);
	size += iHeaderSize;
	a_PacketHeader.nMsg = MSGID_COMMAND;
	a_PacketHeader.nSize = size;
	memcpy(szBuf, &a_PacketHeader, iHeaderSize);

	bool bRet = m_SafeUDP.Send(szIP, nPort, szBuf, size);
}

bool MMatchServer::UDPSocketRecvEvent(DWORD dwIP, WORD wRawPort, char* pPacket, DWORD dwSize)
{
	if (dwSize < sizeof(MPacketHeader)) return false;

	MPacketHeader*	pPacketHeader;
	pPacketHeader = (MPacketHeader*)pPacket;

	if ((dwSize < pPacketHeader->nSize) || 
		((pPacketHeader->nMsg != MSGID_COMMAND)&&(pPacketHeader->nMsg != MSGID_RAWCOMMAND))	) return false;

	MMatchServer* pServer = MMatchServer::GetInstance();
	pServer->ParseUDPPacket(&pPacket[sizeof(MPacketHeader)], pPacketHeader, dwIP, wRawPort);
	return true;
}

void MMatchServer::ParseUDPPacket(char* pData, MPacketHeader* pPacketHeader, DWORD dwIP, WORD wRawPort)
{
	switch (pPacketHeader->nMsg)
	{
	case MSGID_RAWCOMMAND:
		{
			unsigned short nCheckSum = MBuildCheckSum(pPacketHeader, pPacketHeader->nSize);
			if (pPacketHeader->nCheckSum != nCheckSum) {
				static int nLogCount = 0;
				if (nLogCount++ < 100) {	// Log Flooding 방지
					mlog("MMatchServer::ParseUDPPacket() -> CHECKSUM ERROR(R=%u/C=%u)\n", 
						pPacketHeader->nCheckSum, nCheckSum);
				}
				return;
			} else {
				MCommand* pCmd = new MCommand();
				pCmd->SetData(pData, &m_CommandManager);

				if (pCmd->GetID() == MC_MATCH_BRIDGEPEER) {
					pCmd->m_Sender = MUID(0,0);
					pCmd->m_Receiver = m_This;

					unsigned long nPort = ntohs(wRawPort);

					MCommandParameterUInt* pParamIP = (MCommandParameterUInt*)pCmd->GetParameter(1);
					MCommandParameterUInt* pParamPort = (MCommandParameterUInt*)pCmd->GetParameter(2);
					if (pParamIP==NULL || pParamIP->GetType()!=MPT_UINT)
					{
						delete pCmd;
						break;
					}
					if (pParamPort==NULL || pParamPort->GetType()!=MPT_UINT)
					{
						delete pCmd;
						break;
					}

					char pData[64];
					MCommandParameterUInt(dwIP).GetData(pData, 64);
					pParamIP->SetData(pData);
					MCommandParameterUInt(nPort).GetData(pData, 64);
					pParamPort->SetData(pData);

					PostSafeQueue(pCmd);
				}
			}
		}
		break;
	case MSGID_COMMAND:
		{
			_ASSERT(0);
			// 서버상에 암호화된 UDP는 사용하지 않음
			Log(LOG_DEBUG, "MMatchServer::ParseUDPPacket: Parse Packet Error");
		}
		break;
	default:
		{
			_ASSERT(0);
			Log(LOG_DEBUG, "MMatchServer::ParseUDPPacket: Parse Packet Error");
		}

		break;
	}
}

void MMatchServer::ResponseBridgePeer(const MUID& uidChar, int nCode)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pNew = CreateCommand(MC_MATCH_BRIDGEPEER_ACK, MUID(0,0));
	pNew->AddParameter(new MCmdParamUID(uidChar));
	pNew->AddParameter(new MCmdParamInt(nCode));
	RouteToListener(pObj, pNew);
}

// 난입한 유저가 방안에 있는 다른 사람들 정보 달라고 요청했을때 방안의 유저정보를 알려준다
void MMatchServer::ResponsePeerList(const MUID& uidChar, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_PEERLIST, MUID(0,0));
	pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));

	// Battle에 들어간 사람만 List를 만든다.
	int nPeerCount = pStage->GetObjInBattleCount();

	void* pPeerArray = MMakeBlobArray(sizeof(MTD_PeerListNode), nPeerCount);
	int nIndex=0;
	for (auto itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++) {
		MMatchObject* pObj = itor->second;
		if (pObj->GetEnterBattle() == false) continue;

		MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pPeerArray, nIndex++);
		pNode->uidChar = pObj->GetUID();

		pNode->dwIP = pObj->GetIP();
//		strcpy_safe(pNode->szIP, pObj->GetIP());
		pNode->nPort = pObj->GetPort();

		CopyCharInfoForTrans(&pNode->CharInfo, pObj->GetCharInfo(), pObj);

		ZeroMemory(&pNode->ExtendInfo, sizeof(MTD_ExtendInfo));
		if (pStage->GetStageSetting()->IsTeamPlay())
			pNode->ExtendInfo.nTeam = (char)pObj->GetTeam();
		else
			pNode->ExtendInfo.nTeam = 0;
		pNode->ExtendInfo.nPlayerFlags = pObj->GetPlayerFlags();
	}
	pNew->AddParameter(new MCommandParameterBlob(pPeerArray, MGetBlobArraySize(pPeerArray)));
	MEraseBlobArray(pPeerArray);

	RouteToListener(pObj, pNew);
}


bool MMatchServer::CheckBridgeFault()
{
	for (MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++) {
		MMatchObject* pObj = (*i).second;
		if (pObj->GetBridgePeer() == false)
			return true;
	}
	return false;
}




void MMatchServer::OnUserWhisper(const MUID& uidComm, char* pszSenderName, char* pszTargetName, char* pszMessage)
{
	if (strlen(pszSenderName) < 2) return;
	if (strlen(pszTargetName) < 2) return;

	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}
	if (pTargetObj->CheckUserOption(MBITFLAG_USEROPTION_REJECT_WHISPER) == true) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_USER_WHISPER_REJECTED);
//		NotifyMessage(pTargetObj->GetUID(), MATCHNOTIFY_USER_WHISPER_IGNORED);
		return;
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_USER_WHISPER, MUID(0,0));
	pCmd->AddParameter(new MCmdParamStr(pObj->GetName()));
	pCmd->AddParameter(new MCmdParamStr(pszTargetName));
	pCmd->AddParameter(new MCmdParamStr(pszMessage));
	RouteToListener(pTargetObj, pCmd);
}

void MMatchServer::OnUserWhere(const MUID& uidComm, char* pszTargetName)
{
	if (strlen(pszTargetName) < 2) return;

	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (!IsEnabledObject(pObj)) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	if ((IsAdminGrade(pObj) == false) && (IsAdminGrade(pTargetObj) == true)) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	char szLog[256] = "";

	bool bUnknownChannel = true;

	MMatchChannel* pChannel = FindChannel(pTargetObj->GetChannelUID());
	if (pChannel) {
		if (pTargetObj->GetPlace() == MMP_LOBBY)
		{
			bUnknownChannel = false;
			sprintf_safe(szLog, "[%s] '%s'",
				pTargetObj->GetName(), 
				pChannel->GetName() );
		}
		else if ((pTargetObj->GetPlace() == MMP_STAGE) || (pTargetObj->GetPlace() == MMP_BATTLE))
		{
			MMatchStage* pStage = FindStage( pTargetObj->GetStageUID() );
			if( 0 != pStage )
			{
				bUnknownChannel = false;
				sprintf_safe(szLog, "[%s] '%s' , '(%d)%s'",
					pTargetObj->GetName(), 
					pChannel->GetName(), 
					pStage->GetIndex()+1,
					pStage->GetName() );
			}
		}
	}
	
	if (bUnknownChannel)
		sprintf_safe(szLog, "%s , Unknown Channel", pTargetObj->GetName());

	Announce(pObj, szLog);
}

void MMatchServer::OnUserOption(const MUID& uidComm, unsigned long nOptionFlags)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->SetUserOption(nOptionFlags);
}

void MMatchServer::OnChatRoomCreate(const MUID& uidPlayer, const char* pszChatRoomName)
{
	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_ALREADY_EXIST);	// Notify Already Exist
		return;
	}

	pRoom = pChatRoomMgr->AddChatRoom(uidPlayer, pszChatRoomName);
	if (pRoom == NULL) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_CREATE_FAILED);	// Notify Can't Create
		return;
	}

	if (pRoom->AddPlayer(uidPlayer) == true) {
		LOG(LOG_PROG, "ChatRoom Created : '%s' ", pszChatRoomName);
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_CREATE_SUCCEED);
	} else {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_JOIN_FAILED);		// Notify Join Failed
	}
}

void MMatchServer::OnChatRoomJoin(const MUID& uidComm, char* pszPlayerName, char* pszChatRoomName)
{
	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom == NULL) return;
	
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	if (pRoom->GetUserCount() > CHATROOM_MAX_ROOMMEMBER) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_USER_FULL);			// Notify Full Member
		return;
	}

	if (pRoom->AddPlayer(uidComm)) {
		// Notify Joinning to Participant
		MCommand* pCmd = CreateCommand(MC_MATCH_CHATROOM_JOIN, MUID(0,0));
		pCmd->AddParameter(new MCmdParamStr(pObj->GetName()));
		pCmd->AddParameter(new MCmdParamStr(pszChatRoomName));
		pRoom->RouteCommand(pCmd);
	} else {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_JOIN_FAILED);		// Notify Join a room Failed
	}
}

void MMatchServer::OnChatRoomLeave(const MUID& uidComm, char* pszPlayerName, char* pszChatRoomName)
{
	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom == NULL)
		return;

	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pRoom->RemovePlayer(uidComm);

	// Notify to Player and Participant
	MCommand* pCmd = CreateCommand(MC_MATCH_CHATROOM_LEAVE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamStr(pObj->GetName()));
	pCmd->AddParameter(new MCmdParamStr(pszChatRoomName));
	pRoom->RouteCommand(pCmd);
}

void MMatchServer::OnChatRoomSelectWrite(const MUID& uidComm, const char* pszChatRoomName)
{
	MMatchObject* pPlayer = GetObject(uidComm);
	if (pPlayer == NULL) return;

	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom == NULL) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_EXIST);		// Notify Does not Exist
		return;
	}

	pPlayer->SetChatRoomUID(pRoom->GetUID());
}

void MMatchServer::OnChatRoomInvite(const MUID& uidComm, const char* pszTargetName)
{
	if (strlen(pszTargetName) < 2) return;

	MMatchObject* pPlayer = GetPlayerByCommUID(uidComm);
	if (pPlayer == NULL) return;

	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoom(pPlayer->GetChatRoomUID());
	if (pRoom == NULL) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_EXIST);		// Notify Does not Exist
		return;
	}

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) {
		NotifyMessage(pPlayer->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	if (pTargetObj->CheckUserOption(MBITFLAG_USEROPTION_REJECT_INVITE) == true) {
		NotifyMessage(pPlayer->GetUID(), MATCHNOTIFY_USER_INVITE_REJECTED);
		NotifyMessage(pTargetObj->GetUID(), MATCHNOTIFY_USER_INVITE_IGNORED);
		return;
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_CHATROOM_INVITE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamStr(pPlayer->GetName()));
	pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pszTargetName)));
	pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pRoom->GetName())));
	RouteToListener(pTargetObj, pCmd);

}

// RAONHAJE 임시코드
#ifdef _DEBUG
	#include "CMLexicalAnalyzer.h"
	bool StageFinish(MMatchServer* pServer, const MUID& uidPlayer, char* pszChat)
	{
		MMatchObject* pChar = pServer->GetObject(uidPlayer);
		if (pChar == NULL)	return false;
//		if (pChar->GetPlace() != MMP_LOBBY) return false;
		MMatchStage* pStage = pServer->FindStage(pChar->GetStageUID());
		if (pStage == NULL) return false;

		bool bResult = false;
		CMLexicalAnalyzer lex;
		lex.Create(pszChat);

		if (lex.GetCount() >= 1) {
			char* pszCmd = lex.GetByStr(0);
			if (pszCmd) {
				if (_stricmp(pszCmd, "/finish") == 0) {
					pStage->GetRule()->DebugTest();
					bResult = true;
				}	// Finish
			}
		}

		lex.Destroy();
		return bResult;
	}
#endif

void MMatchServer::OnChatRoomChat(const MUID& uidComm, const char* pszMessage)
{
	MMatchObject* pPlayer = GetObject(uidComm);
	if (pPlayer == NULL) return;

#ifdef _DEBUG
	if (StageFinish(this, uidComm, const_cast<char*>(pszMessage)))
		return;
#endif

	if (pPlayer->GetChatRoomUID() == MUID(0,0)) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_USING);		// Notify No ChatRoom
		return;
	}

	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoom(pPlayer->GetChatRoomUID());
	if (pRoom == NULL) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_EXIST);		// Notify Does not Exist
		return;
	}

	pRoom->RouteChat(pPlayer->GetUID(), const_cast<char*>(pszMessage));
}

void MMatchServer::DisconnectObject(const MUID& uidObject)
{
	MMatchObject* pObj = GetObject(uidObject);
	if (pObj == NULL) return;

	Disconnect(pObj->GetCommListener());
}



void MMatchServer::InsertChatDBLog(const MUID& uidPlayer, const char* szMsg)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	unsigned long int nNowTime = timeGetTime();

	static int stnLogTop = 0;
#define MAX_CHAT_LOG 1

	static struct MCHATLOG
	{
		unsigned long int nCID;
		char szMsg[256];
		unsigned long int nTime;
	} stChatLog[MAX_CHAT_LOG];

	stChatLog[stnLogTop].nCID = pObj->GetCharInfo()->m_nCID;
	if (strlen(szMsg) < 256) strcpy_safe(stChatLog[stnLogTop].szMsg, szMsg); else strcpy_safe(stChatLog[stnLogTop].szMsg, "");
	stChatLog[stnLogTop].nTime = timeGetTime();
	stnLogTop++;

	// 일정 개수가 될때만 DB에 넣는다.
	if (stnLogTop >= MAX_CHAT_LOG)
	{
		for (int i = 0; i < stnLogTop; i++)
		{

			if (!m_MatchDBMgr.InsertChatLog(stChatLog[i].nCID, stChatLog[i].szMsg, stChatLog[i].nTime))
			{
				LOG(LOG_ALL, "DB Query(InsertChatDBLog > InsertChatLog) Failed");
			}
		}
		stnLogTop = 0;
	}
}



int MMatchServer::ValidateMakingName(const char* szCharName, int nMinLength, int nMaxLength)
{
/*
#define _MAX_NAME_TOK				100
#define _MAX_NAME_FULLNAME			100

	static string strTok[_MAX_NAME_TOK] = { "maiet", "gunz", "admin", "netmarble", "^", "\"", "'", "`", ",", " "};
	static string strFullName[_MAX_NAME_FULLNAME] = { "maiet", "gunz"};


	char szLwrCharName[256];
	strcpy_safe(szLwrCharName, szCharName);
	_strlwr(szLwrCharName);


	int nNameLen = (int)strlen(szCharName);
	if (nNameLen < nMinLength) return MERR_TOO_SHORT_NAME;
	if (nNameLen > nMaxLength) return MERR_TOO_LONG_NAME;
	
	unsigned char c;


	// 한글, 영문자, 숫자만 허용한다
	for (int i = 0; i < nNameLen; i++)
	{
		c = (unsigned char)szCharName[i];
		if (!( 
			((c >= '0') && (c <= '9')) || 
			((c >= 'A') && (c <= 'z')) || 
			(c > 127) || (c == '[') || (c == ']') || (c == '_') || (c == '-')		// 해당 특수문자도 허용
			
			)
		   )
		{
			return MERR_WRONG_WORD_NAME;
		}
	}


	// 2바이트 문자중에 0xC9xx 또는 0xFExx , 0xA1A1, 0xA4D4인 글자는 이름에 넣을 수 없다
	int nCur = 0;
	while (nCur < nNameLen-1)
	{
		unsigned char c1 = (unsigned char)szCharName[nCur];
		unsigned char c2 = (unsigned char)szCharName[nCur+1];

		if (c1 > 127)
		{
			if ((c1 == 0xc9) && (c2 > 127))
			{
				return MERR_WRONG_WORD_NAME;
			}
			if ((c1 == 0xfe) && (c2 > 127))
			{
				return MERR_WRONG_WORD_NAME;
			}
			if ((c1 == 0xa1) && (c2 == 0xa1))
			{
				return MERR_WRONG_WORD_NAME;
			}
			if ((c1 == 0xa4) && (c2 == 0xd4))
			{
				return MERR_WRONG_WORD_NAME;
			}

			nCur += 2;
		}
		else
		{
			nCur++;
		}
	}


	for (int i = 0;i < _MAX_NAME_TOK; i++)
	{
		if (strTok[i].size() > 0)
		{
			if (strstr(szLwrCharName, strTok[i].c_str()) != NULL)
			{
				return MERR_WRONG_WORD_NAME;
			}
		}
	}

	for (int i = 0; i < _MAX_NAME_FULLNAME; i++)
	{
		if (strFullName[i].size() > 0)
		{
			if (!strcmp(szLwrCharName, strFullName[i].c_str()))
				return MERR_WRONG_WORD_NAME;
		}
	}

	return MOK;
*/

	int nNameLen = (int)strlen(szCharName);

	if (nNameLen < nMinLength)
		return MERR_TOO_SHORT_NAME;

	if (nNameLen > nMaxLength)
		return MERR_TOO_LONG_NAME;

	return MOK;
}


// 플레이어가 방에 들어갈 수 있는지 검증한다.
int MMatchServer::ValidateStageJoin(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return MERR_CANNOT_JOIN_STAGE;

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return MERR_CANNOT_JOIN_STAGE;

	// close상태인지 체크
	if (pStage->GetState() == STAGE_STATE_CLOSE) return MERR_CANNOT_JOIN_STAGE;

	// 운영자, 관리자면 제한없이 입장
	if (!IsAdminGrade(pObj))
	{
		// 인원체크
		if (pStage->GetStageSetting()->GetMaxPlayers() <= pStage->GetCountableObjCount())
		{
			return MERR_CANNOT_JOIN_STAGE_BY_MAXPLAYERS;
		}

		// 레벨체크
		if (pStage->GetStageSetting()->GetLimitLevel() != 0)
		{
			int nMasterLevel, nLimitLevel;
			MMatchObject* pMaster = GetObject(pStage->GetMasterUID());

			if (IsEnabledObject(pMaster))
			{
				nMasterLevel = pMaster->GetCharInfo()->m_nLevel;
				nLimitLevel = pStage->GetStageSetting()->GetLimitLevel();
				if (abs(pObj->GetCharInfo()->m_nLevel - nMasterLevel) > nLimitLevel)
				{
					return MERR_CANNOT_JOIN_STAGE_BY_LEVEL;
				}
			}
		}

		// 게임중참가
		if ((pStage->GetStageSetting()->GetForcedEntry() == false) && 
			(pStage->GetState() != STAGE_STATE_STANDBY))
		{
			return MERR_CANNOT_JOIN_STAGE_BY_FORCEDENTRY;
		}

		// Ban Check
		if (pStage->CheckBanList(pObj->GetCharInfo()->m_nCID))
			return MERR_CANNOT_JOIN_STAGE_BY_BAN;
	}

	return MOK;
}

int MMatchServer::ValidateChannelJoin(const MUID& uidPlayer, const MUID& uidChannel)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return MERR_CANNOT_JOIN_CHANNEL;
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return MERR_CANNOT_JOIN_CHANNEL;

	// 개발자나 영자는 레벨에 제한없음..
	if(!IsAdminGrade(pObj)) 
	{
		// 인원체크
		if ((int)pChannel->GetObjCount() >= pChannel->GetMaxPlayers())
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_MAXPLAYERS;
		}

		// 레벨 하한체크
		if ( (pChannel->GetLevelMin() > 0) && (pChannel->GetLevelMin() > pObj->GetCharInfo()->m_nLevel) )
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_LEVEL;
		}
		// 레벨 상한체크
		if ( (pChannel->GetLevelMax() > 0) && (pChannel->GetLevelMax() < pObj->GetCharInfo()->m_nLevel) )
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_LEVEL;
		}

		// 뉴비채널 체크
		if ((pChannel->GetRuleType() == MCHANNEL_RULE_NEWBIE) && (pObj->IsNewbie() == false)) 
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_NEWBIE;
		}
	}

	return MOK;
}


int MMatchServer::ValidateEquipItem(MMatchObject* pObj, MMatchItem* pItem, const MMatchCharItemParts parts)
{
	if (! IsEnabledObject(pObj)) return MERR_UNKNOWN;
	if (pItem == NULL) return MERR_UNKNOWN;
	
	if (!IsEquipableItem(pItem->GetDescID(), pObj->GetCharInfo()->m_nLevel, pObj->GetCharInfo()->m_nSex))
	{
		return MERR_LOW_LEVEL;
	}

	// 무게 체크
	int nWeight = 0;
	int nMaxWeight = 0;

	MMatchEquipedItem* pEquipedItem = &pObj->GetCharInfo()->m_EquipedItem;
	pObj->GetCharInfo()->GetTotalWeight(&nWeight, &nMaxWeight);

	// 교체할 아이템의 무게를 뺀다.
	if (!pEquipedItem->IsEmpty(parts))
	{
		if (pEquipedItem->GetItem(parts)->GetDesc() != NULL)
		{
			nWeight -= pEquipedItem->GetItem(parts)->GetDesc()->m_nWeight;
			nMaxWeight -= pEquipedItem->GetItem(parts)->GetDesc()->m_nMaxWT;
		}
	}

	// 장착할 아이템의 무게를 더한다.
	if (pItem->GetDesc() != NULL)
	{
		nWeight += pItem->GetDesc()->m_nWeight;
	}
	
	if (nWeight > nMaxWeight)
	{
		return MERR_TOO_HEAVY;
	}


	// 양슬롯에 같은 무기를 장비하려는지 체크
	if ((parts == MMCIP_PRIMARY) || (parts == MMCIP_SECONDARY))
	{
		MMatchCharItemParts tarparts = MMCIP_PRIMARY;
		if (parts == MMCIP_PRIMARY) tarparts = MMCIP_SECONDARY;

		if (!pEquipedItem->IsEmpty(tarparts))
		{
			MMatchItem* pTarItem = pEquipedItem->GetItem(tarparts);
			if (pTarItem)
			{
				if (pTarItem->GetDescID() == pItem->GetDescID())
				{
					return MERR_CANNOT_EQUIP_EQUAL_ITEM;
				}
			}
		}
	}

	return MOK;
}

void MMatchServer::OnNetClear(const MUID& CommUID)
{
	MMatchObject* pObj = GetObject(CommUID);
	if (pObj)
		OnCharClear(pObj->GetUID());

	MAgentObject* pAgent = GetAgent(CommUID);
	if (pAgent)
		AgentRemove(pAgent->GetUID(), NULL);

	MServer::OnNetClear(CommUID);
}

void MMatchServer::OnNetPong(const MUID& CommUID, unsigned int nTimeStamp)
{
	MMatchObject* pObj = GetObject(CommUID);
	if (pObj) {
		pObj->UpdateTickLastPacketRecved();	// Last Packet Timestamp
		pObj->AddPing(GetGlobalClockCount() - nTimeStamp);
	}
}

void MMatchServer::OnHShieldPong(const MUID& CommUID, unsigned int nTimeStamp)
{
	MMatchObject* pObj = GetObject(CommUID);
	if (pObj) 
		pObj->UpdateLastHShieldMsgRecved();	// Last Packet Timestamp
}

void MMatchServer::UpdateCharDBCachingData(MMatchObject* pObject)
{
	if (! IsEnabledObject(pObject)) return;

	int	nAddedXP, nAddedBP, nAddedKillCount, nAddedDeathCount;

	nAddedXP = pObject->GetCharInfo()->GetDBCachingData()->nAddedXP;
	nAddedBP = pObject->GetCharInfo()->GetDBCachingData()->nAddedBP;
	nAddedKillCount = pObject->GetCharInfo()->GetDBCachingData()->nAddedKillCount;
	nAddedDeathCount = pObject->GetCharInfo()->GetDBCachingData()->nAddedDeathCount;

	if ((nAddedXP != 0) || (nAddedBP != 0) || (nAddedKillCount != 0) || (nAddedDeathCount != 0))
	{
		MAsyncDBJob_UpdateCharInfoData* pJob = new MAsyncDBJob_UpdateCharInfoData();
		pJob->Input(pObject->GetCharInfo()->m_nCID, 
					nAddedXP,
					nAddedBP,
					nAddedKillCount,
					nAddedDeathCount);
		PostAsyncJob(pJob);

		// 실패했는지는 알 수 없지만, 악용을 위해 Reset한다.
		pObject->GetCharInfo()->GetDBCachingData()->Reset();

/*
		if (m_MatchDBMgr.UpdateCharInfoData(pObject->GetCharInfo()->m_nCID,
			nAddedXP, nAddedBP, nAddedKillCount, nAddedDeathCount))
		{
			pObject->GetCharInfo()->GetDBCachingData()->Reset();
			st_ErrCounter = 0;
		}
		else
		{
			Log(LOG_ALL, "DB Query(UpdateCharDBCachingData > UpdateCharInfoData) Failed\n");

			LOG(LOG_ALL, "[CRITICAL ERROR] DB Connection Lost. ");

			m_MatchDBMgr.Disconnect();
			InitDB();

			st_ErrCounter++;
			if (st_ErrCounter > MAX_DB_QUERY_COUNT_OUT) 
			{
				LOG(LOG_ALL, "[CRITICAL ERROR] UpdateCharInfoData - Shutdown");
				Shutdown();
			}
		}
*/
	}
}

// item xml 체크용 - 테스트
bool MMatchServer::CheckItemXML()
{
	map<unsigned long int, string>	ItemXmlMap;

	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(FILENAME_ITEM_DESC))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MICTOK_ITEM))
		{
			unsigned long int id;
			int n;
			char szItemName[256];
			chrElement.GetAttribute(&n, MICTOK_ID);
			id = n;
			chrElement.GetAttribute(szItemName, MICTOK_NAME);

			if (ItemXmlMap.find(id) != ItemXmlMap.end())
			{
				_ASSERT(0);	// 아이템 ID 중복
				char szTemp[256];
				sprintf_safe(szTemp, "item xml 아이디 중복: %u\n", id);
				mlog(szTemp);
				return false;
			}
			ItemXmlMap.insert(map<unsigned long int, string>::value_type(id, string(szItemName)));
		}
	}

	xmlIniData.Destroy();

	FILE* fp = nullptr;
	fopen_s(&fp, "item.sql", "wt");
	if (!fp)
	{
		MLog("Failed to open item.sql\n");
		return false;
	}

	for (map<unsigned long int, string>::iterator itor = ItemXmlMap.begin();
		itor != ItemXmlMap.end(); ++itor)
	{
		char szTemp2[256];
		unsigned long int id = (*itor).first;
		size_t pos = (*itor).second.find( ":" );
		if( string::npos == pos ) 
		{
			ASSERT( 0 && "구분자를 찾지 못함. 문법오류." );
			continue;
		}

		string name = (*itor).second.c_str() + pos + 1;

		if( 0 == _stricmp("nomsg", MGetStringResManager()->GetString(name)) )
			mlog( "Item : %s\n", name.c_str() );
		
		sprintf_safe(szTemp2, "INSERT INTO Item (ItemID, Name) Values (%u, '%s')\n", // id, name.c_str() );
			id, MGetStringResManager()->GetString(name) );

		fputs(szTemp2, fp);
	}

	fputs("\n\n--------------------------------------\n\n", fp);

	for (MMatchItemDescMgr::iterator itor = MGetMatchItemDescMgr()->begin(); 
		itor != MGetMatchItemDescMgr()->end(); ++itor)
	{
		MMatchItemDesc* pItemDesc = (*itor).second;

		int nIsCashItem = 0;
		int nResSex=1, nResLevel=0, nSlot=0, nWeight=0, nHP=0, nAP=0, nMaxWT=0;

		int nDamage=0, nDelay=0, nControl=0, nMagazine=0, nReloadTime=0, nSlugOutput=0, nMaxBullet=0;

		if (pItemDesc->IsCashItem()) nIsCashItem=1;
		switch (pItemDesc->m_nResSex)
		{
		case 0: nResSex = 1; break;
		case 1: nResSex = 2; break;
		case -1: nResSex = 3; break;
		}

		nResLevel = pItemDesc->m_nResLevel;
		nWeight = pItemDesc->m_nWeight;
		nHP = pItemDesc->m_nHP;
		nAP = pItemDesc->m_nAP;
		nMaxWT = pItemDesc->m_nMaxWT;

		switch (pItemDesc->m_nSlot)
		{
		case MMIST_MELEE: nSlot = 1; break;
		case MMIST_RANGE: nSlot = 2; break;
		case MMIST_CUSTOM: nSlot = 3; break;
		case MMIST_HEAD: nSlot = 4; break;
		case MMIST_CHEST: nSlot = 5; break;
		case MMIST_HANDS: nSlot = 6; break;
		case MMIST_LEGS: nSlot = 7; break;
		case MMIST_FEET: nSlot = 8; break;
		case MMIST_FINGER: nSlot = 9; break;
		case MMIST_EXTRA: nSlot = 9; break;
		}


		nDamage = pItemDesc->m_nDamage;
		nDelay = pItemDesc->m_nDelay;
		nControl = pItemDesc->m_nControllability;
		nMagazine = pItemDesc->m_nMagazine;
		nReloadTime = pItemDesc->m_nReloadTime;
		if (pItemDesc->m_bSlugOutput) nSlugOutput=1;
		nMaxBullet = pItemDesc->m_nMaxBullet;

		fprintf(fp, "UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=%d, Delay=%d, Controllability=%d, Magazine=%d, ReloadTime=%d, SlugOutput=%d, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=%d, LimitSpeed=%d, IsCashItem=%d, \n",
			nDamage, nDelay, nControl, nMagazine, nReloadTime, nSlugOutput, nMaxBullet, 
			pItemDesc->m_nLimitSpeed, nIsCashItem);

		fprintf(fp, "ResSex=%d, ResLevel=%d, Slot=%d, Weight=%d, HP=%d, AP=%d, MAXWT=%d, \n",
			nResSex, nResLevel, nSlot, nWeight, nHP, nAP, nMaxWT);

		fprintf(fp, "Description='%s' \n", pItemDesc->m_szDesc);

		// 이거 절대로 지우지 마세요. DB작업할때 대형 사고 날수 있습니다. - by SungE.
		fprintf(fp, "WHERE ItemID = %u\n", pItemDesc->m_nID );

		/*
		fprintf(fp, "UPDATE Item SET Slot = %d WHERE ItemID = %u AND Slot IS NULL\n", nSlot, pItemDesc->m_nID );
		*/
	}
	


	fclose(fp);

	return true;

}

// sql파일 생성을 위해서. 게임을 위해서 사용되지는 않음.
struct ix
{
	string id;
	string name;
	string desc;
};

bool MMatchServer::CheckUpdateItemXML()
{
	// map<unsigned long int, string>	ItemXmlMap;
	
	map< string, ix > imName;
	map< string, ix > imDesc;

	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile("strings.xml"))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, "STR"))
		{
			ix tix;
			char szID[256] = {0,};
			char szInfo[256] = {0,};

			chrElement.GetAttribute(szID, "id");
			chrElement.GetContents( szInfo );

			if( 0 == strncmp("ZITEM_NAME_", szID, 11) )
			{
				if( imName.end() != imName.find(szID) )
				{
					ASSERT( "중복" );
					continue;
				}

				tix.id = szID;
				tix.name = szInfo;

				imName.insert( map<string, ix>::value_type(szID, tix) );
			}
			else if( 0 == strncmp("ZITEM_DESC_", szID, 11) )
			{
				if( imDesc.end() != imDesc.find(szID) )
				{
					ASSERT( "중복" );
					continue;
				}

				tix.id = szID;
				tix.desc = szInfo;

				imDesc.insert( map<string, ix>::value_type(szID, tix) );
			}
			else
			{
				// ASSERT( 0 && "이상하다...." );
			}
		}
	}

	int ic, dc;
	ic = static_cast< int >( imName.size() );
	dc = static_cast< int >( imDesc.size() );

	xmlIniData.Destroy();

	map< string, ix >::iterator it, end;
	it = imName.begin();
	end = imName.end();
	FILE* fpName;
	fopen_s(&fpName, "name.sql", "w");
	for( ; it != end; ++it )
	{
		char szID[ 128 ];
		string a = it->second.name;
		strcpy_safe(szID, it->second.id.c_str() + 11);

		unsigned int nID = static_cast< unsigned long >( atol(szID) );
		int k = 0;

		fprintf( fpName, "UPDATE Item SET Name = '%s' WHERE ItemID = %d\n",
			it->second.name.c_str(), nID );
	}
	fclose( fpName );

	it = imDesc.begin();
	end = imDesc.end();
	FILE* fpDesc = nullptr;
	fopen_s(&fpDesc, "desc.sql", "w");
	if (!fpDesc)
	{
		MLog("Failed to open desc.spl\n");
		return false;
	}

	for( ; it != end; ++it )
	{
		char szID[ 128 ];
		string a = it->second.name;
		strcpy_safe(szID, it->second.id.c_str() + 11);

		unsigned int nID = static_cast< unsigned long >( atol(szID) );
		int k = 0;

		char szQ[ 1024 ] = {0,};
		fprintf( fpDesc, "UPDATE Item SET Description = '%s' WHERE ItemID = %d\n",
			it->second.desc.c_str(), nID );
	}
	fclose( fpDesc );
	
	return true;
}


unsigned long int MMatchServer::GetStageListChecksum(MUID& uidChannel, int nStageCursor, int nStageCount)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return 0;

	unsigned long int nStageListChecksum = 0;
	int nRealStageCount = 0;

	for (int i = nStageCursor; i < pChannel->GetMaxPlayers(); i++)
	{
		if (nRealStageCount >= nStageCount) break;

		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		nStageListChecksum += pStage->GetChecksum();

		nRealStageCount++;
	}

	return nStageListChecksum;
}




void MMatchServer::BroadCastClanRenewVictories(const char* szWinnerClanName, const char* szLoserClanName, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_CLAN_RENEW_VICTORIES, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterString(szWinnerClanName));
	pCmd->AddParameter(new MCommandParameterString(szLoserClanName));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToAllClient(pCmd);
}

void MMatchServer::BroadCastClanInterruptVictories(const char* szWinnerClanName, const char* szLoserClanName, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_CLAN_INTERRUPT_VICTORIES, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterString(szWinnerClanName));
	pCmd->AddParameter(new MCommandParameterString(szLoserClanName));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToAllClient(pCmd);
}

void MMatchServer::BroadCastDuelRenewVictories(const MUID& chanID, const char* szChampionName, const char* szChannelName, int nRoomNumber, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_DUEL_RENEW_VICTORIES, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterString(szChampionName));
	pCmd->AddParameter(new MCommandParameterString(szChannelName));
	pCmd->AddParameter(new MCommandParameterInt(nRoomNumber));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToChannel(chanID, pCmd);
}

void MMatchServer::BroadCastDuelInterruptVictories(const MUID& chanID, const char* szChampionName, const char* szInterrupterName, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_DUEL_INTERRUPT_VICTORIES, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterString(szChampionName));
	pCmd->AddParameter(new MCommandParameterString(szInterrupterName));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToChannel(chanID, pCmd);
}


bool MMatchServer::InitScheduler()
{
	// 스케쥴 업데이트시 커멘드를 포스트하기 위해서,
	//  MMatchServer의 주소를 인자로 받아 멤버로 저장해둠.
	m_pScheduler = new MMatchScheduleMgr( this );
	if( 0 == m_pScheduler )		
		return false;

	if( !m_pScheduler->Init() ){
		delete m_pScheduler;
		m_pScheduler = 0;
		return false;
	}

	// 검사 시간을 10초로 설정. 임시.
	m_pScheduler->SetUpdateTerm( 10 );

	// 상속한 클래스의 스케쥴 등록.
	if( !InitSubTaskSchedule() ){
		delete m_pScheduler;
		m_pScheduler = 0;
		return false;
	}

	return true;
}


bool MMatchServer::InitLocale()
{
	if( MGetServerConfig()->IsComplete() )
	{
		
		MGetLocale()->Init( GetCountryID(MGetServerConfig()->GetLanguage().c_str()) );
	}
	else
	{
		ASSERT( 0 && "'MMatchConfig' is must be completed befor init 'MMatchLocale'." );
		return false;
	}

	MGetStringResManager()->Init( "", MGetLocale()->GetCountry() );	// 실행파일과 같은 폴더에 xml파일이 있다.

	return true;
}


bool MMatchServer::InitCountryFilterDB()
{
	IPtoCountryList icl;
	BlockCountryCodeList bccl;
	CustomIPList cil;

	/* 실시간으로 caching하기로 함.
	if( !GetDBMgr()->GetIPtoCountryList(icl) )
	{
		ASSERT( 0 && "Fail to init IPtoCountryList.\n" );
		mlog( "Fail to init IPtoCountryList.\n" );
		return false;
	}
	*/

	if( !GetDBMgr()->GetBlockCountryCodeList(bccl) )
	{
		ASSERT( 0 && "Fail to init BlockCoutryCodeList.\n" );
		mlog( "Fail to init BlockCountryCodeList.\n" );
		return false;
	}

	if( !GetDBMgr()->GetCustomIPList(cil) )
	{
		ASSERT( 0 && "Fail to init CustomIPList.\n" );
		mlog( "Fail to init CustomIPList.\n" );
		return false;
	}

	if( !GetCountryFilter().Create(bccl, icl, cil) )
	{
		ASSERT( 0 && "Fail to create country filter.\n" );
		mlog( "Fail to create country filter.\n" );
		return false;
	}

	return true;
}


void MMatchServer::SetUseCountryFilter()
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_UPDATE_USE_COUNTRY_FILTER, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


void MMatchServer::UpdateIPtoCountryList()
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_GET_DB_IP_TO_COUNTRY, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


void MMatchServer::UpdateBlockCountryCodeLsit()
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_GET_DB_BLOCK_COUNTRY_CODE, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


void MMatchServer::UpdateCustomIPList()
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_GET_DB_CUSTOM_IP, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


void MMatchServer::SetAccetpInvalidIP()
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_UPDATE_ACCEPT_INVALID_IP, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


bool MMatchServer::CheckIsValidIP( const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter )
{
	switch( CheckIsValidCustomIP(CommUID, strIP, strCountryCode3, bUseFilter) )
	{
	case CIS_INVALID :
		{
			switch( CheckIsNonBlockCountry(CommUID, strIP, strCountryCode3, bUseFilter) )
			{
			case CCS_NONBLOCK :
				{
					return true;
				}
				break;

			case CCS_BLOCK :
				{
				}
				break;

			case CCS_INVALID :
				{
					return MGetServerConfig()->IsAcceptInvalidIP();
				}
				break;

			default :
				{
					ASSERT( 0 );
				}
				break;
			}
		}
		break;

	case CIS_NONBLOCK :
		{
			return true;
		}
		break;

	case CIS_BLOCK :
		{
			return false;
		}
		break;

	default : 
		{
			ASSERT( 0 );
		}
		break;
	}
	
	return false;
}

const CUSTOM_IP_STATUS MMatchServer::CheckIsValidCustomIP( const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter )
{
	string strComment;
	bool bIsBlock = false;

	if( !GetCountryFilter().GetCustomIP(strIP, bIsBlock, strCountryCode3, strComment) )
		return CIS_INVALID;

	if( bUseFilter && bIsBlock )
	{
		MCommand* pCmd = CreateCommand(MC_RESPONSE_BLOCK_COUNTRYCODE, CommUID);
		if( 0 != pCmd )
		{
			pCmd->AddParameter( new MCommandParameterString(strComment.c_str()) );
			Post( pCmd );
		}

		if( 3 != strCountryCode3.length() )
			strCountryCode3 = "er_";
		return CIS_BLOCK;
	}
	
	return CIS_NONBLOCK;
}


const COUNT_CODE_STATUS MMatchServer::CheckIsNonBlockCountry( const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter )
{
	if( !bUseFilter )
		return CCS_NONBLOCK;

	if( GetCountryFilter().GetIPCountryCode(strIP, strCountryCode3) )
	{
		string strRoutingURL;

		if( GetCountryFilter().IsNotBlockCode(strCountryCode3, strRoutingURL) )
			return CCS_NONBLOCK;
		else 
		{
			MCommand* pCmd = CreateCommand(MC_RESPONSE_BLOCK_COUNTRYCODE, CommUID);
			if( 0 != pCmd )
			{
				pCmd->AddParameter( new MCommandParameterString(strRoutingURL.c_str()) );
				Post( pCmd );
			}
			return CCS_BLOCK;
		}
	}	
	else
	{
		DWORD dwIPFrom = 0;
		DWORD dwIPTo = 0;
		
		// IP를 포함하고 있는 범위의 정보를 DB에서 새로 가져옴.
		if( GetDBMgr()->GetIPContryCode(strIP, dwIPFrom, dwIPTo, strCountryCode3) )
		{
			// 새로운 IP범위를 리스트에 추가 함.
			if( !GetCountryFilter().AddIPtoCountry(dwIPFrom, dwIPTo, strCountryCode3) )
			{
				mlog( "MMatchServer::CheckIsNonBlockCountry - add new IPtoCountry(f:%u, t%u, c:%s) fail.\n",
					dwIPFrom, dwIPTo, strCountryCode3.c_str() );
			}

			// 해당 IP가 블럭 국가의 IP인지 검사.
			string strRoutingURL;
			if( GetCountryFilter().IsNotBlockCode(strCountryCode3, strRoutingURL) )
				return CCS_NONBLOCK;
		}

		// 여기까지 내려오면 현제 IPtoCountry테이블에 등록되지 않은 IP.
		if( MGetServerConfig()->IsAcceptInvalidIP() )
		{
			strCountryCode3 = "N/S";
			return CCS_INVALID;
		}
		else
		{
			MCommand* pCmd = CreateCommand(MC_RESPONSE_BLOCK_COUNTRYCODE, CommUID);
			if( 0 != pCmd )
			{
				pCmd->AddParameter( new MCommandParameterString(strCountryCode3.c_str()) );
				Post( pCmd );
			}
			return CCS_INVALID;
		}
	}

	return CCS_BLOCK;
}

bool MMatchServer::InitEvent()
{
	if( !MMatchEventDescManager::GetInstance().LoadEventXML(EVENT_XML_FILE_NAME) )
	{
		ASSERT( 0 && "fail to Load Event.xml" );
		mlog( "MMatchServer::InitEvent - fail to Load %s\n", 
			EVENT_XML_FILE_NAME );
		return false;
	}

	if( !MMatchEventFactoryManager::GetInstance().LoadEventListXML(EVENT_LIST_XML_FILE_NAME) )
	{
		ASSERT( 0 && "fail to load EventList.xml" );
		mlog( "MMatchServer::InitEvent - fail to Load %s\n",	
			EVENT_LIST_XML_FILE_NAME );
		return false;
	}

	MMatchEventFactoryManager::GetInstance().SetUsableState( MGetServerConfig()->IsUseEvent() );

	EventPtrVec EvnPtrVec;
	if( !MMatchEventFactoryManager::GetInstance().GetEventList(MMATCH_GAMETYPE_ALL, ET_CUSTOM_EVENT, EvnPtrVec) )
	{
		ASSERT( 0 && "이벤트 리스트 생성 실패.\n" );
		mlog( "MMatchServer::InitEvent - 리스트 생성 실패.\n" );
		MMatchEventManager::ClearEventPtrVec( EvnPtrVec );
		return false;
	}
	m_CustomEventManager.ChangeEventList( EvnPtrVec );
	
	return true;
}


void MMatchServer::CustomCheckEventObj( const DWORD dwEventID, MMatchObject* pObj, void* pContext )
{
	m_CustomEventManager.CustomCheckEventObj( dwEventID, pObj, pContext );
}

void MMatchServer::SendHShieldReqMsg()
{
	//{{RouteToAllClient HShieldReqMsg
	MCommand* pCommand = CreateCommand(MC_HSHIELD_PING, MUID(0,0));
	pCommand->AddParameter(new MCmdParamUInt(GetGlobalClockCount()));


	unsigned long HSOption = ANTICPSVR_CHECK_GAME_MEMORY;
	m_HSCheckCounter++;
	if(m_HSCheckCounter == 3)
	{
		m_HSCheckCounter = 0;
		HSOption = ANTICPSVR_CHECK_ALL;
	}

	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)((*i).second);

		// 이전 ReqMsg에 대한 응답(m_bHShieldMsgRecved)이 있는 클라이언트들에게만 보낸다. 그렇지 않은 경우 골치아픔. 새로 보냈는데 이전 ReqMsg에 대한 응답이 온다던가..
		if (pObj->GetUID() < MUID(0,3) || !pObj->GetHShieldMsgRecved()) continue;	// MUID로 Client인지 판별할수 있는 코드 필요함

		unsigned char* pbyReqMsg = pObj->GetHShieldInfo()->m_pbyReqMsg;
		unsigned char* pbyReqInfo = pObj->GetHShieldInfo()->m_pbyReqInfo;

		memset(pbyReqMsg, 0, sizeof(pbyReqMsg));
		memset(pbyReqInfo, 0, sizeof(pbyReqInfo));

		// 이 경우는 앞에서의 AnalyzeGuidAckMsg에서 에러가 났지만 접속이 끊기기 전에 타이머 타이밍이 맞아 이 루틴이 호출되는 경우이다.
		// 어차피 핵 판정 났으니 여기서 빠져나간다.
		if(!pObj->GetHShieldInfo()->m_bGuidAckPass)
		{
//			LOG(LOG_FILE, "%s's CrcInfo is NULL.", pObj->GetAccountName());
			break;
		}

		DWORD dwRet = MGetMatchServer()->HShield_MakeReqMsg(pObj->GetHShieldInfo()->m_pCRCInfo, pbyReqMsg, pbyReqInfo, HSOption);

		if(dwRet != ERROR_SUCCESS)
			LOG(LOG_FILE, "@MakeReqMsg - %s Making Req Msg Failed. (Error code = 0x%x, CrcInfo Address = 0x%x)", pObj->GetAccountName(), dwRet, pObj->GetHShieldInfo()->m_pCRCInfo);

		void* pBlob = MMakeBlobArray(sizeof(unsigned char), SIZEOF_REQMSG);
		unsigned char* pCmdBlock = (unsigned char*)MGetBlobArrayElement(pBlob, 0);
		CopyMemory(pCmdBlock, pbyReqMsg, SIZEOF_REQMSG);

		pCommand->AddParameter(new MCmdParamBlob(pBlob, MGetBlobArraySize(pBlob)));

		MCommand* pSendCmd = pCommand->Clone();
		pSendCmd->m_Receiver = pObj->GetUID();
		Post(pSendCmd);

		pObj->SetHShieldMsgRecved(false);	// 새로 보냈으니 초기화해야지
		MEraseBlobArray(pBlob);
		pCommand->ClearParam(1);
	}	

	delete pCommand;
}