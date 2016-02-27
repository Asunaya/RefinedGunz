// UpdateBuilderDlg.cpp : 구현 파일
//
#include "stdafx.h"
#include "UpdateBuilder.h"
#include "UpdateBuilderDlg.h"
#include "FolderDlg.h"

#include "MZFileSystem.h"
#include ".\updatebuilderdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUpdateBuilderDlg 대화 상자



CUpdateBuilderDlg::CUpdateBuilderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUpdateBuilderDlg::IDD, pParent)
	, m_strOld(_T(""))
	, m_strNew(_T(""))
	, m_strOutput(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUpdateBuilderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_OLD, m_strOld);
	DDX_Text(pDX, IDC_EDIT_NEW, m_strNew);
	DDX_Text(pDX, IDC_EDIT_OUTPUT, m_strOutput);
	DDX_Control(pDX, IDC_RICHEDIT_LOG, m_EditLog);
}

BEGIN_MESSAGE_MAP(CUpdateBuilderDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_OLD, OnBnClickedButtonBrowseOld)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_NEW, OnBnClickedButtonBrowseNew)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_OUTPUT, OnBnClickedButtonBrowseOutput)
	ON_BN_CLICKED(ID_BUILD, OnBnClickedBuild)
	ON_EN_CHANGE(IDC_EDIT_OLD, OnEnChangeEditOld)
	ON_EN_CHANGE(IDC_EDIT_NEW, OnEnChangeEditNew)
	ON_EN_CHANGE(IDC_EDIT_OUTPUT, OnEnChangeEditOutput)
END_MESSAGE_MAP()


// CUpdateBuilderDlg 메시지 처리기

BOOL CUpdateBuilderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	// 프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	
	return TRUE;  // 컨트롤에 대한 포커스를 설정하지 않을 경우 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면 
// 아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
// 프레임워크에서 이 작업을 자동으로 수행합니다.

void CUpdateBuilderDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다. 
HCURSOR CUpdateBuilderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CUpdateBuilderDlg::OnBnClickedButtonBrowseOld()
{
	CFolderDlg dlg("이전 버젼의 폴더를 선택하십시오.", m_strOld, NULL, NULL, this);
	if(dlg.DoModal() != IDOK) return;

	m_strOld=dlg.GetFolderPath();
	UpdateData(FALSE);
}

void CUpdateBuilderDlg::OnBnClickedButtonBrowseNew()
{
	CFolderDlg dlg("새 버젼의 폴더를 선택하십시오.", m_strNew, NULL, NULL, this);
	if(dlg.DoModal() != IDOK) return;

	m_strNew=dlg.GetFolderPath();
	UpdateData(FALSE);
}

void CUpdateBuilderDlg::OnBnClickedButtonBrowseOutput()
{
	CFolderDlg dlg("패키지 파일이 저장될 폴더를 선택하십시오.", m_strOutput, NULL, NULL, this);
	if(dlg.DoModal() != IDOK) return;

	m_strOutput=dlg.GetFolderPath();
	UpdateData(FALSE);
}

BOOL Copy(const char *szFileName,MZFileSystem *pfs,const char *szBasePath)
{
	char *buffer=NULL;
	const char *szCurr=szFileName;
	const char *szNextSlash;
	while((szNextSlash=strchr(szCurr,'/'))!=NULL){
		
		int nLength=(int)(szNextSlash-szFileName);
		char dirname[_MAX_PATH];
		strncpy(dirname,szFileName,nLength);
		dirname[nLength]=0;

		char buffer[_MAX_PATH];
		sprintf(buffer,"%s%s",szBasePath,dirname);

		CreateDirectory(buffer,NULL);

		szCurr=szNextSlash+1;
	}


	MZFile mzf;
	if(!mzf.Open(szFileName,pfs)) return FALSE;

	size_t nFileLength=mzf.GetLength();
	buffer=new char[nFileLength];
	mzf.Read(buffer,(int)nFileLength);
	mzf.Close();

	char fullfilename[_MAX_PATH];
	sprintf(fullfilename,"%s%s",szBasePath,szFileName);

	FILE *file;
	file=fopen(fullfilename,"wb+");
	if(!file) goto error;

	if(fwrite(buffer,1,nFileLength,file)<nFileLength)
		goto error;

	fclose(file);

	delete buffer;

	return TRUE;

error:

	if(file)
		fclose(file);
	if(buffer)
		delete buffer;
	return FALSE;
}

