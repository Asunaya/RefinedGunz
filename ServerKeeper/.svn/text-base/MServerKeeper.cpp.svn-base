#include "stdafx.h"
#include "MServerKeeper.h"
#include "winsock2.h"
#include "MXml.h"
#include "MProcessController.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "FileInfo.h"
#include "Updater.h"
#include "MMatchServer.h"
#include "MMatchSchedule.h"
#include "MCommandBuilder.h"
#include <process.h>


/*
bool FindModule(const char* pszModulePath, MODULEENTRY32* pOutME32)
{
	BOOL          bRet        = FALSE; 
	BOOL          bFound      = FALSE; 
	HANDLE        hModuleSnap = NULL; 

	MODULEENTRY32 me32        = {0}; 
	me32.dwSize = sizeof(MODULEENTRY32); 

	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0); 
	if (hModuleSnap == INVALID_HANDLE_VALUE) 
		return false; 

	bool bResult = false;
	if (Module32First(hModuleSnap, &me32)) 
	{ 
		do { 
			TRACE("ModuleEnum: %s \n", me32.szExePath);
			if (stricmp(pszModulePath, me32.szExePath) == 0) {
				CopyMemory(pOutME32, &me32, sizeof(MODULEENTRY32));
				bResult = true;
			}
		} while (Module32Next(hModuleSnap, &me32)); 
	}
	CloseHandle (hModuleSnap);

	return bResult;
}
*/

MServerKeeper::MServerKeeper() : m_bIsConnectMatchServer( false ), 
	m_ConnectionState( CNN_START_CHECK ), m_bIsSendPing( false ), m_pScheduler( 0 )
{
	m_This = ( KEEPERSERVER_UID );
	m_uidNextAlloc.Increase(11);
	m_dwRecvPingTime = timeGetTime();

	InitializeCriticalSection( &m_csServerState );

	m_strServerResVer.clear();
	m_strServerLastMod.clear();
	m_strAgentLastMod.clear();
	m_strKeeperLastMod.clear();
}


MServerKeeper::~MServerKeeper()
{
	DisconnectMatchServer();
	DisconnectKeeperMgr();

	Destroy();

	SaveConfig();

	if( 0 != m_pScheduler )
	{
		delete m_pScheduler;
		m_pScheduler = 0;
	}
}


MUID MServerKeeper::UseUID(void)
{
	if( !IsConnectKeeperManager() )
		return MUID( 0, 5 );

	return MUID( 0, 0 );
}

void MServerKeeper::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MCommandCommunicator::OnRegisterCommand(pCommandManager);
	MAddSharedCommandTable(pCommandManager, MSCT_AGENT);
	//Log("Command registeration completed");
}



DWORD MServerKeeper::GetKeeperMgrPingElapseTime()
{
	return timeGetTime() - m_dwRecvPingTime;
}


void MServerKeeper::SendPingToPatchInterface()
{
	if( m_KeeperMgrObj.isConnected() )
	{
		// 체크할만한 충분한 시간이 경과되었는지 검사.
		if( MAX_ELAPSE_RESPONSE_REQUEST_HEARBEAT < GetKeeperMgrPingElapseTime() )
		{
			// 이전에 핑을 보냈으면 문제가 있는것으로 간주하고 접속 종료.
			if( !IsKeeperMgrPingSend() )
			{
				MCommand* pCmd = CreateCommand( MC_CHECK_KEEPER_MANAGER_PING, m_KeeperMgrObj.GetUID() );
				if( 0 != pCmd )
				{
					PostSafeQueue( pCmd );

					m_dwRecvPingTime = timeGetTime();
					SetKeeperMgrPingState( true );
#ifdef _DEBUG
					mlog( "MServerKeeper::SendPingToPatchInterface - send ping.\n" );
#endif
				}
				else
				{
					mlog( "MServerKeeper::SendPingToPatchInterface - Fail to create ping command of updater\n" );
				}
			}
			else
			{
				DisconnectKeeperMgr();
				mlog( "MServerKeeper::SendPingToPatchInterface - dissconnect ping time elepsed.\n" );
			}
		}
	}
}


void MServerKeeper::OnCheckPing( MCommand* pCommand )
{
	if( (0 != pCommand) && m_KeeperMgrObj.isConnected() )
	{
		// 마지막 받은 시간을 최근의 시간으로 업데이트.
		m_dwRecvPingTime = timeGetTime();

		SetKeeperMgrPingState( false );

		OnRequestServerAndAgentState( m_KeeperMgrObj.GetUID() );
	}
}


void MServerKeeper::OnRun(void)
{
	// 서버의 상태 정보 요청에 대한 응답 경과를 검사함.
	CheckElapseRequestServerHearbeat();

	if( 0 != m_pScheduler )
		m_pScheduler->Update(); // 등록된 스케줄 업데이트.
}

