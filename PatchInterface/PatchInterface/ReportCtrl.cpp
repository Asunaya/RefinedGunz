// ReportCtrl.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "PatchInterface.h"
#include "ReportCtrl.h"
#include ".\reportctrl.h"
#include "Keeper.h"
#include "KeeperManager.h"
#include "commctrl.h"

// #pragma comment( lib, "Comclt32.dll" )



// CReportCtrl

IMPLEMENT_DYNAMIC(CReportCtrl, CListCtrl)
CReportCtrl::CReportCtrl()
{
}

CReportCtrl::~CReportCtrl()
{
}

void CReportCtrl::InsertColumns()
{
	int nColumnCount = GetHeaderCtrl()->GetItemCount();
	for( int j = 0; j < nColumnCount; ++j )
		DeleteColumn( 0 );

	LV_COLUMN lvc;
	
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for( int i = 0; i<NUM_COLUMNS; i++ )
	{
		lvc.iSubItem	= -1;
		lvc.pszText		= const_cast<char*>( COLUMN_NAME[i] );
		lvc.cx			= COLUMN_LENTH[ i ];
		lvc.fmt			= LVCFMT_CENTER;

		InsertColumn( i, &lvc );
	}
}


void CReportCtrl::Update( const int nID )
{
	// return ;

	KeeperVec& kv = GetKeeperMgr.GetKeeperObjs();
	if( kv.empty() )
		return;

	char* pszKeeperName;
	char* pszIP;

	string& strName = kv[nID]->GetName();
	string& strIP = kv[nID]->GetAddr();

	pszKeeperName = const_cast<char*>( strName.c_str() );
	pszIP = const_cast<char*>( strIP.c_str() );

	int a = kv[nID]->GetConnectionState();
	int b = kv[nID]->GetLastJobState();

	SetItemText( nID, 0, pszKeeperName );
	SetItemText( nID, 1, pszIP );
	SetItemText( nID, 2, RUN_STATE_LIST[kv[nID]->GetServerRunState()] );
	SetItemText( nID, 3, RUN_STATE_LIST[kv[nID]->GetAgentRunState()] );
	SetItemText( nID, 4, CONNECTION_STATE_LIST[kv[nID]->GetConnectionState()] );	
	SetItemText( nID, 5, SERVER_ERR_STATE_LIST[kv[nID]->GetServerErrState()] );
	SetItemText( nID, 6, JOB_STATE_LIST[kv[nID]->GetLastJobState()] );
	SetItemText( nID, COLUMN_SERVER_START, CONFIG_STATE_LIST[kv[nID]->GetConfigState(COLUMN_SERVER_START)]);
	SetItemText( nID, COLUMN_AGENT_START, CONFIG_STATE_LIST[kv[nID]->GetConfigState(COLUMN_AGENT_START)]);
	SetItemText( nID, COLUMN_DOWNLOAD, CONFIG_STATE_LIST[kv[nID]->GetConfigState(COLUMN_DOWNLOAD)]);
	SetItemText( nID, COLUMN_PREPARE, CONFIG_STATE_LIST[kv[nID]->GetConfigState(COLUMN_PREPARE)]);
	SetItemText( nID, COLUMN_PATCH, CONFIG_STATE_LIST[kv[nID]->GetConfigState(COLUMN_PATCH)]);
	
	// CListCtrl::Update( nID );
}


BEGIN_MESSAGE_MAP(CReportCtrl, CListCtrl)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// CReportCtrl 메시지 처리기입니다.


void CReportCtrl::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{


	// TODO:  지정된 항목을 그리는 코드를 추가합니다.
}


BOOL CReportCtrl::Create( CWnd* pParentWnd, UINT nID )
{
	RECT rt;
	int	 nColumnLenSum = 0;

	for( int i = 0; i < NUM_COLUMNS; ++i )
		nColumnLenSum += COLUMN_LENTH[ i ];

	nColumnLenSum += 20;

	SetRect( &rt, 0, 0, nColumnLenSum, 290 );

	return CListCtrl::Create( REPORT_WND_STYLE , rt, pParentWnd, nID );
}


void CReportCtrl::SetGridLines( bool bSet )
{
	DWORD dwStyle = GetExtendedStyle();
	if( bSet )
		dwStyle |= LVS_EX_GRIDLINES;
	else
		dwStyle &= ~LVS_EX_GRIDLINES;

	SetExtendedStyle( bSet );
}


void CReportCtrl::SetCheckboxStyle( int nStyle )
{
	DWORD dwStyle = GetExtendedStyle();
	if( 0 == nStyle )
		dwStyle &= ~LVS_EX_CHECKBOXES;
	else
	{
		dwStyle |= LVS_EX_CHECKBOXES;
	}

	SetExtendedStyle(dwStyle);
}


void CReportCtrl::SetFullrowSelectStyle( bool bSet )
{
	DWORD dwStyle = GetExtendedStyle();
	if( bSet )
		dwStyle |= LVS_EX_FULLROWSELECT;
	else
		dwStyle &= ~LVS_EX_FULLROWSELECT;

	SetExtendedStyle( dwStyle );
}


