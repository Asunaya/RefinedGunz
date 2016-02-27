/*
 CNewAppPopupDlg 클래스(NewAppPopupDlg.cpp)

  desc : Popup Dialog 관련 클래스
  date : 2004년 5월 30일
  comp : 임동환
*/


#include <stdafx.h>
#include "NewAppPopupDlg.h"


/************************************************************************************
  CNewAppPopupDlg dialog
*************************************************************************************/
// CNewAppPopupDlg
CNewAppPopupDlg::CNewAppPopupDlg(CWnd* pParent /*=NULL*/)
	: CDDBDialog(CNewAppPopupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewAppPopupDlg)
	//}}AFX_DATA_INIT

	// Set dialog information : 이곳에서 생성할 다이얼로그의 정보를 입력한다.
	// 좀 더 자세한 설명을 원하면 CDDBDialog.h 화일의 Structure of CDDBDIALOGINFO 내용을 참고한다.
	CDDBDLGINFO DlgInfo;
	DlgInfo.nWidth = 306;
	DlgInfo.nHeight = 236;
	DlgInfo.IDDlgSkinBmpResource = IDB_SKIN_POPUP;
	DlgInfo.nTextColor = RGB( 170, 170, 170);
	DlgInfo.nTextBkColor = RGB( 0, 0, 0);
	SetDialogInfo( DlgInfo);


	// 변수 초기화
	m_szMessage[ 0] = 0;
	m_nIcon = 0;
}


// DoDataExchange
void CNewAppPopupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewAppPopupDlg)
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_MESSAGE, m_cMessage);
	DDX_Control(pDX, IDOK, m_cOK);
}


// Message map
BEGIN_MESSAGE_MAP(CNewAppPopupDlg, CDDBDialog)
	//{{AFX_MSG_MAP(CNewAppPopupDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/************************************************************************************
  CNewAppPopupDlg message handlers
*************************************************************************************/
// OnInitDialog
BOOL CNewAppPopupDlg::OnInitDialog() 
{
	CDDBDialog::OnInitDialog();

	
	// 버튼 생성
	m_cOK.InitDDBButton( IDB_BTNSKIN_NORMAL, NULL);
	m_cOK.MoveWindow( 106, 186);


	// UI 설정
	m_cMessage.SetWindowText( (LPCTSTR)m_szMessage);
	m_cMessage.MoveWindow( 63, 25, 233, 160);

	m_cDDB.PutBitmap( 8, 27, GetSkinBmp(), m_nIcon * 49, 237, 48, 73, SRCCOPY);

	
	// 사운드 출력
//	PlayEffectSound( IDR_WAVE_POPUP);

	return TRUE;
}


// DrawDialogWindow
void CNewAppPopupDlg::DrawDialogWindow()
{
	CRect rect;
	GetWindowRect( &rect);
	m_cDDB.PutBitmap( 0, 0, GetSkinBmp(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
}


// SetMessage
void CNewAppPopupDlg::SetMessage( const char* szMessage)
{
	strcpy( m_szMessage, szMessage);
}


// SetIcon
void CNewAppPopupDlg::SetIcon( int nIconNum)
{
	m_nIcon = nIconNum;
}