BOOL CUpdateBuilderDlg::Build()
{
	MZFileSystem oldfs,newfs;

//	newfs.Create("./","update");
	newfs.Create(m_strNew,"update");
	oldfs.Create(m_strOld,"update");

	int nCount=0;
	ClearLog();
	for(int i=0;i<newfs.GetFileCount();i++)
	{
		const char *filename=newfs.GetFileName(i);
		const MZFILEDESC *desc=newfs.GetFileDesc(i);
		if(strlen(filename)==0) continue;

		const MZFILEDESC *olddesc=oldfs.GetFileDesc(filename);
		
		// 파일이 없거나, 사이즈가 틀리거나, 위치가 다르거나, 날짜가 다르면
		if(!olddesc || desc->m_iSize != olddesc->m_iSize ||
			stricmp(desc->m_szZFileName,olddesc->m_szZFileName) ||
			oldfs.GetCRC32(filename)!=newfs.GetCRC32(filename))
		{
			if(!Copy(filename,&newfs,m_strOutput))
			{
				Log("file copy error : %s\r\n",filename);
				return FALSE;
			}

			char buffer[256];
			sprintf(buffer,"%s",filename);

			if(strlen(desc->m_szZFileName))
			{
				strcat(buffer," in ");
				strcat(buffer,desc->m_szZFileName);
			}
			strcat(buffer,"\r\n");
			Log(buffer);

			nCount++;
		}
	}

	Log("\r\n %d file copy completed\r\n",nCount);

	return true;
}

void CUpdateBuilderDlg::OnBnClickedBuild()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Build();
}

void CUpdateBuilderDlg::ClearLog()
{
	m_EditLog.SetWindowText("");
}

void __cdecl CUpdateBuilderDlg::Log(const char *pFormat,...)
{
	char temp[256];

	va_list args;

	va_start(args,pFormat);
	vsprintf(temp,pFormat,args);
	va_end(args);

	int nBegin, nEnd;
	nBegin=m_EditLog.LineIndex(m_EditLog.GetLineCount());
	nEnd=nBegin + m_EditLog.LineLength(nBegin);
	m_EditLog.SetSel(nEnd,nEnd);

	m_EditLog.SetFocus();
	m_EditLog.ReplaceSel(temp);
}

void CUpdateBuilderDlg::OnEnChangeEditOld()
{
	// TODO:  RICHEDIT 컨트롤인 경우 이 컨트롤은
	// CDialog::마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여
	// CRichEditCtrl().SetEventMask()를 호출하도록 OnInitDialog() 함수를 재지정하지 않으면
	// 이 알림을 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData();
}

void CUpdateBuilderDlg::OnEnChangeEditNew()
{
	// TODO:  RICHEDIT 컨트롤인 경우 이 컨트롤은
	// CDialog::마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여
	// CRichEditCtrl().SetEventMask()를 호출하도록 OnInitDialog() 함수를 재지정하지 않으면
	// 이 알림을 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData();
}

void CUpdateBuilderDlg::OnEnChangeEditOutput()
{
	// TODO:  RICHEDIT 컨트롤인 경우 이 컨트롤은
	// CDialog::마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여
	// CRichEditCtrl().SetEventMask()를 호출하도록 OnInitDialog() 함수를 재지정하지 않으면
	// 이 알림을 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData();
}
