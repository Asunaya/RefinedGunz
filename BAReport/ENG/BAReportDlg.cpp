// BAReportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BAReport.h"
#include "BAReportDlg.h"
#include ".\bareportdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define WM_REPORTMESSAGE	WM_USER+1;




/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBAReportDlg dialog

CBAReportDlg::CBAReportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBAReportDlg::IDD, pParent), m_bProgress(true), m_bSingle(false)
{
	//{{AFX_DATA_INIT(CBAReportDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBAReportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBAReportDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBAReportDlg, CDialog)
	//{{AFX_MSG_MAP(CBAReportDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBAReportDlg message handlers

BOOL CBAReportDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

//	ModifyStyleEx(WS_EX_APPWINDOW, 0);

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}


	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	
	if (g_bAgree)
	{
		g_pFileTransfer->BeginTransfer();
		PostMessage(WM_CLOSE, 0, 0);
	}
	else
	{
		if(m_bSingle) {
			m_SingleDlg.Create(IDD_DLG_SINGLE, this);
			m_SingleDlg.ShowWindow(SW_SHOW);
		}
		else{
			m_AgreementDlg.Create(IDD_DLG_AGREEMENT, this);
			m_AgreementDlg.ShowWindow(SW_SHOW);
		}
	}

	return FALSE;  // return TRUE  unless you set the focus to a control
}

void CBAReportDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBAReportDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBAReportDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void SetCommand(LPTSTR pszCmd)
{

	CString strCmd = pszCmd;
	strCmd.MakeLower();
	

	if (strCmd == "/help") {
		AfxMessageBox("USAGE_EX) BAReport app=acesaga;addr=moon.maiet.net;port=21;id=ftp;passwd=ftp@;gid=10;user=moanus;localfile=rslog.txt;remotefile=remote_rslog.txt;agree=0;srcdelete=0;", MB_OK);
		return;
	}

#define MAX_LOCALFILE	10
	char szLocalFileName[MAX_LOCALFILE][256];
	char szRemoteFileName[MAX_LOCALFILE][256];
	int nLocalFileCount = 0, nRemoteFileCount = 0;


	CString strAppName, strAddr, strPort, strID, strPasswd, strGID, strUser, strLocalFileName, strRemoteFileName;
	CString strAgree, strSrcDelete;

	while (true) {
		int nPos = strCmd.Find(';');
		if ((nPos == -1) && (strCmd.GetLength() == 0))
			break;
		CString strTmp;
		if (nPos != -1)
			strTmp = strCmd.Left(nPos);
		else
			strTmp = strCmd;

		int nPos2 = strTmp.Find('=');
		if (nPos2 != -1) {
			CString strName = strTmp.Left(nPos2);
			CString strVal = strTmp.Mid(nPos2+1);
			TRACE("%s == %s \n", strName, strVal);
			if (strName == "app")
				strAppName = strVal;
			else if (strName == "addr")
				strAddr = strVal;
			else if (strName == "port")
				strPort = strVal;
			else if (strName == "id")
				strID = strVal;
			else if (strName == "passwd")
				strPasswd = strVal;
			else if (strName == "gid")
				strGID = strVal;
			else if (strName == "user")
				strUser = strVal;
			else if (strName == "localfile")
			{
				strLocalFileName = strVal;

				while (true) {
					if (nLocalFileCount >= MAX_LOCALFILE) break;
					int nLocalPos = strLocalFileName.Find(',');
					if ((nLocalPos == -1) && (strLocalFileName.GetLength() == 0)) break;

					CString strTmp;
					if (nLocalPos != -1)
						strTmp = strLocalFileName.Left(nLocalPos);
					else
						strTmp = strLocalFileName;

					strcpy(szLocalFileName[nLocalFileCount++], strTmp);

					if (nLocalPos != -1)
						strLocalFileName.Delete(0, nLocalPos+1);
					else
						strLocalFileName.Delete(0, strLocalFileName.GetLength());
				}
			}
			else if (strName == "remotefile") {
				strRemoteFileName = strVal;

				while (true) {
					if (nRemoteFileCount >= MAX_LOCALFILE) break;
					int nRemotePos = strRemoteFileName.Find(',');
					if ((nRemotePos == -1) && (strRemoteFileName.GetLength() == 0)) break;

					CString strTmp;
					if (nRemotePos != -1)
						strTmp = strRemoteFileName.Left(nRemotePos);
					else
						strTmp = strRemoteFileName;

					strcpy(szRemoteFileName[nRemoteFileCount++], strTmp);

					if (nRemotePos != -1)
						strRemoteFileName.Delete(0, nRemotePos+1);
					else
						strRemoteFileName.Delete(0, strRemoteFileName.GetLength());
				}
			}
			else if (strName == "srcdelete") {
				if (strVal == "1") g_pFileTransfer->SetDeleteSrcFile(true);
			}
			else if (strName == "agree") {
				if (strVal == "1")
				{
					g_bAgree = true;
				}
			}

		}
		
		if (nPos != -1)
			strCmd.Delete(0, nPos+1);
		else
			strCmd.Delete(0, strCmd.GetLength());
	}
//	acesaga_0928_911_moanus_rslog.txt
//	CTime time = CTime::GetCurrentTime();
//	CString strRemoteFileName;
//	strRemoteFileName.Format("%s_%2d%2d_%s_%s_%s", strApp, time.GetMonth(), time.GetDate(), strGID, strUser, strLocalFile);

	for (int i = 0; i < nLocalFileCount; i++) {
		CString strLocal = szLocalFileName[i];
		CString strRemote;
		if (i < nRemoteFileCount) {
			strRemote = szRemoteFileName[i];
		}
		else {
			strRemote = szRemoteFileName[0];
		}
		g_pFileTransfer->PutFileQueue(strAddr, atoi((LPCSTR)strPort), strID, strPasswd, strLocal, strRemote);
	}

//		g_pFileTransfer->PutFileQueue(strAddr, atoi((LPCSTR)strPort), strID, strPasswd, strLocalFileName, strRemoteFileName);
}

void CBAReportDlg::Upload()
{
	if (g_bAgree) return;

	m_TransferDlg.Create(IDD_DLG_TRANSFER, this);
	m_TransferDlg.ShowWindow(SW_SHOW);

	g_pFileTransfer->SetTransferCallback(m_TransferDlg.TransferCallback);
	g_pFileTransfer->BeginTransfer();
}

void CBAReportDlg::OnDestroy() 
{
	g_pFileTransfer->Destroy();
	CDialog::OnDestroy();
}

int CBAReportDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.
	return 0;
}
