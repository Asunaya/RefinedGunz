#pragma once


// CBuildDlg dialog

class CBuildDlg : public CDialog
{
	DECLARE_DYNAMIC(CBuildDlg)

public:
	CBuildDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBuildDlg();

	void StartBuild();
	bool Build();

// Dialog Data
	enum { IDD = IDD_DLG_BUILD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	afx_msg long OnBuildInit(WPARAM wParam, LPARAM lParam);

};