void CReportCtrl::InsertItems()
{
	KeeperVec& kv = GetKeeperMgr.GetKeeperObjs();
	if( kv.empty() )
		return;

	DeleteAllItems();

	int i;
	KeeperVecIter it, end;
	LVITEM lvi;
	
	for( i = 0, it = kv.begin(), end = kv.end(); it != end; ++it, ++i )
	{
		string& strName = (*it)->GetName();
		string& strIP = (*it)->GetAddr();

		char* pszKeeperName = const_cast<char*>( strName.c_str() );
		char* pszIP			= const_cast< char* >( strIP.c_str() );

		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = pszKeeperName;
		lvi.state = INDEXTOSTATEIMAGEMASK(1);
		lvi.stateMask = LVIS_SELECTED;
		lvi.iImage = 0;
		lvi.lParam = 0;

		InsertItem( &lvi );

		SetItemText( i, 0, pszKeeperName );
		SetItemText( i, 1, pszIP );
		SetItemText( i, 2, RUN_STATE_LIST[(*it)->GetServerRunState()] );
		SetItemText( i, 3, RUN_STATE_LIST[(*it)->GetAgentRunState()] );
		SetItemText( i, 4, CONNECTION_STATE_LIST[(*it)->GetConnectionState()] );	
		SetItemText( i, 5, SERVER_ERR_STATE_LIST[(*it)->GetServerErrState()] );
		SetItemText( i, 6, JOB_STATE_LIST[(*it)->GetLastJobState()] );
		SetItemText( i, COLUMN_SERVER_START, CONFIG_STATE_LIST[(*it)->GetConfigState(COLUMN_SERVER_START)]);
		SetItemText( i, COLUMN_AGENT_START, CONFIG_STATE_LIST[(*it)->GetConfigState(COLUMN_AGENT_START)]);
		SetItemText( i, COLUMN_DOWNLOAD, CONFIG_STATE_LIST[(*it)->GetConfigState(COLUMN_DOWNLOAD)]);
		SetItemText( i, COLUMN_PREPARE, CONFIG_STATE_LIST[(*it)->GetConfigState(COLUMN_PREPARE)]);
		SetItemText( i, COLUMN_PATCH, CONFIG_STATE_LIST[(*it)->GetConfigState(COLUMN_PATCH)]);

		// SetCheck( i );
	}
}
void CReportCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CListCtrl::OnLButtonDown(nFlags, point);

	const int nItemID = FindSelectedItem();
	if( -1 != nItemID )
	{
		int nColumnID = FindSelectedColumn( point );
		if( (-1 != nColumnID) && GetCheck(nItemID) ) 
			ConvertSelectedColumnConfigState( nItemID, nColumnID );
	}
}


int CReportCtrl::FindSelectedColumn( const CPoint Point )
{
	KeeperVec& kv = GetKeeperMgr.GetKeeperObjs();
	if( kv.empty() )
		return -1;

	int nColumnSum = 0;
	for( int i = 0; i < NUM_COLUMNS; ++i )
	{
		nColumnSum += GetColumnWidth( i );
		if( nColumnSum > Point.x )
			return i;
	}

	return -1;
}


int CReportCtrl::FindSelectedItem()
{
	bool isFocus = ( GetFocus() == this );

	size_t nItemCount = GetKeeperMgr.GetKeeperObjs().size();
	LVITEM lvi;
	char szTemp[ 1024 ];
	bool isSelected;

	for( size_t i = 0; i < nItemCount; ++i )
	{
		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
		lvi.iItem = static_cast<int>(i);
		lvi.iSubItem = 0;
		lvi.pszText = szTemp;
		lvi.cchTextMax = 1024;
		lvi.stateMask = 0xFFFF;

		GetItem( &lvi );

		isSelected = (isFocus || (GetStyle() & LVS_SHOWSELALWAYS)) && lvi.state & LVIS_SELECTED;
		isSelected	= isSelected || (lvi.state & LVIS_DROPHILITED);

		if( isSelected )
			return static_cast<int>(i);
	}

	return -1;
}


void CReportCtrl::ConvertSelectedColumnConfigState( const int nItemID, const int nColumnID )
{
	KeeperVec kv = GetKeeperMgr.GetKeeperObjs();
	if( kv[nItemID]->IsConnectedKeeper() && (nItemID < static_cast<int>(kv.size())) && (nColumnID < NUM_COLUMNS) )
	{
		switch( nColumnID )
		{
		case COLUMN_SERVER_START :
			{
				kv[ nItemID ]->InvertServerConfigState();
			}
			break;

		case COLUMN_AGENT_START :
			{
				kv[ nItemID ]->InvertAgentConfigState();
			}
			break;

		case COLUMN_DOWNLOAD :
			{
				kv[ nItemID ]->InvertDownloadConfigState();
			}
			break;

		case COLUMN_PREPARE :
			{
				kv[ nItemID ]->InvertPrepareConfigState();
			}
			break;

		case COLUMN_PATCH :
			{
				kv[ nItemID ]->InvertPatchConfigState();
			}
			break;

		default :
			{
			}
			break;
		}
	}
}