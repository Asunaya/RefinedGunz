// QuestItemList.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "DBQuestItemEditor.h"
#include ".\questitemlist.h"
#include "DBQuestItemEditorDlg.h"
#include "MDebug.h"



// CQuestItemList

IMPLEMENT_DYNAMIC(CQuestItemList, CListCtrl)
CQuestItemList::CQuestItemList()
{
}

CQuestItemList::~CQuestItemList()
{
	m_MCharInfo.m_QuestItemList.Clear();
}


void CQuestItemList::InitListColumn()
{
	InsertColumn( 0, _T("QIID"), LVCFMT_LEFT, 70 );
	InsertColumn( 1, _T("Quest item name"), LVCFMT_LEFT, 230 );
	InsertColumn( 2, _T("Count"), LVCFMT_LEFT, 50 );
}


void CQuestItemList::MakeQItemInfoColumns()
{
	if( !m_MCharInfo.m_QuestItemList.empty() )
	{
		DeleteAllItems();

		int iItem;
		char szTmp[ 256 ];
		LV_ITEM	lvi;

		MQuestItemMap::iterator it, end;

		it = m_MCharInfo.m_QuestItemList.begin();
		end = m_MCharInfo.m_QuestItemList.end();

		for( iItem = 0; it != end; ++it, ++iItem )
		{
			sprintf( szTmp, "%d", it->second->GetItemID() );

			lvi.mask = LVIF_TEXT | LVIF_IMAGE;
			lvi.iItem = iItem;
			lvi.iSubItem = 0;
			lvi.pszText = szTmp;
			lvi.iImage = 0;
			lvi.stateMask = LVIS_STATEIMAGEMASK;
			lvi.state = 0; // INDEXTOSTATEIMAGEMASK( 0 );

			InsertItem( &lvi );
		}

		it = m_MCharInfo.m_QuestItemList.begin();
		for( iItem = 0; it != end; ++it, ++iItem )
		{
			sprintf( szTmp, "%d", it->second->GetItemID() );

			SetItemText( iItem, 0, szTmp );
			SetItemText( iItem, 1, it->second->GetDesc()->m_szQuestItemName );

			sprintf( szTmp, "%d", it->second->GetCount() );
			SetItemText( iItem, 2, szTmp );
		}
	}	
}


bool CQuestItemList::GetDBQItemInfo( const string strCharName )
{
	if( strCharName.empty() || (32 < strCharName.size()) )
		return false;

	int nCID;

	ConnectDB();
	m_DBMgr.GetCID( strCharName.c_str(), nCID );
	DisconnectDB();

    m_MCharInfo.m_nCID = nCID;
	strncpy( m_MCharInfo.m_szName, strCharName.c_str(), strCharName.size() );

	return GetDBQItemInfo( nCID );
}


bool CQuestItemList::GetDBQItemInfo( const int nCID )
{
	if( 0 > nCID )
		return false;

	if( GetQuestItemDescMgr().empty() )
		return false;

	string strCharName;

	m_MCharInfo.m_nCID = nCID;
	m_MCharInfo.m_QuestItemList.Clear();

	ConnectDB();

	m_DBMgr.GetCharName( nCID, strCharName );

	strncpy( m_MCharInfo.m_szName, strCharName.c_str(), strCharName.size() );

	if( !m_DBMgr.GetCharQuestItemInfo(&m_MCharInfo) )
	{
		AfxMessageBox( "퀘스트 아이템 정보를 가져오기 실패.\n" );
		DisconnectDB();
		return false;
	}

	DisconnectDB();
	return true;
}

bool CQuestItemList::Init()
{
	return true;
}


bool CQuestItemList::ConnectDB()
{
	char szDNS[ 256 ]		= {0};
	char szUserName[ 256 ]	= {0};
	char szPassword[ 256 ]	= {0};

	char szCurrDir[_MAX_DIR] = {0};
	GetCurrentDirectory(_MAX_DIR, szCurrDir);
	sprintf(szCurrDir, "%s\\%s", szCurrDir, CONFIG_FILE);

	int nResult = 0;
	nResult = GetPrivateProfileString( "DB", "DNS", "testdb", szDNS, 255, szCurrDir );
	nResult = GetPrivateProfileString( "DB", "USERNAME", "sa", szUserName, 255, szCurrDir );
	nResult = GetPrivateProfileString( "DB", "PASSWORD", "tnseornr", szPassword, 255, szCurrDir );

	string strConnInfo = m_DBMgr.BuildDSNString( szDNS, szUserName, szPassword );

	if( !m_DBMgr.Connect(strConnInfo.c_str()) )
	{
		AfxMessageBox( "디비 접속 실패.\n" );
		return false;
	}

	return true;
}


bool CQuestItemList::DisconnectDB()
{
	m_DBMgr.Disconnect();
	return true;
}


bool CQuestItemList::LoadQuestItemXML()
{
	if( !GetQuestItemDescMgr().ReadXml(QUEST_ITEM_XML_FILE) )
		return false;

	return true;
}