int MServerKeeper::OnConnected(MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj)
{
	
	int nRetCode = MServer::OnConnected(pTargetUID, pAllocUID, nTimeStamp, pCommObj);
	if (nRetCode != MOK) return nRetCode;

	m_uidMatchServer = *pTargetUID;

	pCommObj->GetCommandBuilder()->SetCheckCommandSN( false );
	
	RequestConnectServer();

	return MOK;
}


void MServerKeeper::OnLocalLogin(const MUID& uidComm)
{
}

bool MServerKeeper::LoadFromXml(const char* pszFileName)
{
	#define MTOK_SERVERITEM			"SERVERITEM"
	#define MTOK_VITALCHECK			"VITALCHECK"
	#define MTOK_SERVERTYPE			"SERVERTYPE"
	#define MTOK_NAME				"NAME"
	#define	MTOK_PATH				"PATH"
	#define MTOK_ATTR_SLOTID		"slotid"

	MXmlDocument	xmlIniData;
	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(pszFileName))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, itemElement;
	char szTagName[256];
	char szBuf[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i=0; i<iCount; i++)
	{
		itemElement = rootElement.GetChildNode(i);
		itemElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, MTOK_SERVERITEM))
		{
			int nSlotID = 0;
			bool bVitalCheck = false;
			char szServerType[128] = "";
			char szName[128] = "";
			char szPath[_MAX_DIR] = "";

			itemElement.GetAttribute(szBuf, "slotid");
			nSlotID = atoi(szBuf);

			MXmlElement childElement;
			int nChildCount = itemElement.GetChildNodeCount();
			for (int j=0; j<nChildCount; j++) 
			{
				childElement = itemElement.GetChildNode(j);
				childElement.GetTagName(szTagName);
				if (szTagName[0] == '#') continue;

				if (!strcmp(szTagName, MTOK_VITALCHECK)) {
					childElement.GetContents(szBuf);
					if (!stricmp("true", szBuf))
						bVitalCheck = true;
					else
						bVitalCheck = false;
				} else if (!strcmp(szTagName, MTOK_SERVERTYPE)) {
					childElement.GetContents(szBuf);
					strcpy(szServerType, szBuf);
				} else if (!strcmp(szTagName, MTOK_NAME)) {
					childElement.GetContents(szBuf);
					strcpy(szName, szBuf);
				} else if (!strcmp(szTagName, MTOK_PATH)) {
					childElement.GetContents(szBuf);
					strcpy(szPath, szBuf);
				}
			}

			// Add ServerItem
			MServerItem* pItem = new MServerItem(nSlotID, bVitalCheck, szServerType, szName, szPath);
			AddItem(pItem);
		}
	}

	xmlIniData.Destroy();
	return true;
}

MServerItem* MServerKeeper::FindServerItemBySlotID(int nSlotID)
{
	MServerItemList* pList = GetServerItemList();
	for (MServerItemList::iterator i=pList->begin();i!=pList->end(); i++) {
		MServerItem* pServerItem = (*i);
		if (pServerItem->GetSlotID() == nSlotID)
			return pServerItem;
	}
	return NULL;
}

bool MServerKeeper::StartServer(MServerItem* pServerItem)
{
	UpdateServer(pServerItem);
	MProcessController::StartProcess(pServerItem->GetServerPath());
	return false;
}

bool MServerKeeper::StopServer(MServerItem* pServerItem)
{
	HANDLE hProcess = MProcessController::OpenProcessHandleByFilePath(pServerItem->GetServerPath());
	return MProcessController::StopProcess(hProcess);
}

