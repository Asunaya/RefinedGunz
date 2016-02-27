// InsertAbuseWordDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "AbuseWord.h"
#include "InsertAbuseWordDlg.h"
#include "AbuseWordDlg.h"
#include "AbuseWordDB.h"
#include ".\insertabuseworddlg.h"


// CInsertAbuseWordDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CInsertAbuseWordDlg, CDialog)
CInsertAbuseWordDlg::CInsertAbuseWordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInsertAbuseWordDlg::IDD, pParent)
{
	// m_edtName.SetWindowText( "" );
}

CInsertAbuseWordDlg::~CInsertAbuseWordDlg()
{
}

void CInsertAbuseWordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_edtName);
}


void CInsertAbuseWordDlg::Reset()
{
	m_edtName.SetWindowText( "" );
	m_edtName.SetFocus();
}


BEGIN_MESSAGE_MAP(CInsertAbuseWordDlg, CDialog)
	ON_BN_CLICKED(ID_INSERT, OnBnClickedInsert)
	ON_EN_SETFOCUS(IDC_EDIT1, OnEnSetfocusEdit)
END_MESSAGE_MAP()


// CInsertAbuseWordDlg 메시지 처리기입니다.

void CInsertAbuseWordDlg::OnBnClickedInsert()
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

			if( 0 == AbuseWordDB.GetAbuseListSize() )
			{
				// DB에서 리스트를 가지고 오지 않을것으로 간주하고, 추가 단어가 중복될수 있을을 알림.
				if( IDYES != AfxMessageBox("리스트가 비어있습니다.\n만약 DB에서 리스트 정보를 가져오지 않았을 경우에는 데이터 유일성을 보장할 수 없습니다.\n계속 진행하시겠습니까?",
					MB_YESNO, 0) )
				{
					return;
				}
			}

			if( AbuseWordDB.ConnectDB() )
			{
				if( AbuseWordDB.InsertAbuseWordDirectDB(string(strAbuseWordName.GetBuffer()), string("NULL"), AW_NEW) )
				{
					AfxMessageBox( "추가 성공. \n리스트를 다시 구성합니다." );
					pAbuseWordDlg->MakeAbuseWordReportCtrlList();
				}
				else
					AfxMessageBox( "추가 실패. \n중복되는 단어가 있음." );
			}
			AbuseWordDB.DisconnectDB();
		}
	}
}

void CInsertAbuseWordDlg::OnEnSetfocusEdit()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	m_edtName.SetWindowText( "" );
}
