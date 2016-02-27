#pragma once

#define STICKYNESS	5

class StickyDlg : public CDialog
{
	DECLARE_DYNAMIC(StickyDlg)


private:
	struct Window
	{
		HWND m_hWnd;
		Window * m_Next;
	};

public:
	StickyDlg(CWnd* pParent = NULL);
	virtual ~StickyDlg();

	enum { IDD = IDD_DOWNLOADER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


	// Implementation
protected:

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

private:
	static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
	void AddWindow(HWND hWnd);
	void DeleteWindows(void);
	void SnapWindow(bool Sizing, LPRECT pRect);

	Window * m_RootWindow;
	CPoint m_MouseOffset;
};