void CQuestItemList::EditQItem( const long nQIID, const int nCount )
{
	if( 0 == m_MCharInfo.m_nCID )
	{
		AfxMessageBox( "캐릭터를 선택해 주세요.", 0, 0 );
		return;
	}

	MQuestItem* pQItem = m_MCharInfo.m_QuestItemList.Find( nQIID );
	if( 0 != pQItem )
	{
		pQItem->SetCount( nCount + MIN_QUEST_ITEM_COUNT );

		QItem HistoryQItem( pQItem->GetItemID(), 
			pQItem->GetDesc()->m_szQuestItemName, 
			pQItem->GetCount() );

		m_QItemEditHistory.push_back( HistoryQItem );
	}
	else
	{
		// 새로 추가해 줌.
		if( IDOK == AfxMessageBox("아이템이 새로 추가하시겠습니까?", MB_OKCANCEL, 0) )
		{
			// 새로 추가시켜 줌.
			if( m_MCharInfo.m_QuestItemList.CreateQuestItem(nQIID, nCount + MIN_QUEST_ITEM_COUNT) )
			{
				QItem HistoryQItem( nQIID, 
									GetQuestItemDescMgr().FindQItemDesc(nQIID)->m_szQuestItemName, 
									nCount );
				m_QItemEditHistory.push_back( HistoryQItem );
			}
			else
			{
				AfxMessageBox( "새로운 아이템 추가 실패.", 0, 0 );
			}
		}
	}
}


const int CQuestItemList::FindCIDByCharName( const string& strCharName )
{
	if( strCharName.empty() )
		return -1;

	int nCID;

	ConnectDB();
	if( !m_DBMgr.GetCID(strCharName.c_str(), nCID) )
		return -1;
	DisconnectDB();

	return nCID;
}


const string CQuestItemList::FindCharNameByCID( const int nCID )
{
	if( 0 > nCID )
		return string( "" );

	string strCharName;
	ConnectDB();
	if( !m_DBMgr.GetCharName(nCID, strCharName) )
		return string( "" );
	DisconnectDB();

	return strCharName;
}	


void CQuestItemList::UpdateDB()
{
	ConnectDB();

	m_DBMgr.UpdateQuestItem( m_MCharInfo.m_nCID, 
		m_MCharInfo.m_QuestItemList, 
		m_MCharInfo.m_QMonsterBible );

	DisconnectDB();
}


BEGIN_MESSAGE_MAP(CQuestItemList, CListCtrl)
	ON_WM_CREATE()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()



// CQuestItemList 메시지 처리기입니다.


BOOL CQuestItemList::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	dwStyle &= ~MUST_NOT_STYLE;
	dwStyle |= MUST_STYLE;

	return CListCtrl::Create(dwStyle, rect, pParentWnd, nID);
}

BOOL CQuestItemList::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	cs.style &= ~MUST_NOT_STYLE;
	cs.style |= MUST_STYLE;
	return CListCtrl::PreCreateWindow(cs);
}

int CQuestItemList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	lpCreateStruct->style &= ~MUST_NOT_STYLE;
	lpCreateStruct->style |= MUST_STYLE;
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	SetExtendedStyle(GetExtendedStyle());
	ASSERT(GetHeaderCtrl() != NULL);

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.

	return 0;
}

void CQuestItemList::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	if( !m_MCharInfo.m_QuestItemList.empty() )
	{
		int						iItem;
		char					szTmp[ 256 ];
		LV_ITEM					lvi;
		bool					isFocus;
		bool					isSelected;
		MQuestItemMap::iterator it, end;

		isFocus = ( GetFocus() == this );

		it = m_MCharInfo.m_QuestItemList.begin();
		end = m_MCharInfo.m_QuestItemList.end();

		for( iItem = 0; it != end; ++it, ++iItem )
		{
			lvi.mask		= LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
			lvi.iItem		= iItem;
			lvi.iSubItem	= 0;
			lvi.pszText		= szTmp;
			lvi.cchTextMax	= 256;
			lvi.stateMask	= 0xFFFF;
			bool bb = GetItem(&lvi);

			isSelected	= (isFocus || (GetStyle() & LVS_SHOWSELALWAYS)) && lvi.state & LVIS_SELECTED;
			isSelected	= isSelected || (lvi.state & LVIS_DROPHILITED);

			if( isSelected )
			{
				CDBQuestItemEditorDlg* pDlg = reinterpret_cast< CDBQuestItemEditorDlg* >( GetParent() );
				pDlg->SetChoiceQIID( szTmp );
				pDlg->SetFocusToCount();
			}
		}
	}	

	CListCtrl::OnLButtonDblClk(nFlags, point);
}


void CQuestItemList::Clear()
{
	m_MCharInfo.m_QuestItemList.Clear();
	m_MCharInfo.m_QMonsterBible.Clear();
	DeleteAllItems();
}