bool MServerKeeper::UpdateServer(MServerItem* pServerItem)
{
	STARTUPINFO sui;
	PROCESS_INFORMATION pi;

	char szDir[_MAX_DIR], szBat[_MAX_DIR];
	FILE* file;
	CString upFile = pServerItem->GetServerPath(), bkFile = upFile, setupFile;

	ZeroMemory(&sui, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	sui.cb = sizeof(STARTUPINFO);

	GetPurePath(szDir, pServerItem->GetServerPath());
	
	setupFile = szDir;
	setupFile += "setup.exe";
	
	strcpy(szBat, szDir);
	strcat(szBat, "setup.bat");

	upFile += ".update";
	bkFile += ".bak";

	if(file = fopen(szBat, "r")){
		fclose(file);

		if(CreateProcess(NULL, _T(szBat), NULL, NULL, FALSE, 0, NULL, _T(szDir), &sui, &pi)){
			WaitForSingleObject(pi.hProcess, INFINITE);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			DeleteFile(szBat);
		}
	}

	if(file = fopen(setupFile, "r")){
		fclose(file);
		if(_spawnl(_P_WAIT, setupFile, setupFile, NULL) != -1){
			DeleteFile(setupFile);
		}
	}

	if(file = fopen(upFile,"r")){
		fclose(file);
		CopyFile(pServerItem->GetServerPath(), bkFile, FALSE);
		MoveFileEx(upFile, pServerItem->GetServerPath(), MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
	}
   
	return true;
}


bool MServerKeeper::Create()
{
	if( !GetUpdater.Init() )
	{
		mlog( "업데이트 초기화 실패\n" );
		return false;
	}

	if( !MServer::Create(GetUpdater.GetUpdaterPort(), true) )
	{
		mlog( "포트 생성 실패\n" );
		return false;
	}

	m_PatchChkMap.Init();

	// MatchServer와 AgentServer의 자동실행이 xml파일을 로드해서 설정하기 되어있어서 아래로 수정함.
	// KeeperManager의 서정 저장내용을 적용하기 위해서, CServerKeeperDlg::OnInitDialog()에서 MServerItem을 등록후 
	//  InitConfigState()을 호출하여 설정 내용을 저장하게 함.
	// InitConfigState();

	if( 0 != m_pScheduler )
		delete m_pScheduler;
	m_pScheduler = new MMatchScheduleMgr( this );
	if( m_pScheduler->Init() )
		m_pScheduler->SetUpdateTerm( 10 ); // 10초에 한번씩 검사.
	else 
	{
		mlog( "스케줄러 초기화 실패\n" );
		return false;
	}
	
	return true;
}

void MServerKeeper::DisconnectKeeperMgr()
{
	Disconnect( m_KeeperMgrObj.GetUID() );

	m_KeeperMgrObj.SetConnection( false );
	m_KeeperMgrObj.SetUID( MUID(0, 0) );
	SetKeeperMgrPingState( false );	
}


bool MServerKeeper::ConnectMatchServer()
{
	MCommObject* pCommObj = new MCommObject( this );
	if( 0 == pCommObj )
	{
		return false;
	}

	// MatchServer로 접속을 함.
	pCommObj->SetUID( MATCHSERVER_UID );
	pCommObj->SetAddress( GetUpdater.GetServerIP().GetBuffer(), 6000 );

	SetMatchServerUID( pCommObj->GetUID() );

	if( MOK != Connect(pCommObj) )
	{
		m_bIsConnectMatchServer = false;
		return false;
	}

	// 연결되어있다는 설정은 MC_KEEPER_CONNECT_TO_MATCHSERVER커맨드 응답이 왔을때 설정.
	
	SetConnectionState( CNN_CONNECTING_TO_SERVER );
	
	return true;
}


bool MServerKeeper::GetServerStatus()
{
	return true;

	MCommand* pCmd = CreateCommand( MC_REQUEST_MATCHSERVER_STATUS, m_uidMatchServer );
	if( 0 == pCmd )
		return false;

	PostSafeQueue( pCmd );
	
	return true;
}

void MServerKeeper::RequestServerHearbeat()
{
	if( IsConnectMatchServer() )
	{
		MCommand* pCmd = CreateCommand( MC_REQUEST_SERVER_HEARBEAT, m_uidMatchServer );
		if( 0 == pCmd )
			return;

		PostSafeQueue( pCmd );

		SetConnectionState( CNN_CHECKING_SERVER_HEARTBEAT );

		UpdateServerHeartbeatTime();
	}
}


void MServerKeeper::CheckElapseRequestServerHearbeat()
{
	if( IsStopServerHeartbeat() )
	{
		if( IsConnectMatchServer() )
		{
			// 상태 업데이트. 업데이트된 상테는 다음 거사때나, 응답이 있을때 PatchInterface로 전송됨.
			if( IsConnectionState(CNN_CONNECTED_SERVER) || IsConnectionState(CNN_START_CHECK) )
				RequestServerHearbeat();
			else
				DisconnectMatchServer(); // 만약 자동 연결이면 아래검사 부분에서 MatchServer로 연결을 시도함.
		}
		else if( IsConnectionState(CNN_CONNECTING_TO_SERVER) || IsConnectionState(CNN_START_CHECK) )
		{
			// 연결을 시도중이거나, Keeper가 방금 시작하였을 경우.
			SetConnectionState( CNN_CHECKING_SERVER_HEARTBEAT );

			UpdateServerHeartbeatTime();
		}
		else if( IsConnectionState(CNN_CHECKING_SERVER_HEARTBEAT) )
		{
			DisconnectMatchServer();
		}
		else if( IsConnectionState(CNN_NOT_CONNECTED_SERVER) )
		{
			// 자동 연결이면 연결 시도.
			/*
			if( !ConnectMatchServer() )
			{
				SetConnectionState( CNN_FAIL_CONNECT );
				if( m_KeeperMgrObj.isConnected() )
					OnResponseConnectionState();
			}
			*/
		}
		else
		{
			DisconnectMatchServer();
		}
	}
}


void MServerKeeper::DisconnectMatchServer()
{
	Disconnect( MATCHSERVER_UID );

	SetConnectionState( CNN_NOT_CONNECTED_SERVER );
	m_bIsConnectMatchServer = false;

#ifdef _DEBUG
	mlog( "MServerKeeper::DisconnectMatchServer - Dead:%d Elapsed:%d\n", isKeeperMgrObjLive(), timeGetTime() - m_dwStartCheckServerHearbeat );
#endif
}



MCommObject* MServerKeeper::GetObject( const MUID& uidObj )
{
	MUIDRefCache::iterator it = m_CommRefCache.find( uidObj );
	if( m_CommRefCache.end() == it )
		return 0;

	return reinterpret_cast<MCommObject*>(it->second);
}


void MServerKeeper::SetConnectionState( const CONNECTION_STATE nState )
{
	if( (0 <= nState) && (nState < CNN_END) )
	{
		EnterCriticalSection( &m_csServerState );

#ifdef _DEBUG
		if( CNN_NOT_CONNECTED_SERVER == nState )
		{
			int k = 0;
		}
#endif

		m_ConnectionState = nState;

		if( m_KeeperMgrObj.isConnected() )
			OnResponseConnectionState();

		LeaveCriticalSection( &m_csServerState );
	}
}


void MServerKeeper::UpdateServerHeartbeatTime( const DWORD dwTime )
{
	m_dwStartCheckServerHearbeat = dwTime;
}




bool MServerKeeper::IsKeeperMgrConnectionValid( const MUID& uidKeeperMgr )
{
	if( m_KeeperMgrObj.isConnected() && (uidKeeperMgr == m_KeeperMgrObj.GetUID()) )
		return true;

	return false;
}


void MServerKeeper::InitConfigState()
{
	LoadConfig();
}


void MServerKeeper::AddConfigState( const COLUMN_ID nColumnID, const CONFIG_STATE nConfigState )
{
	if( nColumnID < CONFIG_END )
	{
		ConfigStateMap::iterator it = m_ConfigStateMap.find( nColumnID );
		if( m_ConfigStateMap.end() == it )
		{
			m_ConfigStateMap.insert( ConfigStateMap::value_type(nColumnID, nConfigState) );
		}
		else
		{
			ASSERT( 0 );
		}
	}
	else
	{
		ASSERT( 0 );
	}
}


void MServerKeeper::SetConfig( const  COLUMN_ID nColumnID, const CONFIG_STATE nConfigState )
{
	if( nColumnID < CONFIG_END )
	{
		ConfigStateMap::iterator it = m_ConfigStateMap.find( nColumnID );
		if( m_ConfigStateMap.end() != it )
			it->second = nConfigState;
	}
}


const CONFIG_STATE MServerKeeper::GetConfigState( const COLUMN_ID nColumnID )
{
	if( nColumnID < CONFIG_END )
	{
		ConfigStateMap::iterator it = m_ConfigStateMap.find( nColumnID );
		if( m_ConfigStateMap.end() != it )
			return it->second;
	}
	
	return CONFIG_ERROR;
}

void MServerKeeper::SaveConfig()
{
	FILE* fp = fopen( CONFIG_SAVE_FILE, "w" );
	if( 0 == fp )
		return;

	ConfigStateMap::iterator it, end;
	end = m_ConfigStateMap.end();
	for( it = m_ConfigStateMap.begin(); it != end; ++it )
	{
		fprintf( fp, "%d %d\n", it->first, it->second );
#ifdef _DEBUG
		mlog( "COLUMN_ID:%d CONFIG_STATE:%d\n", it->first, it->second );
#endif
	}

	fclose( fp );
}


void MServerKeeper::LoadConfig()
{
	FILE* fp = fopen( CONFIG_SAVE_FILE, "r" );
	if( 0 == fp )
		return;

	COLUMN_ID		nColumnID;
	CONFIG_STATE	nConfigState;
	char			szBuf[ 128 ];

	ConfigStateMap::iterator it, end;
	end = m_ConfigStateMap.end();
	while( fgets(szBuf, 127, fp) )
	{
		sscanf( szBuf, "%d %d", &nColumnID, &nConfigState );
		
		AddtionalCheckConfigState( nColumnID, nConfigState );
		AddConfigState( nColumnID, nConfigState );		

#ifdef _DEBUG
		mlog( "COLUMN_ID:%d CONFIG_STATE:%d\n", nColumnID, nConfigState );
#endif
	}

	fclose( fp );
}


// 해당 설정에 추가작업이 필요한 부분.
void MServerKeeper::AddtionalCheckConfigState( const COLUMN_ID nColumnID, const CONFIG_STATE nConfigState )
{
	if( COLUMN_SERVER_START == nColumnID )
	{
		MServerItem* pServerItem = FindServerItemBySlotID( 0 );
		if( 0 != pServerItem )
		{
			if( CONFIG_AUTO_START_SERVER == nConfigState )
				pServerItem->SetVitalCheck( true );
			else if( CONFIG_MANUAL_START_SERVER == nConfigState )
				pServerItem->SetVitalCheck( false );
		}
	}
	else if( COLUMN_AGENT_START == nColumnID )
	{
		MServerItem* pServerItem = FindServerItemBySlotID( 1 );
		if( 0 != pServerItem )
		{
			if( CONFIG_AUTO_START_AGENT == nConfigState )
				pServerItem->SetVitalCheck( true );
			else if( CONFIG_MANUAL_START_AGENT == nConfigState )
				pServerItem->SetVitalCheck( false );
		}
	}
}


bool MServerKeeper::CreateServerDownloadWorkThread()
{
	HANDLE  hThread;
	DWORD   dwThreadId;

	hThread = CreateThread(NULL, 0, ServerDownloadWorkThread, this, 0, &dwThreadId);

	return (NULL != hThread);
}


bool MServerKeeper::CreateServerPatchWorkThread()
{
	HANDLE  hThread;
	DWORD   dwThreadId;

	hThread = CreateThread(NULL, 0, ServerPatchWorkThread, this, 0, &dwThreadId);

	return (NULL != hThread);
}

bool MServerKeeper::CreateServerPreparePatchWorkThread()
{
	HANDLE  hThread;
	DWORD   dwThreadId;

	hThread = CreateThread(NULL, 0, ServerPreparePatchWorkThread, this, 0, &dwThreadId);

	return (NULL != hThread);
}


DWORD WINAPI MServerKeeper::ServerDownloadWorkThread( void* pWorkContext )
{
	if( 0 == pWorkContext )
		return -1;
	
	MServerKeeper* pServerKeeper = reinterpret_cast< MServerKeeper* >( pWorkContext );

	if( GetUpdater.ConnectPatchFileServer() )
	{
		pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_CONNECT_FTP_SERVER, JOB_SUCCESS_CONNECT_FTP_SERVER );
		
		if( GetUpdater.DownloadServer() )
		{
			pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_DOWNLOAD_SERVER_PATCH_FILE, JOB_SUCCESS_DOWNLOAD_SERVER_PATCH_FILE );
			pServerKeeper->OnResponseLastJobState();
		}
		else
		{
			pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_DOWNLOAD_SERVER_PATCH_FILE, JOB_FAIL_DOWNLOAD_SERVER_PATCH_FILE );

			if( !GetUpdater.IsEnableServerPatch() )
				pServerKeeper->GetPatchCheckMap().Check( JOB_DISABLE_SERVER_PATCH, JOB_DISABLE_SERVER_PATCH );

			pServerKeeper->OnResponseLastJobState();
			
		}

		GetUpdater.DisconnectPatchFileServer();
	}
	else
	{
		pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_DOWNLOAD_SERVER_PATCH_FILE, JOB_FAIL_DOWNLOAD_SERVER_PATCH_FILE );
		pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_CONNECT_FTP_SERVER, JOB_FAIL_CONNECT_FTP_SERVER );
		pServerKeeper->OnResponseLastJobState();
	}

	return 0;
}


