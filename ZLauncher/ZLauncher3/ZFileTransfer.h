/******************************************************************
   
   ZFileTransfer.h

     Corperation : MAIET entertainment
     Programmer  : Lim Dong Hwan
	 Date        : 22.June.2005

*******************************************************************/


#ifndef _ZFILETRANSFER
#define _ZFILETRANSFER


#include <afxinet.h>


// class  MFTSession
class MFTSession : public CInternetSession
{
public:
	MFTSession(LPCTSTR pszAppName = NULL, int nMethod = PRE_CONFIG_INTERNET_ACCESS);

	virtual void OnStatusCallback(DWORD dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInfomration, DWORD dwStatusInformationLen);
};


// class File Transfer UI information
class ZFileTransferUI
{
protected:
	char				m_szDownloadFileName[ 256];
	unsigned long		m_nTotalDownloadSize;
	unsigned long		m_nCurrDownloadSize;


public:
	ZFileTransferUI()
	{
		m_szDownloadFileName[ 0] = 0;
		m_nTotalDownloadSize = 0;
		m_nCurrDownloadSize = 0;
	};

    const char* GetDownloadFileName()							{ return m_szDownloadFileName; }
	void SetDownloadFileName( const char* szFileName)			{ strcpy( m_szDownloadFileName, szFileName); }
	void ClearDownloadFileName()								{ m_szDownloadFileName[ 0] = 0; }

	unsigned long GetTotalDownloadSize()						{ return m_nTotalDownloadSize; }
	void SetTotalDownloadSize( unsigned long nSize)				{ m_nTotalDownloadSize = nSize; }
	void AddTotalDownloadSize( unsigned long nSize)				{ m_nTotalDownloadSize += nSize; }
	void ClearTotalDownloadSize()								{ m_nTotalDownloadSize = 0; }

	unsigned long GetCurrDownloadSize()							{ return m_nCurrDownloadSize; }
	void SetCurrDownloadSize( unsigned long nSize)				{ m_nCurrDownloadSize = nSize; }
	void AddCurrDownloadSize( unsigned long nSize)				{ m_nCurrDownloadSize += nSize; }
	void ClearCurrDownloadSize()								{ m_nCurrDownloadSize = 0; }
};



// class ZFileTransfer
class ZFileTransfer
{
protected:
	MFTSession			m_Session;
	CHttpConnection*	m_pConnection;
	ZFileTransferUI		m_FileTransferUI;

	bool				m_bOpenConnection;
	bool				m_bStopDownload;

	bool				m_bInitialize;
	char				m_szAddress[ 256];
	char				m_szDefDir[ 256];
	unsigned long		m_nPort;
	char				m_szID[ 256];
	char				m_szPassword[ 256];

	TCHAR				m_szLastError[ 1024];


public:
	ZFileTransfer();
	virtual ~ZFileTransfer();

	bool Create( const char* pszAddress, unsigned long nPort, const char* pszID, const char* pszPassword);
	bool Destroy();

	bool OpenConnection();
	bool CloseConnection();

	CHttpConnection* GetConnection()		{ return m_pConnection; }
	ZFileTransferUI* GetFileTransferUI()	{ return &m_FileTransferUI; }

	bool DownloadFile( const char* pszRemoteFileName, const char* pszLocalFileName, DWORD nFileSize);
	bool ChangeDirectory( const char* pszDirectory);
	void StopDownload()						{ m_bStopDownload = true; }

	char* GetLastError()					{ return m_szLastError; }
};

#endif
