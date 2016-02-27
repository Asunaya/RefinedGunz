#include "stdafx.h"
#include "keepermanager.h"
#include "ReportCtrl.h"

CKeeperManager::CKeeperManager(void) : m_nPort( 0 )
{
}

CKeeperManager::~CKeeperManager(void)
{
	DeleteKeeperObjs();
}


bool CKeeperManager::LoadKeeperList( const char* pszFileName )
{
	ASSERT( 0 != m_nPort ); 
	ASSERT( 0 != strlen(pszFileName) );

	if( 0 == strlen(pszFileName) ) return false;

	FILE* fp = fopen( pszFileName, "r" );
	if( 0 == fp )
		return false;

	int nID = 0;
	CKeeper* pKeeper = 0;
	char szBuf[ 512 ];
	char szKeeperName[ 256 ];
	char szKeeperAddr[ 16 ];
	
	while( true )
	{
		memset( szBuf, 0, static_cast<int>(sizeof(szBuf)) );
		memset( szKeeperAddr, 0, static_cast<int>(sizeof(szKeeperAddr)) );
		memset( szKeeperName, 0, static_cast<int>(sizeof(szKeeperAddr)) );

		if( 0 == fgets(szBuf, 512, fp) )
			break;

		sscanf( szBuf, "%s %s", szKeeperName, szKeeperAddr );

		// 주소가 없거나 너무 길 경우, 잘못된것으로 취급하고 리스트에 추가하지 않음.
		if( 0 == static_cast<int>(strlen(szKeeperAddr)) ||
			ADD_MAX_LEN < static_cast<int>(strlen(szKeeperAddr)) )
			break;

		if( 0 == pKeeper )
		{
			pKeeper = new CKeeper( nID, m_nPort );
			if( 0 == pKeeper )
				continue;

			++nID;
		}

		if( !pKeeper->SetName(szKeeperName) || !pKeeper->SetAddr(szKeeperAddr) )
		{
			if( 0 != pKeeper )
			{
				delete pKeeper;
				pKeeper = 0;
			}

			continue;
		}

		m_vKeeper.push_back( pKeeper );

		pKeeper = 0;
	}

	fclose( fp );

	return true;
}


bool CKeeperManager::InitKeeperObjs()
{
	bool bResult = true;;
	KeeperVecIter it, end;
	for( it = m_vKeeper.begin(), end = m_vKeeper.end(); it != end; ++it )
	{
		if( !(*it)->Create() )
		{
			bResult = false;
		}

		if( !(*it)->OneTimeInit() )
		{
			bResult = false;
		}
	}

	return bResult;
}


void CKeeperManager::Connect()
{
	bool bResult = true;
	KeeperVecIter it, end;
	for( it = m_vKeeper.begin(), end = m_vKeeper.end(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->Connect();
	}
}


void CKeeperManager::DeleteKeeperObjs()
{
	if( m_vKeeper.empty() )
		return;

	KeeperVecIter it, end;
	for( it = m_vKeeper.begin(), end = m_vKeeper.end(); it != end; ++it )
	{
		// Disconnect의 인자인 UID는 현제 사용하고 있지 않음.
		(*it)->ReleaseAll();
		CKeeper* c = (*it);
		delete (*it);
	}

	m_vKeeper.clear();
}


void CKeeperManager::Disconnect()
{
	if( m_vKeeper.empty() )
		return;

	KeeperVecIter it, end;
	for( it = m_vKeeper.begin(), end = m_vKeeper.end(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
            (*it)->Disconnect( MUID(0, 0) );
	}
}


void CKeeperManager::Release()
{
	DeleteKeeperObjs();
}


void CKeeperManager::Update()
{
	KeeperVecIter it, end;
	for( it = m_vKeeper.begin(), end = m_vKeeper.end(); it != end; ++it )
	{
		(*it)->Update();
	}
}


// Keeper리스트의 모든 Keeper들에게 서버의 현제 상테정보를 요청하게 함.
void CKeeperManager::RequestConnectionState()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		(*it)->RequestConnectionState();
	}
}


void CKeeperManager::RequestStartServer()
{
	KeeperVecIter it,end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestStartServer();
	}
}


void CKeeperManager::RequestStopServer()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestStopServer();
	}
}


void CKeeperManager::RequestKeeperConnectMatchServer()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestKeeperConnectMatchServer();
	}
}


void CKeeperManager::RequestRefreshServerList()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestRefreshServer();
	}
}


void CKeeperManager::RequestPrepareServerPatch()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestPrepareServerPatch();
	}
}


void CKeeperManager::RequestServerPatch()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestServerPatch();
	}
}


void CKeeperManager::RequestDownloadServerPatchFile()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestDownloadServerPatchFile();
	}
}


void CKeeperManager::RequestLastJobState()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		(*it)->RequestLastJobState();
	}
}