DWORD WINAPI MServerKeeper::ServerPreparePatchWorkThread( void* pWorkContext )
{
	if( 0 == pWorkContext )
		return -1;
	
	MServerKeeper* pServerKeeper = reinterpret_cast< MServerKeeper* >( pWorkContext );

	if( GetUpdater.PrepareServerPatching() )
	{
        pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_PREPARE_SERVER_PATCH, JOB_SUCCESS_PREPARE_SERVER_PATCH );
		pServerKeeper->OnResponseLastJobState();
	}
	else
	{
		pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_PREPARE_SERVER_PATCH, JOB_FAIL_PREPARE_SERVER_PATCH );
		pServerKeeper->OnResponseLastJobState();
	}

	return 0;
}


DWORD WINAPI MServerKeeper::ServerPatchWorkThread( void* pWorkContext )
{
	if( 0 == pWorkContext )
		return -1;

	MServerKeeper* pServerKeeper = reinterpret_cast< MServerKeeper* >( pWorkContext );

	if( pServerKeeper->GetPatchCheckMap().IsServerPrepareComplete() )
	{
		if( GetUpdater.ServerPatching() )
		{
			pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_PATCH_SERVER, JOB_SUCCESS_PATCH_SERVER );
			pServerKeeper->OnResponseLastJobState();
		}
		else
		{
			pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_PATCH_SERVER, JOB_FAIL_PATCH_SERVER );
			pServerKeeper->OnResponseLastJobState();
		}
	}

	return 0;
}


