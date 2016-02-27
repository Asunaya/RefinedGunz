// BASingleDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "BAReport.h"
#include "BASingleDlg.h"
#include "BASysInfo.h"
#include "FileTransfer.h"
#include "BAReportDlg.h"


// BASingleDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(BASingleDlg, CDialog)
BASingleDlg::BASingleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BASingleDlg::IDD, pParent)
{
}

BASingleDlg::~BASingleDlg()
{
}

void BASingleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_NAME, m_Name);
	DDX_Control(pDX, IDC_EDIT_SYMPTOM, m_Symptom);
}


BEGIN_MESSAGE_MAP(BASingleDlg, CDialog)
END_MESSAGE_MAP()

BOOL BASingleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	m_Name.SetWindowText("NULL");
	m_Name.SetSel(0,-1);
	m_Name.SetFocus();
	return FALSE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void BASingleDlg::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CWnd *pStatic = GetDlgItem(IDC_STATIC_SINGLE);
	pStatic->SetWindowText("필요한 정보를 수집하고 있습니다. 잠시만 기다려주세요.");

	time_t currtime;
	time(&currtime);
	struct tm* pTM = localtime(&currtime);

	CString strName;
	m_Name.GetWindowText(strName);

	char szRemoteFileName[256];
	wsprintf(szRemoteFileName, "incoming/info/_report_%s_%.2d%.2d_%.2d%.2d.txt",
		(LPCSTR)strName, pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min );

#define SYSINFO_FILENAME	"sysinfo.txt"

	FILE *pFile = fopen(SYSINFO_FILENAME,"w+");
	if(pFile) {
		CString strSymptom;
		m_Symptom.GetWindowText(strSymptom);
		fprintf(pFile,(LPCSTR)strSymptom);
		fprintf(pFile,"\n");
		fclose(pFile);

		BASysInfo(SYSINFO_FILENAME);

		g_pFileTransfer->PutFileQueue("www.battlearena.com", 21, "ftp", "ftp", SYSINFO_FILENAME, szRemoteFileName);

		CBAReportDlg* pDlg = (CBAReportDlg*)AfxGetMainWnd();
		pDlg->Upload();
		DestroyWindow();
	}else {
		AfxMessageBox("파일저장실패.");
	}

//	CDialog::OnOK();
}

void BASingleDlg::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	CBAReportDlg* pDlg = (CBAReportDlg*)AfxGetMainWnd();
	pDlg->DestroyWindow();

//	CDialog::OnCancel();
}