void CKeeperManager::RequestStopAgentServer()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestStopAgentServer();
	}
}


void CKeeperManager::RequestStartAgentServer()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestStartAgentServer();
	}
}


bool CKeeperManager::IsCheckedKeeper( const int nKeeperID )
{
	return ( TRUE == GetReport.GetCheck(nKeeperID) );
}

void CKeeperManager::RequestAnnounce( const char* pszAnnounce, const int nLen )
{
	if( (0 == pszAnnounce) || (strlen(pszAnnounce) != nLen) || (255 < strlen(pszAnnounce)) )
		return;

	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestAnnounce( pszAnnounce, nLen );
	}
}

void CKeeperManager::RequestPrepareAgentPatch()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestPrepareAgentPatch();
	}
}


void CKeeperManager::RequestAgentPatch()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestAgentPatch();
	}
}


void CKeeperManager::RequestDownloadAgentPatchFile()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestDownloadAgentPatchFile();
	}
}


void CKeeperManager::RequestResetPatch()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->ReqeustResetPatch();
	}
}


void CKeeperManager::RequestDisconnectServer()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestDisconnectServer();
	}
}


void CKeeperManager::RequestRebootWindows()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestRebootWindows();
	}
}


void CKeeperManager::RequestStopServerWithAnnounce()
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestServerStopWithAnnounce();
	}
}

// 커맨드 생성에 필요한 정보 구성을 위해서.
class CScheduleInfo
{
public :
	CScheduleInfo( const int nType, const int nYear, const int nMonth, 
		const int nDay, const int nHour, const int nMin, const int nCount, 
		const int nCmd, const string& strAnnounce ) :
	m_nType( nType ), m_nYear( nYear ), m_nMonth( nMonth ), m_nDay( nDay ), m_nHour( nHour ),
		m_nMin( nMin ), m_nCount( nCount ), m_nCmd( nCmd ), m_strAnnounce( strAnnounce ) 
	{
	}

	int		m_nType;
	int		m_nYear;
	int		m_nMonth;
	int		m_nDay;
	int		m_nHour;
	int		m_nMin;
	int		m_nCount;
	int		m_nCmd;
	string	m_strAnnounce;

private :
	CScheduleInfo() {}
};


void CKeeperManager::RequestKeeperManagerSchedule( const CScheduleInfo* pScheduleInfo )
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestSchedule( pScheduleInfo );
	}
}


const CKeeper* CKeeperManager::GetKeeperByPos( const int nPos ) const 
{
	ASSERT( nPos < static_cast<int>(m_vKeeper.size()) );
	if( nPos < static_cast<int>(m_vKeeper.size()) )
		return m_vKeeper[ nPos ];
	
	return 0;
}


void CKeeperManager::RequestWriteClientCRC( const DWORD dwClientCRC )
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestWriteClientCRC( dwClientCRC );
	}
}


void CKeeperManager::UpdateSertverStatusReport( CListCtrl* pReport )
{
	if( 0 != pReport )
	{
		pReport->DeleteAllItems();

		int i;
		char szVal[ 256 ] = {0,};
		KeeperVecIter it, end;
		end = m_vKeeper.end();
		for( i = 0, it = m_vKeeper.begin(); it != end; ++it, ++i )
		{
			pReport->SetItemText( i, 0, (*it)->GetName().c_str() );
			pReport->SetItemText( i, 1, (*it)->GetAddr().c_str() );
			pReport->SetItemText( i, 2, (*it)->GetServerResVer().c_str() );
			pReport->SetItemText( i, 3, (*it)->GetServerLastMod().c_str() );

			memset( szVal, 0, sizeof(szVal) );
			_snprintf( szVal, 255, "%u", (*it)->GetServerFileSize() );
			pReport->SetItemText( i, 4, szVal );

			pReport->SetItemText( i, 5, (*it)->GetAgentLastMod().c_str() );

			memset( szVal, 0, sizeof(szVal) );
			_snprintf( szVal, 255, "%u", (*it)->GetAgentFileSize() );
			pReport->SetItemText( i, 6, szVal );
		}
	}
}


void CKeeperManager::RequestReloadServerConfig( const string& strReloadFileList )
{
	KeeperVecIter it, end;
	end = m_vKeeper.end();
	for( it = m_vKeeper.begin(); it != end; ++it )
	{
		if( IsCheckedKeeper((*it)->GetID()) )
			(*it)->RequestReloadServerConfig( strReloadFileList );
	}
}


void CKeeperManager::RequestAddHashMap( const string& strNewHashValue )
{
	if( !strNewHashValue.empty() )
	{
		KeeperVecIter it, end;
		end = m_vKeeper.end();
		for( it = m_vKeeper.begin(); it != end; ++it )
		{
			if( IsCheckedKeeper((*it)->GetID()) )
				(*it)->RequestAddHashMap( strNewHashValue );
		}
	}
}