bool MServerKeeper::CreateAgentDownloadWorkThread()
{
	HANDLE  hThread;
	DWORD   dwThreadId;

	hThread = CreateThread(NULL, 0, AgentDownloadWorkThread, this, 0, &dwThreadId);

	return (NULL != hThread);
}


bool MServerKeeper::CreateAgentPreparePatchWorkThread()
{
	HANDLE  hThread;
	DWORD   dwThreadId;

	hThread = CreateThread(NULL, 0, AgentPreparePatchWorkThread, this, 0, &dwThreadId);

	return (NULL != hThread);
}


bool MServerKeeper::CreateAgentPatchWorkThread()
{
	HANDLE  hThread;
	DWORD   dwThreadId;

	hThread = CreateThread(NULL, 0, AgentPatchWorkThread, this, 0, &dwThreadId);

	return (NULL != hThread);
}

DWORD WINAPI MServerKeeper::AgentDownloadWorkThread( void* pWorkContext )
{
	if( 0 == pWorkContext )
		return -1;

	MServerKeeper* pServerKeeper = reinterpret_cast< MServerKeeper* >( pWorkContext );

	if( GetUpdater.ConnectPatchFileServer() )
	{
		pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_CONNECT_FTP_SERVER, JOB_SUCCESS_CONNECT_FTP_SERVER );

		if( GetUpdater.DownloadAgent() )
		{
			pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_DOWNLOAD_AGENT_PATCH_FILE, JOB_SUCCESS_DOWNLOAD_AGENT_PATCH_FILE );
			pServerKeeper->OnResponseLastJobState();
		}
		else
		{
			pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_DOWNLOAD_AGENT_PATCH_FILE, JOB_FAIL_DOWNLOAD_AGENT_PATCH_FILE );

			if( !GetUpdater.IsEnableAgentPatch() )
				pServerKeeper->GetPatchCheckMap().Check( JOB_DISABLE_AGENT_PATCH, JOB_DISABLE_AGENT_PATCH );

			pServerKeeper->OnResponseLastJobState();
		}
		
		GetUpdater.DisconnectPatchFileServer();
	}
	else
	{
		pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_DOWNLOAD_AGENT_PATCH_FILE, JOB_FAIL_DOWNLOAD_AGENT_PATCH_FILE );
		pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_CONNECT_FTP_SERVER, JOB_FAIL_CONNECT_FTP_SERVER );
		pServerKeeper->OnResponseLastJobState();
	}

	return 0;
}


