// ReloadServerConfigDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "PatchInterface.h"
#include "ReloadServerConfigDlg.h"
#include "KeeperManager.h"
#include ".\reloadserverconfigdlg.h"


// ReloadServerConfigDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(ReloadServerConfigDlg, CDialog)
ReloadServerConfigDlg::ReloadServerConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ReloadServerConfigDlg::IDD, pParent)
{
}

ReloadServerConfigDlg::~ReloadServerConfigDlg()
{
}

void ReloadServerConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	// DDX_Control(pDX, IDC_RELOAD_LIST, m_ReloadList);
	DDX_Control(pDX, IDC_NEWHASHVALUE, m_NewHashValue);
}


bool ReloadServerConfigDlg::LoadReloadList()
{
	FILE* fp = fopen( RELOAD_LIST, "r" );
	if( 0 == fp )
		return false;

	m_ReloadFileList.clear();

	char szLine[ 256 ]	= {0,};
	char szFile[ 64 ]	= {0,};

	while( 0 != fgets(szLine, 255, fp) )
	{
		sscanf( szLine, "%s", szFile );

		m_ReloadFileList.push_back( string(szFile) );

		memset( szLine, 0, 256 );
		memset( szFile, 0, 64 );
	}

	fclose( fp );

	return true;
}


bool ReloadServerConfigDlg::InitReport()
{
	m_ReloadList.InsertColumn( 0, "Reload file name.", LVCFMT_LEFT, 300 );

	int i = 0;
	ReloadListVec::iterator it, end;
	end = m_ReloadFileList.end();
	for( it = m_ReloadFileList.begin(); it != end; ++it, ++i )
	{
		m_ReloadList.InsertItem(i, (*it).c_str() );
	}

	return true;
}



BEGIN_MESSAGE_MAP(ReloadServerConfigDlg, CDialog)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(ID_RELOAD, OnBnClickedReload)
	ON_BN_CLICKED(IDC_ADDHASHMAP, OnBnClickedAddhashmap)
END_MESSAGE_MAP()


// ReloadServerConfigDlg 메시지 처리기입니다.

int ReloadServerConfigDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.

	if( !LoadReloadList() )
		return -1;

	RECT rt;
	SetRect( &rt, 10, 10, 310, 300 );
	m_ReloadList.Create( WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS, rt, this, 0 ); //IDC_RELOAD_LIST );

	m_ReloadList.SetExtendedStyle( /*LVS_EX_BORDERSELECT |*/ LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	if( !InitReport() )
		return -1;

	m_ReloadList.ShowWindow( SW_SHOW );

	return 0;
}

void ReloadServerConfigDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// OnCancel();
	ShowWindow( SW_HIDE );
}

void ReloadServerConfigDlg::OnBnClickedReload()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	// Check list만 요청함.

	char szText[ 64 ];
	ReloadListVec SelectedReloadFileList;
	const int nItemCount = m_ReloadList.GetItemCount();
	for( int i = 0; i < nItemCount; ++i )
	{
		if( m_ReloadList.GetCheck(i) )
		{
			m_ReloadList.GetItemText(i, 0, szText, 64 );
			SelectedReloadFileList.push_back( szText );
		}
	}

	string strFileList;
	ReloadListVec::iterator it, end;
	end = SelectedReloadFileList.end();
	for( it = SelectedReloadFileList.begin(); it != end; ++it )
	{
		if( !strFileList.empty() )
			 strFileList += string(",");
		strFileList += (*it);
	}

	GetKeeperMgr.RequestReloadServerConfig( strFileList );
}

void ReloadServerConfigDlg::OnBnClickedAddhashmap()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	CString strNewHashValue;
	m_NewHashValue.GetWindowText( strNewHashValue );

	if( '@' == strNewHashValue[0] )
	{
		if( IDYES  != AfxMessageBox("'@'마크로인해 이전에 저장되어있는 모든 hashcode는 지워지고 입력한 값이 새로 추가됩니다. 진행하시겠습니까?",
			MB_YESNO, 0) )
		{
			return;
		}
	}

	GetKeeperMgr.RequestAddHashMap( strNewHashValue.GetBuffer() );
}
