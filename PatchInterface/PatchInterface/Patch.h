#pragma once


// CPatch 대화 상자입니다.

class CPatch : public CDialog
{
	DECLARE_DYNAMIC(CPatch)

public:
	CPatch(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CPatch();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_PATCH_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedDownloadServerPatch();
	afx_msg void OnBnClickedPrepareServerPatch();
	afx_msg void OnBnClickedPatchServer();
	afx_msg void OnBnClickedResetServerPatch();
	afx_msg void OnBnClickedDownloadAgentPatch();
	afx_msg void OnBnClickedPrepareAgentPatch();
	afx_msg void OnBnClickedPatchAgent();
	afx_msg void OnBnClickedResetAgentPatch();
};