DWORD WINAPI MServerKeeper::AgentPreparePatchWorkThread( void* pWorkContext )
{
	if( 0 == pWorkContext )
		return -1;

	MServerKeeper* pServerKeeper = reinterpret_cast< MServerKeeper* >( pWorkContext );

	if( GetUpdater.PrepareAgentPatching() )
	{
        pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_PREPARE_AGENT_PATCH, JOB_SUCCESS_PREPARE_AGENT_PATCH );
		pServerKeeper->OnResponseLastJobState();
	}
	else
	{
		pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_PREPARE_AGENT_PATCH, JOB_FAIL_PREPARE_AGENT_PATCH );
		pServerKeeper->OnResponseLastJobState();
	}

	return 0;
}


DWORD WINAPI MServerKeeper::AgentPatchWorkThread( void* pWorkContext )
{
	if( 0 == pWorkContext )
		return -1;

	MServerKeeper* pServerKeeper = reinterpret_cast< MServerKeeper* >( pWorkContext );

	if( pServerKeeper->GetPatchCheckMap().IsAgentPrepareComplete() ) 
	{
		if( GetUpdater.AgentPatching() )
		{
			pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_PATCH_AGENT, JOB_SUCCESS_PATCH_AGENT );
			pServerKeeper->OnResponseLastJobState();
		}
		else
		{
			pServerKeeper->GetPatchCheckMap().Check( JOB_SUCCESS_PATCH_AGENT, JOB_FAIL_PATCH_AGENT );
			pServerKeeper->OnResponseLastJobState();
		}
	}

	return 0;
}


