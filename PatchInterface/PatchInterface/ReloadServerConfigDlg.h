#pragma once
#include "afxcmn.h"

#include <vector>
#include <string>
#include "afxwin.h"
using namespace std;


#define RELOAD_LIST "reloadlist.txt"


typedef vector< string > ReloadListVec;


// ReloadServerConfigDlg 대화 상자입니다.
class ReloadServerConfigDlg : public CDialog
{
	DECLARE_DYNAMIC(ReloadServerConfigDlg)

private :
	ReloadListVec m_ReloadFileList;

private :
	bool InitReport();

public:
	ReloadServerConfigDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~ReloadServerConfigDlg();

	bool LoadReloadList();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ReloadFileListDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
private:
	CListCtrl m_ReloadList;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedReload();
private:
	CEdit m_NewHashValue;
public:
	afx_msg void OnBnClickedAddhashmap();
};
