// FolderDlg.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "FolderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CFolderDlg callback proc

int CALLBACK FolderDlgProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	ASSERT(lpData != NULL);
	CFolderDlg* pDlg = (CFolderDlg*)lpData;

	TCHAR lpszPath[MAX_PATH];
	ZeroMemory(&lpszPath, sizeof(lpszPath));

	switch(uMsg)
	{
	case BFFM_INITIALIZED:
		pDlg->OnInitialize(hwnd);
		break;

	case BFFM_SELCHANGED:
		SHGetPathFromIDList((LPITEMIDLIST)lParam, lpszPath);
		pDlg->OnSelChange(lpszPath);
		break;

	case BFFM_VALIDATEFAILED:
		pDlg->OnValidateFaile(lpszPath);
		break;
	}
	return 0;
}



/////////////////////////////////////////////////////////////////////////////
// CFolderDlg construction/destruction

CFolderDlg::CFolderDlg(LPCTSTR lpszTitle /*=NULL*/, LPCTSTR 
					   lpszDefPath /*=NULL*/, LPCTSTR lpszRootPath /*=NULL*/,
					   DWORD dwFlags /*=BIF_RETURNONLYFSDIRS*/, CWnd* 
					   pParentWnd /*=NULL*/)
{
	m_hWnd = NULL;

	ZeroMemory(&m_bi, sizeof(m_bi));
	m_bi.lpszTitle = lpszTitle;

	m_strFolderPath = lpszDefPath;
	SetRootPath(lpszRootPath);

	m_bi.ulFlags = dwFlags;
	m_bi.lpfn = FolderDlgProc;

	m_pParentWnd = pParentWnd;
	HWND hParentWnd = NULL;
	if(pParentWnd != NULL)
	{
		hParentWnd = pParentWnd->m_hWnd;
	}
	m_bi.hwndOwner = hParentWnd;

	m_bi.lParam = (LPARAM)this;
}



CFolderDlg::~CFolderDlg()
{
}



/////////////////////////////////////////////////////////////////////////////
// CFolderDlg operations

void CFolderDlg::SetRootPath(LPCTSTR lpszRootPath)
{
	if(lpszRootPath == NULL)
	{
		m_bi.pidlRoot = NULL;
		return;
	}

	IShellFolder* pDesktopFolder;
	if(FAILED(SHGetDesktopFolder(&pDesktopFolder))) return;

	OLECHAR oleRootPath[MAX_PATH];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpszRootPath, -1, 
		oleRootPath, MAX_PATH);

	LPITEMIDLIST  pIDL = NULL;
	DWORD dwEaten;
	DWORD dwAttributes;
	pDesktopFolder->ParseDisplayName(NULL, NULL, oleRootPath, &dwEaten, 
		&pIDL, &dwAttributes);
	pDesktopFolder->Release();
	m_bi.pidlRoot = pIDL;

	m_strRootPath = lpszRootPath;
}



CString CFolderDlg::GetRootPath()
{
	return m_strRootPath;
}



CString CFolderDlg::GetFolderPath()
{
	return m_strFolderPath;
}



CString CFolderDlg::GetFolderName()
{
	return m_strFolderName;
}



void CFolderDlg::EnableOK(BOOL bEnable /*=TRUE*/)
{
	SendMessage(m_hWnd, BFFM_SETSELECTION, NULL, (LPARAM)bEnable);
}



void CFolderDlg::SelectFolder(LPCTSTR lpszPath)
{
	SendMessage(m_hWnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpszPath);
}



void CFolderDlg::SetStatusText(LPCTSTR lpszStatusText)
{
	SendMessage(m_hWnd, BFFM_SETSTATUSTEXT, NULL, (LPARAM)lpszStatusText);
}



/////////////////////////////////////////////////////////////////////////////
// CFolderDlg implementation

int CFolderDlg::DoModal()
{
	LPITEMIDLIST lpID;
	m_bi.pszDisplayName = m_strFolderName.GetBuffer(MAX_PATH);
	lpID = SHBrowseForFolder(&m_bi);
	m_strFolderName.ReleaseBuffer();
	if(lpID == NULL) return IDCANCEL;

	BOOL bResult = SHGetPathFromIDList(lpID, m_strFolderPath.GetBuffer
		(MAX_PATH));
	m_strFolderPath.ReleaseBuffer();
	if(m_strFolderPath[m_strFolderPath.GetLength() - 1] != '\\')
	{
		m_strFolderPath = m_strFolderPath + "\\";
	}
	return (bResult) ? IDOK : IDCANCEL;
}



void CFolderDlg::OnInitialize(HWND hWnd)
{
	ASSERT(hWnd != NULL);
	m_hWnd = hWnd;

	SelectFolder(m_strFolderPath);
}



void CFolderDlg::OnSelChange(LPCTSTR /*lpszPath*/)
{
}



void CFolderDlg::OnValidateFaile(LPCTSTR /*lpszPath*/)
{
}