void MServerKeeper::OnNetClear(const MUID& CommUID)
{
	MServer::OnNetClear( CommUID );

	if( CommUID == m_KeeperMgrObj.GetUID() )
		m_KeeperMgrObj.SetConnection( false );
	else if( CommUID == GetMatchServerUID() )
		m_bIsConnectMatchServer = false;
}


int MServerKeeper::OnAccept(MCommObject* pCommObj)
{
	// 할당할 수 있는 UID 공간이 없다.
	MUID AllocUID = UseUID();
	if(AllocUID.IsInvalid()){
		mlog("Communicator has not UID space to allocate your UID.");
		return MERR_COMMUNICATOR_HAS_NOT_UID_SPACE;
	}

	if( GetUpdater.IsConnectableIP(pCommObj->GetIPString()) )
	{
		pCommObj->SetUID(AllocUID);

		LockAcceptWaitQueue();
		m_AcceptWaitQueue.push_back(pCommObj);
		UnlockAcceptWaitQueue();

		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_LOCAL_LOGIN), m_This, m_This);
		pNew->AddParameter(new MCommandParameterUID(pCommObj->GetUID()));
		pNew->AddParameter(new MCommandParameterUID(MUID(0,0)));
		PostSafeQueue(pNew);

		return MOK;
	}
	else
	{
		mlog( "접속할수 없는 IP에러 IP:%s.\n", pCommObj->GetIPString() );
	}

	Disconnect( pCommObj->GetUID() );

	return MERR_CLIENT_MMUG_BLOCKED;
}


void MServerKeeper::InitServerState()
{
	m_ServerState.push_back( SES_ERR_DB );
}


const SERVER_ERR_STATE MServerKeeper::GetServerErrState()
{
	ServerStateVec::iterator it, end;
	end = m_ServerState.end();
	for( it = m_ServerState.begin(); it != end; ++it )
	{
		if( SES_NO != (*it) )
			return (*it);
	}

	return SES_NO;
}


bool MServerKeeper::IsOkServerStart()
{
	// 다시 시작을 하기전에, Agent는 server가 활성화 되었는지 검사후 시작을 하도록 수정해야 함. -by 추교성.
	// 서버가 정상적으로 실행되었는지 검사할수 있는 방법?

	// 프로세스 검사.
	if( FindServer() )
	{
#ifdef _DEBUG
		CONNECTION_STATE nDbgConState = GetConnectionState();
		mlog( "MServerKeeper::IsOkServerStart -> Connection state : %d\n", nDbgConState );
		mlog( "MServerKeeper::IsOkServerStart -> ServerErrState : %d\n", GetServerErrState() );
#endif
		// server에 접속후 상태정보 요청. 
		if( IsConnectMatchServer() )
		{
			if( SES_NO == GetServerErrState() )
			{
				// 여기까지 오면 정상적으로 실했됬다고 생각함.
				// server가 정상적으로 실행. agent를 실행해도 됨.

				return true;

				// 검사가 끝나고 서버와의 접속을 끊어야 하는가?
				// KeeperManager와 연결이 되어있지 않으면 접속을 끊어 벌리까?
			}
		}
		// 이부분 검사 루틴 빨리 정의를 해야함. 2005.07.20 - by 추교성.
		else if( (CNN_NOT_CONNECTED_SERVER == GetConnectionState()) || (CNN_START_CHECK == GetConnectionState()) )
		{
			// 현제 서버에 접속하지 않아도 서버 프로세스가 실행중이면 그냥 실행 할수 있더록 임시로 정함. - by 추교성.(2005.08.18)
			return true;

			// 만약 접속 실패하면 한번만 다시 시도하도록 수정해야 함.
			// ConnectMatchServer();
		}
	}

	return false;
}


void MServerKeeper::RouteToKeeperMgrServerAndAgentState()
{
	if( IsKeeperMgrConnectionValid(m_KeeperMgrObj.GetUID()) )
	{
        OnResponseLastJobState();
		OnResponseServerAndAgentState();

#ifdef _DEBUG
		mlog( "MServerKeeper::RouteToKeeperMgrServerAndAgentState\n" );
#endif
	}
}


void MServerKeeper::StartServer()
{
	if( GetUpdater.StartServerProcess() )
		m_PatchChkMap.Check( JOB_SUCCESS_START_SERVER, JOB_SUCCESS_START_SERVER );
	else
		m_PatchChkMap.Check( JOB_SUCCESS_START_SERVER, JOB_FAIL_START_SERVER );
	
	RouteToKeeperMgrServerAndAgentState();

#ifdef _DEBUG
	mlog( "MServerKeeper::StartServer - RouteToKeeperMgrServerAndAgentState()\n" );
#endif
}


