// MServerInfoDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "PatchInterface.h"
#include "MServerInfoDlg.h"
#include "KeeperManager.h"
#include ".\mserverinfodlg.h"


// MServerInfoDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(MServerInfoDlg, CDialog)
MServerInfoDlg::MServerInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(MServerInfoDlg::IDD, pParent)
{
}

MServerInfoDlg::~MServerInfoDlg()
{
}


void MServerInfoDlg::Init()
{
	m_ServerInfo.SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	InitColumnName();
}


void MServerInfoDlg::InitColumnName()
{
	m_vColumnName.push_back( string("서버이름") );
	m_vColumnName.push_back( string("IP") );
	m_vColumnName.push_back( string("S-릴리즈버전") );
	m_vColumnName.push_back( string("S-수정일") );
	m_vColumnName.push_back( string("S-파일크기") );
	m_vColumnName.push_back( string("A-수정일") );
	m_vColumnName.push_back( string("A-파일크기") );
	m_vColumnName.push_back( string("A-Count") );

	const int nSize = static_cast< int >( m_vColumnName.size() );

	for( int i = 0; i < nSize; ++i )
		m_ServerInfo.InsertColumn( i, m_vColumnName[i].c_str(), LVCFMT_LEFT, 90 );
}


void MServerInfoDlg::Reset()
{
	m_ServerInfo.DeleteAllItems();

	char szVal[ 7 ] = {0,};

	const int nKeeperCnt = GetKeeperMgr.GetSize();
	for( int i = 0; i < nKeeperCnt; ++i )
	{
		const CKeeper* pKeeper = GetKeeperMgr.GetKeeperByPos( i );
		if( (0 != pKeeper) ) // && pKeeper->IsConnectedKeeper() )
		{
			// 여기서 서버의 정보를 출력함.
			// 출력할 정보는 서버버젼, 서버파일과 에이전트 파일의 마지막 수저날자.

			pKeeper->GetServerResVer();
			pKeeper->GetServerLastMod();
			pKeeper->GetAgentLastMod();
			pKeeper->GetKeeperLastMod();

			m_ServerInfo.InsertItem( i, pKeeper->GetName().c_str() );

			m_ServerInfo.SetItemText( i, 0, pKeeper->GetName().c_str() );
			m_ServerInfo.SetItemText( i, 1, pKeeper->GetAddr().c_str() );

			if( 0 < pKeeper->GetServerResVer().length() )
				m_ServerInfo.SetItemText( i, 2, pKeeper->GetServerResVer().c_str() );
			else
				m_ServerInfo.SetItemText( i, 2, "NO" );

			if( 0 < pKeeper->GetServerLastMod().length() )
				m_ServerInfo.SetItemText( i, 3, pKeeper->GetServerLastMod().c_str() );
			else
				m_ServerInfo.SetItemText( i, 3, "NO" );

			if( 0 != pKeeper->GetServerFileSize() )
			{
				memset( szVal, 0, sizeof(szVal) );
				_snprintf( szVal, sizeof(szVal) - 1, "%.2fMB", 
					static_cast<float>(pKeeper->GetServerFileSize()) / 1048576.0f );
				m_ServerInfo.SetItemText( i, 4, szVal );
			}
			else
				m_ServerInfo.SetItemText( i, 4, "NO" );

			if( 0 < pKeeper->GetAgentLastMod().length() )
				m_ServerInfo.SetItemText( i, 5, pKeeper->GetAgentLastMod().c_str() );
			else 
				m_ServerInfo.SetItemText( i, 5, "NO" );

			if( 0 != pKeeper->GetAgentFileSize() )
			{
				memset( szVal, 0, sizeof(szVal) );
				_snprintf( szVal, sizeof(szVal) - 1, "%.2fMB", 
					static_cast<float>(pKeeper->GetAgentFileSize()) / 1048576.0f );
				m_ServerInfo.SetItemText( i, 6, szVal );
			}
			else
				m_ServerInfo.SetItemText( i, 6, "NO" );

			memset( szVal, 0, sizeof(szVal) );
			_snprintf( szVal, sizeof(szVal) - 1, "%d", static_cast<int>(pKeeper->GetAgentCount()) );
			m_ServerInfo.SetItemText( i, 7, szVal );
		}
	}
}


void MServerInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INFO_LIST, m_ServerInfo);
}


BEGIN_MESSAGE_MAP(MServerInfoDlg, CDialog)
END_MESSAGE_MAP()


// MServerInfoDlg 메시지 처리기입니다.

