#pragma once
#include "afxwin.h"
#include <string>
using std::string;


// MPatchCRCMaker 대화 상자입니다.

class MPatchCRCMaker : public CDialog
{
	DECLARE_DYNAMIC(MPatchCRCMaker)

public:
	MPatchCRCMaker(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~MPatchCRCMaker();
	
	void Clear();

	const DWORD GetServerCRC32() const		{ return m_dwServerCRC32; }
	const DWORD GetAgentCRC32()	const		{ return m_dwAgentCRC32; }

	void SetRootDir( const string& strRootDir )				{ m_strRootDir = strRootDir; }
	void SetListFilePath( const string& strListFilePath )	{ m_strListFilePath = strListFilePath; }

// 대화 상자 데이터입니다.
	enum { IDD = IDD_PATCH_CRC32MAKER_DIALOG };

protected:
	DWORD MakeFileCRC32( const char* pszFileName );
	void MakeListFile();
    const string GetFileName();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
private:
	DWORD m_dwServerCRC32;
	DWORD m_dwAgentCRC32;

	CEdit m_edtServerName;
	CEdit m_edtAgentName;
	CEdit m_edtGunzClientName;

	string m_strRootDir;
	string m_strListFilePath;

public:
	afx_msg void OnBnClickedFindServerFile();
	afx_msg void OnBnClickedFindAgentFile();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