void MServerKeeper::StopServer()
{
	if( GetUpdater.StopServerProcess() )
		m_PatchChkMap.Check( JOB_SUCCESS_STOP_SERVER, JOB_SUCCESS_STOP_SERVER );
	else
		m_PatchChkMap.Check( JOB_SUCCESS_STOP_SERVER, JOB_FAIL_STOP_SERVER );
	
	RouteToKeeperMgrServerAndAgentState();

#ifdef _DEBUG
	mlog( "MServerKeeper::StopServer() - RouteToKeeperMgrServerAndAgentState()\n" );
#endif
}


void MServerKeeper::StartAgent()
{
	if( GetUpdater.StartAgent() )
		GetPatchCheckMap().Check( JOB_SUCCESS_START_AGENT, JOB_SUCCESS_START_AGENT );
	else
		GetPatchCheckMap().Check( JOB_SUCCESS_START_AGENT, JOB_FAIL_START_AGENT );

	RouteToKeeperMgrServerAndAgentState();

#ifdef _DEBUG
	mlog( "MServerKeeper::StartAgent() - RouteToKeeperMgrServerAndAgentState()\n" );
#endif
}


void MServerKeeper::StopAgent()
{
	if( GetUpdater.StopAgent() )
		GetPatchCheckMap().Check( JOB_SUCCESS_STOP_AGENT, JOB_SUCCESS_STOP_AGENT );
	else
		GetPatchCheckMap().Check( JOB_SUCCESS_STOP_AGENT, JOB_FAIL_STOP_AGENT );

	RouteToKeeperMgrServerAndAgentState();

#ifdef _DEBUG
	mlog( "MServerKeeper::StopAgent() - RouteToKeeperMgrServerAndAgentState()\n" );
#endif
}


const string MServerKeeper::GetServerLastMod()
{
	if( m_strServerLastMod.empty() )
		m_strServerLastMod = GetFileLastMod( GetUpdater.GetServerPath() );
	
	return m_strServerLastMod;
}


const string MServerKeeper::GetAgentLastMod()
{
	if( m_strAgentLastMod.empty() )
		m_strAgentLastMod = GetFileLastMod( GetUpdater.GetAgnetPath() );
	
	return m_strAgentLastMod;
}


const string MServerKeeper::GetKeeperLastMod()
{
	if( m_strKeeperLastMod.empty() )
		m_strKeeperLastMod = GetFileLastMod( ".\\ServerKeeper.exe" );

	return m_strKeeperLastMod;
}


const string MServerKeeper::GetFileLastMod( const string& strFile )
{
	string strLastMod( "" );

	CFile file;
	CFileStatus fileStatus;

	if( file.GetStatus(strFile.c_str(), fileStatus) )
	{
		char szServerLastMod[ 128 ] = {0,};
		const CTime Tm = fileStatus.m_mtime;

		_snprintf( szServerLastMod, 127, "%d.%d.%d %d:%d", 
			Tm.GetYear(), Tm.GetMonth(), Tm.GetDay(), Tm.GetHour(), Tm.GetMinute() );

		strLastMod = szServerLastMod;
	}
	else
	{
		// ASSERT( 0 );
		mlog( "MServerKeeper::GetFileLastMod - 파일 Status읽기 실패 : %s.\n", strFile.c_str() );
	}

	return strLastMod;
}


const ULONGLONG MServerKeeper::GetServerFileSize()
{
	return GetFileSize( GetUpdater.GetServerPath() );
}


const ULONGLONG MServerKeeper::GetAgentFileSize()
{
	return GetFileSize( GetUpdater.GetAgnetPath() );
}


const ULONGLONG MServerKeeper::GetKeeperFileSize()
{
	return GetFileSize( ".\\ServerKeeper.exe" );
}


const ULONGLONG MServerKeeper::GetFileSize( const string& strFile )
{
	CFile file;
	CFileStatus fileStatus;

	if( file.GetStatus(strFile.c_str(), fileStatus) )
	{
		return fileStatus.m_size;
	}
	else
	{
		// ASSERT( 0 );
		mlog( "MServerKeeper::GetFileSizeAsString - 파일 Status읽기 실패 : %s.\n", strFile.c_str() );
	}

	return 0;
}


bool MServerKeeper::FindServer()
{
	return GetUpdater.FindServerProcess();
}


bool MServerKeeper::FindAgent()
{
	return GetUpdater.FindAgentProcess();
}


void MServerKeeper::SendCommand( MCommand* pCommand )
{
	if( 0 == pCommand )
		return;

	static unsigned char nSerial = 0;
	nSerial++;
	pCommand->m_nSerialNumber = nSerial;

	MServer::SendCommand( pCommand );
}


void MServerKeeper::Destroy(void)
{
	MServer::Destroy();

	m_CommandManager.Initialize();

//	m_CommandManager.m_CommandDescs.clear();
}