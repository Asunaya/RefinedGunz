/*
 CDDBDialog 클래스(CDDBDialog.h)

  desc : Dialog를 그려주는 클래스
  date : 2004년 5월 30일
  comp : 임동환
*/


#ifndef _INCLUDED_CDDBDLG_
#define _INCLUDED_CDDBDLG_


#include "CDDB.h"
#include "CDDBButton.h"
#include "CSound.h"


// Define button ID resource name
#define WM_MOUSEDRAG				(WM_USER + 100)			// Message of drag mouse
#define WM_TRAYICON_NOTIFY			(WM_USER + 101)			// Message of trayicon
#define IDC_ABOUTBOX				(WM_USER + 200)			// Control button Aboutbox
#define IDC_MINIMIZE				(WM_USER + 201)			// Control button Minimize
#define IDC_MAXIMIZE				(WM_USER + 202)			// Control button Maximize
#define IDC_QUIT					(WM_USER + 203)			// Control button Quit
#define IDC_MENUBUTTON				(WM_USER + 204)			// Control button Menu
#define IDC_MENUBUTTONMAX			(IDC_MENUBUTTON + 50)	// Control button Menu



/////////////////////////////////////////////////////////////////////////////
// CDDBDialog dialog
class CDDBDialog : public CDialog
{
public:
	// Structure of CDDBDIALOGINFO
	struct CDDBDLGINFO
	{
		// 다이얼로그 설정 변수
		UINT nWidth;						// 생성할 다이얼로그의 넓이를 지정한다.
		UINT nHeight;						// 생성할 다이얼로그의 높이를 지정한다.
		UINT IDDlgSkinBmpResource;			// (필수)다이얼로그에 입힐 스킨 리소스의 ID를 지정한다.
		UINT IDDlgIconResource;				// 다이얼로그에 사용할 아이콘의 리소스의 ID를 지정한다.
		COLORREF nTextColor;				// 다이얼로그 및 콘트롤에 사용할 텍스트의 색상을 지정한다.
		COLORREF nTextBkColor;				// 다이얼로그 및 콘트롤에 사용할 텍스트 배경 색상을 지정한다.

		// 타이틀바 관련 설정 변수
		BOOL bUseTitlebar;					// 이 항목을 TRUE로 설정하면 다이얼로그 윗부분에 타이틀바를 만든다.
		BOOL bUseBmpTitle;					// 이 항목을 TRUE로 설정하면 타이틀바의 제목을 비트맵 이미지를 사용해서 그린다.
		BOOL bUseIconSystemMenu;			// 이 항목을 TRUE로 설정하면 타이틀바의 시스템 메뉴 영역에 아이콘(IDDlgIconResource)을 그린다.

		// 시스템버튼 사용 플래그
		BOOL bUseAboutboxButton;			// 이 항목을 TRUE로 설정하면 Aboutbox 버튼을 사용할 수 있다.
		BOOL bUseMinimizeButton;			// 이 항목을 TRUE로 설정하면 Minimize 버튼을 사용할 수 있다.
		BOOL bUseMaximizeButton;			// 이 항목을 TRUE로 설정하면 Maximize 버튼을 사용할 수 있다.
		BOOL bUseQuitButton;				// 이 항목을 TRUE로 설정하면 Quit 버튼을 사용할 수 있다.

		// 기타 옵션
		BOOL bEnableDlgMove;				// 이 항목을 TRUE로 설정하면 다이얼로그를 드래그해서 이동시킬 수 있다.
		BOOL bEnableDragAndDrop;			// 이 항목을 TRUE로 설정하면 다이얼로그에 파일을 드래그 앤 드롭할 수 있다.
		BOOL bEnableRenderingLayer;			// 이 항목을 TRUE로 설정하면 Rendering용 DDB를 이용할 수 있다.


		// 초기화
		CDDBDLGINFO()
		{
			nWidth					= 320;
			nHeight					= 240;
			IDDlgSkinBmpResource	= NULL;
			IDDlgIconResource		= NULL;
			nTextColor				= RGB(0,0,0);
			nTextBkColor			= RGB(255,255,255);
			bUseTitlebar			= false;
			bUseBmpTitle			= false;
			bUseIconSystemMenu		= false;
			bUseAboutboxButton		= false;
			bUseMinimizeButton		= false;
			bUseMaximizeButton		= false;
			bUseQuitButton			= false;
			bEnableDlgMove			= false;
			bEnableDragAndDrop		= false;
			bEnableRenderingLayer	= false;
		}
	};

	// Structure of CDDBDLGPANELMENU
	struct CDDBDLGPANELMENU : public CObject
	{
		CDDBButton button;
		UINT nIDMenuResource;
		UINT nSubMenu;
	};


	CDDBDialog( UINT nIDTemplate, CWnd* pParentWnd);
	virtual ~CDDBDialog();
	void SetWindowText( LPCTSTR lpszString);
	UINT GetWidth();
	UINT GetHeight();


	//{{AFX_VIRTUAL(CDDBDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void DrawDialogWindow();
	virtual void DrawPanel( int x, int y, int width, int height, LPCTSTR lpszTitle, UINT nIconNum, UINT nIDMenuResource, UINT 
nSubMenu);
	virtual void CreateAboutButton( int x, int y);
	virtual void CreateMaximizeButton( int x, int y);
	virtual void CreateMinimizeButton( int x, int y);
	virtual void CreateQuitButton( int x, int y);
	//}}AFX_VIRTUAL


protected:
	CDDB m_cDDB;						// DDB to drawing screen
	CSound m_sndSound;

	CDDBButton	m_cButtonAbout,			// About button
				m_cButtonMinimize,		// Minimize button
				m_cButtonMaximize,		// Maximize button
				m_cButtonQuit;			// Quit button


	void Rendering();
	void Rendering( int x, int y, int width, int height);
	void Rendering( CDDB *pDDB);
	void GetDialogInfo( CDDBDLGINFO* DlgInfo);
	void SetDialogInfo( CDDBDLGINFO DlgInfo);
	CDC*  GetSkinBmp()			{ return m_ddbSkin.GetDC(); }
	void CreateTrayicon( LPCTSTR lpszTooltip);
	void DeleteTrayicon();
	void PlayEffectSound( UINT nIDWavResource);


	//{{AFX_MSG(CDDBDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseDrag( WPARAM wParam, LPARAM lParam);
	afx_msg void OnAboutbox();
	afx_msg void OnMaximize();
	afx_msg void OnMinimize();
	afx_msg void OnQuit();
	afx_msg LRESULT OnTrayNotification( WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG


private:
	CDDBDLGINFO m_DlgInfo;				// Informations of this dialog
	HICON m_hIcon;						// Handle of icon
	CBrush m_brushCtlColor;				// Window color brush
	CDDB m_ddbRendering;				// DDB to rendering
	CDDB m_ddbSkin;						// DDB of Skin
	BOOL m_fShowWindow;					// Flag of show window
	BOOL m_fTrayicon;					// Flag of trayicon
	BOOL m_fMouseLButton;				// Mouse left button check flag
	CPoint m_nMouseLBtnDownPos;			// Mouse left button position
	NOTIFYICONDATA trayicon;			// Handle of trayicon
	CObList m_listPanelMenu;			// List of panel menu
	UINT m_nNumOfMenuButton;			// Number of menu button

	void OnMenuButton( UINT nID);

	DECLARE_MESSAGE_MAP()
};

#endif
