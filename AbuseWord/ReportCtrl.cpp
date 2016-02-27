// ReportCtrl.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "AbuseWord.h"
#include "ReportCtrl.h"
#include ".\reportctrl.h"
#include "AbuseWordDlg.h"


// CReportCtrl

IMPLEMENT_DYNAMIC(CReportCtrl, CListCtrl)
CReportCtrl::CReportCtrl()
{
}

CReportCtrl::~CReportCtrl()
{
}


const int CReportCtrl::FindSelectItem()
{
	bool isFocus = ( GetFocus() == this );

	LVITEM lvi;
	const int nItemCount = GetItemCount();
	bool isSelected;

	for( int i = 0; i < nItemCount; ++i )
	{
		lvi.mask = LVIF_STATE;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = 0;
		lvi.cchTextMax = 0;
		lvi.stateMask = 0xFFFF;

		GetItem( &lvi );

		isSelected = (isFocus || (GetStyle() & LVS_SHOWSELALWAYS)) && lvi.state & LVIS_SELECTED;
		isSelected	= isSelected || (lvi.state & LVIS_DROPHILITED);

		if( isSelected )
			return i;
	}

	return -1;
}


const int CReportCtrl::FindChageCheckState()
{
	CWnd* pWnd = theApp.GetMainWnd();
	if( 0 != pWnd )
	{
		CAbuseWordDlg* pDlgWnd		= reinterpret_cast< CAbuseWordDlg* >( pWnd );
		CAbuseWordDB& AbuseWordDB	= pDlgWnd->GetAbuseWordDB();

		LVITEM lvi;
		const int nItemCount = GetItemCount();
		char szItemName[ 1024 ];

		for( int i = 0; i < nItemCount; ++i )
		{
			lvi.mask = LVIF_TEXT;
			lvi.iItem = i;
			lvi.iSubItem = 0;
			lvi.pszText = szItemName;
			lvi.cchTextMax = 1023;

			GetItem( &lvi );

			ASSERT( (0 != strlen(szItemName)) && "CReportCtrl::FindChageCheckState -> 문자 길이가 0이되면 않됨." );

			const ABUSE_WORD_INFO* pAbuseWord = AbuseWordDB.GetAbuseWordByKeyAbuseWord( string(szItemName) );
			if( 0 == pAbuseWord )
			{
				ASSERT( (0 != pAbuseWord) && "CReportCtrl::FindChageCheckState -> 단어를 찾지 못함." );
				break;
			}

			if( GetCheck(i) )
			{
				if( (AW_NEW == pAbuseWord->m_nCheckState) || (AW_DEL == pAbuseWord->m_nCheckState) )
					return i;
			}
			else
			{
				if( (AW_NEW != pAbuseWord->m_nCheckState) && (AW_DEL != pAbuseWord->m_nCheckState) )
					return i;
				// 
			}
		}
	}

	return -1;
}


const int CReportCtrl::FindPosition( const string& strAbuseWord )
{
	if( !strAbuseWord.empty() )
	{
		LVITEM lvi;
		char szItemName[ 1024 ];

		const int nListCount = GetItemCount();
		for( int i = 0; i < nListCount; ++i )
		{
			lvi.mask = LVIF_TEXT;
			lvi.iItem = i;
			lvi.iSubItem = 0;
			lvi.pszText = szItemName;
			lvi.cchTextMax = 1023;

			GetItem( &lvi );

			ASSERT( (0 != strlen(szItemName)) && "CReportCtrl::FindPosition -> 문자 길이가 0이되면 않됨." );

			if( 0 == strncmp(strAbuseWord.c_str(), szItemName, strAbuseWord.length() > strlen(szItemName) ? strAbuseWord.length() : strlen(szItemName)) )
				return i;	
		}
	}

	return -1;
}


void CReportCtrl::ScrollUp( const int nPos )
{
	if( 0 < nPos ) 
		return;

	CSize sz;
	sz.SetSize( 0, nPos );

	Scroll( sz );
}


void CReportCtrl::ScrollDown( const int nPos )
{
	if( 0 > nPos )
		return;

	CSize sz;
	sz.SetSize( 0, nPos );

	Scroll( sz );
}


const string CReportCtrl::GetSelectedItemText()
{
	char szItemText[ 512 ];

	GetItemText( FindSelectItem(), 0, szItemText, 511 );

	return string( szItemText );
}


BEGIN_MESSAGE_MAP(CReportCtrl, CListCtrl)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// CReportCtrl 메시지 처리기입니다.


void CReportCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CListCtrl::OnLButtonDown(nFlags, point);

	if( 0 == GetItemCount() ) return;

	// Check박스를 클릭했을때만 작동해야 함.
	if( -1 == FindSelectItem() )
	{
		const int nItemID = FindChageCheckState();
		ASSERT( (-1 != nItemID) && "CReportCtrl::OnLButtonDown -> 유효하지 않은 아이템 ID임" );
		char szItemName[ 1024 ] = {0};
		LVITEM lvi;

		lvi.mask = LVIF_TEXT;
		lvi.iItem = nItemID;
		lvi.iSubItem = 0;
		lvi.pszText = szItemName;
		lvi.cchTextMax = 1024;

		GetItem( &lvi );

		CWnd* pWnd = theApp.GetMainWnd();
		if( 0 != pWnd )
		{
			CAbuseWordDlg* pDlgWnd		= reinterpret_cast< CAbuseWordDlg* >( pWnd );
			CAbuseWordDB& AbuseWordDB	= pDlgWnd->GetAbuseWordDB();

			const ABUSE_WORD_INFO* pAbuseWord = AbuseWordDB.GetAbuseWordByKeyAbuseWord( string(szItemName) );
			if( 0 != pAbuseWord )
			{
				if( AW_NEW == pAbuseWord->m_nCheckState )
					AbuseWordDB.Check( string(szItemName), AW_ADD );
				else if( AW_USED == pAbuseWord->m_nCheckState )
					AbuseWordDB.Check( string(szItemName), AW_DEL ); // 제거되는 단어표시로 배경색을 바뀌게 하고 싶음 흑...ㅠ.ㅠ
				else if( AW_DEL == pAbuseWord->m_nCheckState )
					AbuseWordDB.Check( string(szItemName), AW_USED );
				else
					AbuseWordDB.Check( string(szItemName), AW_NEW );
			}
			else
			{
				ASSERT( 0 );
			}
		}
	}
}
