// SearchAbuseWord.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "AbuseWord.h"
#include "SearchAbuseWord.h"
#include ".\searchabuseword.h"
#include "AbuseWordDlg.h"
#include "AbuseWordDB.h"


// CSearchAbuseWord 대화 상자입니다.

IMPLEMENT_DYNAMIC(CSearchAbuseWord, CDialog)
CSearchAbuseWord::CSearchAbuseWord(CWnd* pParent /*=NULL*/)
	: CDialog(CSearchAbuseWord::IDD, pParent), m_nLastSearchItemID( -1 )
{
}

CSearchAbuseWord::~CSearchAbuseWord()
{
}


void CSearchAbuseWord::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_edtName);
}


void CSearchAbuseWord::Reset()
{
	m_edtName.SetWindowText( "" );
	m_edtName.SetFocus();
}


BEGIN_MESSAGE_MAP(CSearchAbuseWord, CDialog)
	ON_BN_CLICKED(ID_SEARCH, OnBnClickedSearch)
	ON_EN_SETFOCUS(IDC_EDIT1, OnEnSetfocusEdit)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CSearchAbuseWord 메시지 처리기입니다.

void CSearchAbuseWord::OnBnClickedSearch()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	CWnd* pWnd = theApp.GetMainWnd();
	if( 0 != pWnd )
	{
		CString strAbuseWordName;
		m_edtName.GetWindowText( strAbuseWordName );
		if( 0 != strAbuseWordName.GetLength() )
		{
			CAbuseWordDlg* pAbuseWordDlg = reinterpret_cast< CAbuseWordDlg* >( pWnd );
			CAbuseWordDB& AbuseWordDB	 = pAbuseWordDlg->GetAbuseWordDB();

			const ABUSE_WORD_INFO* pAbuseWordInfo = AbuseWordDB.GetAbuseWordByKeyAbuseWord( string(strAbuseWordName.GetBuffer()) );
			if( 0 != pAbuseWordInfo )
			{
				CReportCtrl& ReportCtrl = pAbuseWordDlg->GetReportCtrl();
				const int nItemID = ReportCtrl.FindPosition( pAbuseWordInfo->m_strKeyAbuseWord );
				if( -1 != nItemID )
				{
					int nTopIndex;
					int nPgCnt = ReportCtrl.GetCountPerPage() / 2;
					while( true )
					{
						nTopIndex = ReportCtrl.GetTopIndex();

						if( ((nItemID > nTopIndex) && ((nItemID - nPgCnt) < nTopIndex)) ||
							(nItemID == nTopIndex) )
						{
							ReportCtrl.SetItemState( m_nLastSearchItemID, ~LVIS_SELECTED, LVIS_SELECTED );
							ReportCtrl.SetItemState( nItemID, LVIS_SELECTED, LVIS_SELECTED );

							m_nLastSearchItemID = nItemID;

							break;
						}
						else if( nItemID > nTopIndex )
						{
							ReportCtrl.ScrollDown();
						}
						else if( nItemID < nTopIndex )
						{
							ReportCtrl.ScrollUp();
						}
						else
						{
							ASSERT( 0 && "검사 조건이 부족함." );
						}
					}
				}
				else
					AfxMessageBox( "탐색 실패.", MB_OK, 0 );
			}
			else
				AfxMessageBox( "해당 단어가 존재하지 않습니다. AbuseWordList와 ReportCtrl이 일치하지 않음.", MB_OK, 0 );
		}
	}
}

void CSearchAbuseWord::OnEnSetfocusEdit()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_edtName.SetWindowText( "" );
}

int CSearchAbuseWord::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.

	return 0;
}
