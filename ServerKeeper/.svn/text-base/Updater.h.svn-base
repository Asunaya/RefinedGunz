#pragma once


/*
 Patch process step.
  1. Download patch file from file server.
  2. Prepare patching work.(ex. execute 'do.bat' batch file.)
  3. Find server process and Stop server process.
  4. Temp folder file copy to main server folder.
  5. Restart server process.
 */

#include "afxinet.h"

#include <vector>
#include <string>
using std::vector;
using std::string;

const static char* CONFINGURE_FILE = "./Configure.ini";

class CUpdater
{
public:
	CUpdater(void);
	virtual ~CUpdater(void);

	inline isInit() { return m_bSuccess; }

	static CUpdater& GetInst()
	{
		static CUpdater Updater;
		return Updater;
	}

	const string GetServerPath() const { return m_strGameServerPath; }
	const string GetAgnetPath() const { return m_strAgentServerPath; }
	const CString GetServerRootDir() const { return m_strLocalDestServerRootDir; }
	const int GetUpdaterPort() const { return m_nPort; }

	CString& GetServerIP() { return m_strServerIP; }

	inline void SetFtpConnection( CFtpConnection* pFtpConnection )
	{
		m_pFtpConnection = pFtpConnection;
	}

	void ParseIP( const string& strIPs );
	
	bool Init();
	bool ConnectPatchFileServer();
	void DisconnectPatchFileServer();

	bool IsConnectableIP( const string& strIP );
	bool IsEnableServerPatch() { return m_IsEnableServerPatch; }
	bool IsEnableAgentPatch()  { return m_IsEnableAgentPatch; }
	
	/// MatchServer.
	bool DownloadServer();
	bool StopServerProcess();
	bool FindServerProcess();
	bool PrepareServerPatching();
	bool ServerPatching();
	bool StartServerProcess();
	// bool FindAgentServerProcess();

	/// MatchAgent.
	bool StartAgent();
	bool StopAgent();
	bool DownloadAgent();
	bool FindAgentProcess();
	bool PrepareAgentPatching();
	bool AgentPatching();
		
private :
	bool LoadConfigureFile();

	const DWORD MakeFileCRC32( const string& strFileName );
	
	bool DownloadPatchInfoFile();
	bool DownloadServerPatchFile();
	bool DownloadAgentPatchFile();
	bool CopyServerPatchFile( const char* pszDstName );	
	bool CopyAgentPatchFile( const char* pszDstName );
	BOOL MakeSubDirToServerRealDir( const CString& strSubDirPath );
	BOOL MaekSubDirToAgentRealDir( const CString& strSubDirPath );

	bool			FindProcess( const string strProcess );
	bool			StopProcess( const string strProcessPath );
	bool			StartProcess( const string strProcessPath );
	bool			LoadListFile();
	bool			DownloadPatchFile( const string& strSrcDir );
	void			RemakeTempDirectory( const char* pszDirName );
	bool			FindFolder( const CString& strCur, const CString& strDst );
	const CString	TokenizeFromBack( const CString& strTok, CString& strSrc );
	
	// 파일이름의 임시 폴더 경로를 만듬.
	inline const CString MakeTempDirFilePath( const CString& strFileName )
	{
		return m_strLocalTempDir + "\\" + strFileName;
	}
	// 다운로드한 Patch file의 정보를 담고있는 정보파일의 경로를 만듬.
	inline const CString MakeDownloadedPatchInfoFilePath()
	{
		return m_strLocalTempDir + "\\" + m_strFtpListFile;
	}
	// 복사될 MatchServer폴더의 파일 경로를 만듬.
	inline const CString MakeRealServerDirFilePath( const CString& strFileName )
	{
		return m_strLocalDestServerRootDir + "\\" + strFileName;
	}
	/// 복사될 MatchAgent폴더의 파일 경로를 만듬.
	inline const CString MakeRealAgentDirFilePath( const CString& strFileName )
	{
		return m_strLocalDestAgentRootDir + "\\" + strFileName; 
	}

	inline bool IsValidDir( const CString& strDir )
	{
		return ( ('.' != strDir) && (".." != strDir) && (-1 == strDir.Find('.')) );
	}

private :
	/// FTP Server정보.
	CInternetSession*	m_pInetSession;
	CFtpConnection*		m_pFtpConnection;
	CString				m_strServerName;
	CString				m_strUserName;
	CString				m_strPassword;
	CString				m_strMatchServerDir;
	CString				m_strMatchAgentDir;
	///

	// 공통 정보.
	CString				m_strFtpRootDir;			
	CString				m_strFtpListFile;
	CString				m_strLocalTempDir;	// Ftp File Server로부터 다운받은 파일일 복사하긴에 임시로 저장하는 폴더.
	CString				m_strServerIP;		// Keeper가 접속할 서버 IP
	CString				m_strTok;			// 파일 경로파싱할때 토큰으로 사용될 문자.
	///

	/// MatchServer정보.
	CString				m_strLocalDestServerRootDir;	// 패치될 Server폴더의 Root폴더 이름. 절대 경로로 해줘야 함.
	string				m_strGameServerPath;			// server실팽 파일의 전체 경로.
	string				m_strGameServerName;			// server process이름.
	DWORD				m_dwServerCRC32;
	bool				m_IsEnableServerPatch;			// Patch목록파일에 server파일이 있으면 true로 설정됨.
	///

	/// MatchAgent정보.
	CString				m_strLocalDestAgentRootDir;		// 패치될 Agent의 Root폴더 이름. 절대 경로로 해줘야 함.
	string				m_strAgentServerPath;			// agent실행 파일의 전체 경로.
	string				m_strAgentServerName;			// agent process이름.
	DWORD				m_dwAgentCRC32;
	bool				m_IsEnableAgentPatch;			// Patch목록파일에 agent파일이 있으면 true로 설정됨.
	///

	bool				m_bSuccess;

	vector< string >	m_vConnectableIP;
	int					m_nPort;
};

#define GetUpdater CUpdater::GetInst()
