// DBQuestItemEditorDlg.h : 헤더 파일
//

#pragma once



#include "QuestItemList.h"
#include "afxwin.h"



// CDBQuestItemEditorDlg 대화 상자
class CDBQuestItemEditorDlg : public CDialog
{
// 생성
public:
	CDBQuestItemEditorDlg(CWnd* pParent = NULL);	// 표준 생성자

// 대화 상자 데이터
	enum { IDD = IDD_DBQUESTITEMEDITOR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원


	void InitProcess();

public :
	void SetChoiceQIID( const string strQIID );
	void SetFocusToChoiceQIID();
	void SetFocusToCount();
	void Clear();

private :
	CQuestItemList m_QItemList;


// 구현
protected:
	HICON m_hIcon;

	// 메시지 맵 함수를 생성했습니다.
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	afx_msg void OnBnClickedOk();
	CEdit m_szCharName;
	CEdit m_szCID;
	CEdit m_szChoiceQIID;
	CEdit m_szSetCount;
	afx_msg void OnBnClickedFinduser();
	afx_msg void OnBnClickedEditqitem();
	afx_msg void OnBnClickedCancel();
	CButton m_EditBtn;
public:
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedRefresh();
};
