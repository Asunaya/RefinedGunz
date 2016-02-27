// FolderDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FOLDERDLG_H__19DFE03F_AD76_45BA_ABAF_5738FEB7633E__INCLUDED_)
#define AFX_FOLDERDLG_H__19DFE03F_AD76_45BA_ABAF_5738FEB7633E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



/////////////////////////////////////////////////////////////////////////////
// CFolderDlg dialog

class CFolderDlg : public CObject
{
public:
	CFolderDlg(LPCTSTR lpszTitle = NULL, LPCTSTR lpszDefPath = NULL, 
		LPCTSTR lpszRootPath = NULL, DWORD dwFlags = BIF_RETURNONLYFSDIRS, CWnd* 
		pParentWnd = NULL);
	virtual ~CFolderDlg();

	// Attributes
protected:
	HWND m_hWnd;
	CWnd* m_pParentWnd;

	CString m_strComment;
	CString m_strRootPath;
	CString m_strFolderPath;
	CString m_strFolderName;

public:
	BROWSEINFO m_bi;


	// Operations
public:
	void SetRootPath(LPCTSTR lpszRootPath);
	CString GetRootPath();
	CString GetFolderPath();
	CString GetFolderName();

	void EnableOK(BOOL bEnable = TRUE);
	void SelectFolder(LPCTSTR lpszPath);
	void SetStatusText(LPCTSTR lpszStatusText);


	// Overrides
	virtual int DoModal();
	virtual void OnInitialize(HWND hWnd);
	virtual void OnSelChange(LPCTSTR lpszPath);
	virtual void OnValidateFaile(LPCTSTR lpszPath);
};



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FOLDERDLG_H__19DFE03F_AD76_45BA_ABAF_5738FEB7633E__INCLUDED_)


