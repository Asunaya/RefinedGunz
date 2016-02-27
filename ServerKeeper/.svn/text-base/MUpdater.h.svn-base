#ifndef _MUpdater
#define _MUpdater

#include "afxinet.h"

#include <string>
using namespace std;

class MUpdater
{
public :
	MUpdater();
	~MUpdater();

	void SetInfo( const string& strName, const string& strFTP_URL, const string& strUserID,
		const string& strPassword, const string& strFTP_RootDirPath, const string& strFTP_DirInfoFile,
		const string& strWorkRootDirPath, const string& strDownloadTempDir, 
		const string& strPatchRootPath, const string& strPatchFileName );

	void SetName( const string& strName )							{ m_strName = strName; }

	void SetFTP_URL( const string& strFTP_URL )						{ m_strFTP_URL = strFTP_URL; }
	void SetUserID( const string& strUserID )						{ m_strUserID = strUserID; }
	void SetPassword( const string& strPassword )					{ m_strPassword = strPassword; }
	void SetFTP_RootDirPath( const string& strFTP_RootDirPath )		{ m_strFTP_RootDirPath = strFTP_RootDirPath; }
	void SetFTP_DirInfoFile( const string& strFTP_DirInfoFile )		{ m_strFTP_DirInfoFile = strFTP_DirInfoFile; }

	void SetCRC32CheckSum( const DWORD dwCRC32CheckSum )			{ m_dwCRC32CheckSum = dwCRC32CheckSum; }
	void SetWorkRootDirPath( const string& strWorkRootDirPath )		{ m_strWorkRootDirPath = strWorkRootDirPath; }
	void SetDownloadTempDir( const string& strDownloadTempDir )		{ m_strDownloadTempDir = strDownloadTempDir; }
	void SetPatchRootDirPath( const string& strPatchRootDirPath )	{ m_strPatchRootDirPath = strPatchRootDirPath; }
	void SetPatchFileName( const string& strPatchFileName )			{ m_strPatchFileName = strPatchFileName; }

	bool CreateDownloadWorkThread();
	bool CreatePreparePatchWorkThread();
	bool CreatePatchWorkThread();

	bool Download();
	bool Prepare();
	bool CopyPatchFileToPatchRootDir();

private :
	const string& GetName() const				{ return m_strName; }

	const string& GetFTP_URL() const			{ return m_strFTP_URL; }
	const string& GetUserID() const				{ return m_strUserID; }
	const string& GetPassword() const			{ return m_strPassword; }
	const string& GetFTP_RootDirPath() const	{ return m_strFTP_RootDirPath; }
	const string& GetFTP_DirInfoFile() const	{ return m_strFTP_DirInfoFile; }

	const DWORD	  GetCRC32CheckSum() const		{ return m_dwCRC32CheckSum; }
	const string& GetWorkRootDirPath() const	{ return m_strWorkRootDirPath; }
	const string& GetDownloadTempDir() const	{ return m_strDownloadTempDir; }
	const string& GetPatchRootDirPath() const	{ return m_strPatchRootDirPath; }
	const string& GetPatchFileName() const		{ return m_strPatchFileName; }
	
	bool ConnectFTPServer();
	void DisconnectFTPServer();
	bool DownloadFTP_DirInfoFile();
	bool LoadFTP_DirInfoFile();
	bool DownloadPatchFile();

	bool CheckCRC32CheckSum();
	bool CreateDownloadTempDir( const string& strParentDir );
	bool CopyPatchFile( const string& strDestDirPath );

	bool			CreateSubDir( const string& strDestRootDirPath, const string& strSrcDirPath );
	const string	TokenizeFromBack( const string& str );
	const string	LinkTwoPath( const string& strPath1, const string& strPath2 );
	bool			IsValidDirName( const CString& strDir );
	bool			FindFolder( const string& strRootDir, const string& strFindDir ); // strRootDir의 폴더에서만 검사. 트리구조로 탐색하지 않음.

	static DWORD WINAPI DownloadWorkThread( void* pWorkContext );
	static DWORD WINAPI PreparePatchWorkThread( void* pWorkContext );
	static DWORD WINAPI PatchWorkThread( void* pWorkContext );
	
private :
	string m_strName;

	CInternetSession*	m_pInetSession;
	CFtpConnection*		m_pFtpConnection;

	// ftp info.
	string m_strFTP_URL;
	string m_strUserID;
	string m_strPassword;
	string m_strFTP_RootDirPath;
	string m_strFTP_DirInfoFile;
	
	// local info.
	DWORD	m_dwCRC32CheckSum;
	string	m_strWorkRootDirPath;
	string	m_strDownloadTempDir;
	string	m_strPatchRootDirPath;
	string	m_strPatchFileName;
};

#endif