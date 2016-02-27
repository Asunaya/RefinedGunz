#include "stdafx.h"
#include "MUpdater.h"
#include "MDebug.h"
#include "mcrc32.h"


MUpdater::MUpdater() : m_pInetSession( 0 ), m_pFtpConnection( 0 )
{
}


MUpdater::~MUpdater()
{
}


void MUpdater::SetInfo( const string& strName, const string& strFTP_URL, const string& strUserID,
					   const string& strPassword, const string& strFTP_RootDirPath, 
					   const string& strFTP_DirInfoFile, const string& strWorkRootDirPath, 
					   const string& strDownloadTempDir, const string& strPatchRootDirPath,
					   const string& strPatchFileName )
{
	SetName( strName );
	SetFTP_URL( strFTP_URL );
	SetUserID( strUserID );
	SetPassword( strPassword );
	SetFTP_RootDirPath( strFTP_RootDirPath );
	SetFTP_DirInfoFile( strFTP_DirInfoFile );
	SetWorkRootDirPath( strWorkRootDirPath );
	SetDownloadTempDir( strDownloadTempDir );
	SetPatchRootDirPath( strPatchRootDirPath );
	SetPatchFileName( strPatchFileName );
}


bool MUpdater::Download()
{
	if( !ConnectFTPServer() ) 
		return false;
	if( !DownloadFTP_DirInfoFile() ) 
	{
		DisconnectFTPServer();
		return false;
	}
	if( !LoadFTP_DirInfoFile() )
	{
		DisconnectFTPServer();
		return false;
	}
	if( !DownloadPatchFile() ) 
	{
		DisconnectFTPServer();
		return false;
	}
	if( !CheckCRC32CheckSum() )
	{
		DisconnectFTPServer();
		return false;
	}

	DisconnectFTPServer();

	return true;
}


bool MUpdater::Prepare()
{
	return true;
}


bool MUpdater::ConnectFTPServer()
{
	DisconnectFTPServer();

	m_pInetSession = new CInternetSession();
	if( 0 == m_pInetSession )
		return false;

	try
	{
		m_pFtpConnection = m_pInetSession->GetFtpConnection( GetFTP_URL().c_str(), GetUserID().c_str(), GetPassword().c_str() );
		if( 0 == m_pFtpConnection )
			return false;
	}
	catch( CInternetException* pEx )
	{
		TCHAR szErr[ 1024 ];
		pEx->GetErrorMessage( szErr, 1024 );
		mlog( "MUpdater::ConnectFTPServer - %s\n", szErr );
		return false;
	}

	return true;
}


void MUpdater::DisconnectFTPServer()
{
	if( 0 != m_pFtpConnection )
	{
		m_pFtpConnection->Close();
		delete m_pFtpConnection;
		m_pFtpConnection = 0;
	}

	if( 0 != m_pInetSession )
	{
		m_pInetSession->Close();
		delete m_pInetSession;
		m_pInetSession = 0;
	}
}


bool MUpdater::DownloadFTP_DirInfoFile()
{
	if( !FindFolder(GetWorkRootDirPath(), GetDownloadTempDir()) )
		return false;

	if( 0 == m_pFtpConnection )
		return false;

	if( !m_pFtpConnection->SetCurrentDirectory(GetFTP_RootDirPath().c_str()) )
		return false;

	CString strFileName;
	BOOL	bContinue;

	CFtpFileFind ftpFind( m_pFtpConnection );
	bContinue = ftpFind.FindFile( GetFTP_RootDirPath().c_str() );
	if( !bContinue )
		return false;

	while( bContinue )
	{
		bContinue = ftpFind.FindNextFile();
		strFileName = ftpFind.GetFileName();

		if( GetFTP_DirInfoFile() != strFileName.GetBuffer() )
			continue;
		
		if( ftpFind.IsDirectory() )
			continue;

		if( !m_pFtpConnection->GetFile(strFileName, 
				LinkTwoPath(GetWorkRootDirPath(), GetDownloadTempDir()).c_str(), 
				TRUE) )
		{
			mlog( "fail to Download FTP DirInfoFile(%s)\n", 
				GetDownloadTempDir().c_str() );
			return false;
		}
	}

	return true;
}


bool MUpdater::LoadFTP_DirInfoFile()
{
	string strFTP_DirInofFilePath = LinkTwoPath( GetDownloadTempDir(), GetFTP_DirInfoFile() );
	FILE* fp = fopen( GetFTP_DirInfoFile().c_str(), "r" );
	if( 0 == fp )
		return false;

	DWORD dwCRC32CheckSum;
	char szBuf[ 512 ];
	char szDir[ 512 ];

	while( 0 != fgets(szBuf, 511, fp) )
	{
		memset( szDir, 0, 512 );
		sscanf( szBuf, "%s %u", szDir, &dwCRC32CheckSum );

		if( GetName() == string(szDir) )
		{
			fclose( fp );
			SetCRC32CheckSum( dwCRC32CheckSum );
			return true;
		}
	}

	fclose( fp );
	return false;
}


bool MUpdater::DownloadPatchFile()
{
	if( 0 == m_pFtpConnection )
		return false;

	BOOL	bContinue;
	CString strCurDir;
	CString strFileName;

	if( !m_pFtpConnection->SetCurrentDirectory(GetName().c_str()) )
		return false;

	if( !m_pFtpConnection->GetCurrentDirectory(strCurDir) )
		return false;

	strCurDir += "/*";

	CFtpFileFind ftpFind( m_pFtpConnection );

	if( !(bContinue = ftpFind.FindFile(strCurDir)) )
	{
		// 파일이 없음.
		return false;
	}

	string strLocalFileName;
	const string strWorkTempDirPath = LinkTwoPath( GetWorkRootDirPath(), GetDownloadTempDir() );

	while( bContinue )
	{
		bContinue			= ftpFind.FindNextFile();
		strFileName			= ftpFind.GetFileName();
		strLocalFileName	= LinkTwoPath( strWorkTempDirPath, strFileName.GetBuffer() );

		if( !m_pFtpConnection->GetFile(strFileName,  strLocalFileName.c_str(), TRUE) )
		{
			ftpFind.Close();
			mlog( "MUpdater::DownloadPatchFile - %s파일 다운로드 실패.\n", strFileName.GetBuffer() );
			return false;
		}
	}

	ftpFind.Close();

	return true;
}


bool MUpdater::CopyPatchFileToPatchRootDir()
{
	return true;
}


bool MUpdater::CheckCRC32CheckSum()
{
	const string strTempDirPath = LinkTwoPath( GetWorkRootDirPath(), GetDownloadTempDir() );
	const string strFilePath = LinkTwoPath( strTempDirPath, GetPatchFileName() );

	// 바이너리로 열어서 데이터를 읽음.
	CFile file;
	CFileException e;
	if( !file.Open(strFilePath.c_str(), CFile::modeRead | CFile::typeBinary, &e) )
	{
		char szErr[ 1024 ] = {0,};
		e.GetErrorMessage( szErr, 1024 );
		mlog( "MUpdater::CheckCRC32CheckSum - %s(%s)\n", szErr, strFilePath.c_str() );
		return 0;
	}

	const unsigned int nFileLen = file.GetLength();
	char* szFileData = new char[ nFileLen + 1 ];
	file.Read( szFileData, nFileLen );
	file.Close();
	const bool bRes = MCRC32::BuildCRC32( reinterpret_cast<BYTE*>(szFileData), nFileLen );
	delete [] szFileData;

	// 읽은 데이터로 CRC32생성.
	return bRes;
}


const string MUpdater::LinkTwoPath( const string& strPath1, const string& strPath2 )
{
	return strPath1 + "\\" + strPath2;
}


bool MUpdater::IsValidDirName( const CString& strDir )
{
	return ( ('.' != strDir) && (".." != strDir) && (-1 == strDir.Find('.')) );
}


bool MUpdater::FindFolder( const string& strRootDir, const string& strFindDir )
{
	CFileFind fileFind;
	CString rootDir = strRootDir.c_str();

	BOOL bContinue = fileFind.FindFile( rootDir + ".\\*" );
	if( !bContinue )
		return false;

	while( bContinue )
	{
		bContinue = fileFind.FindNextFile();
		CString tempName = fileFind.GetFileName();
		if( fileFind.IsDirectory() )
		{
			if( !IsValidDirName(fileFind.GetFileName()) )
			continue;

			if( strFindDir == string(tempName.GetBuffer()) )
			{
				fileFind.Close();
				return true;
			}
		}
	}

	fileFind.Close();

	return false;
}


const string MUpdater::TokenizeFromBack( const string& str )
{
	size_t index = str.find( GetDownloadTempDir() );
	if( string::npos == index )
		return str;

	while( true )
	{
		index += (GetDownloadTempDir().length() + 1);
		int tmpIndex = str.find( GetDownloadTempDir(), index );
		if( string::npos == tmpIndex )
			break;
		index = tmpIndex;
	}

	string strValidPath;
	if( index < str.length() )
		strValidPath = &str[index];

	return strValidPath;
}


bool MUpdater::CreateSubDir( const string& strDestRootDirPath, const string& strSrcDirPath )
{
	const size_t pos = strSrcDirPath.find( GetDownloadTempDir() );
	if( string::npos == pos ) 
		return false;

	const string strDestFullDirPath = strDestRootDirPath + TokenizeFromBack( strSrcDirPath );

	return CreateDirectory( strDestFullDirPath.c_str(), NULL );
}


bool MUpdater::CreateDownloadTempDir( const string& strParentDir )
{
	CString strFileName = strParentDir.c_str();
	CString strFilePath;
	CString strFullName;
	CFileFind fileFind;

	bool bDelete = false;
	BOOL bContinue = fileFind.FindFile( strFileName + "\\*" );
	if( !bContinue )
		return false;
	
	while( bContinue )
	{
		bContinue = fileFind.FindNextFile();

		if( fileFind.IsDirectory() )
		{
			strFileName = fileFind.GetFileName();
			if( !IsValidDirName(strFileName.GetBuffer()) )
				continue;

			strFilePath = fileFind.GetFilePath();
			strFilePath.Remove( '.' );
            
			// Directory의 내용을 지우기 위해서 제귀호출을 함.
			if( !CreateDownloadTempDir(strFilePath.GetBuffer()) )
			{
				fileFind.Close();
				return false;
			}

			strFullName = strFilePath + '\\';
			if( !RemoveDirectory(strFullName) )
			{
				fileFind.Close();
				return false;
			}
		}
		else
		{
			strFilePath = fileFind.GetFilePath();
			if( !DeleteFile(strFilePath) )
			{
				fileFind.Close();
				return false;
			}
		}
	}

	fileFind.Close();
	return true;
}


bool MUpdater::CopyPatchFile( const string& strDestDirPath )
{
	if( strDestDirPath.empty() ) return false;

	BOOL bContinue;
	CString strDirName = strDestDirPath.c_str();
	CString strPathName;
	CString strFullName;
	
	// 임시 폴더 검사.

	CFileFind fileFind;	

	bContinue = fileFind.FindFile( strDirName + ".\\*" );
	if( !bContinue )
		return false;

	while( bContinue )
	{
		bContinue = fileFind.FindNextFile();
		if( fileFind.IsDirectory() )
		{
			if( !IsValidDirName(fileFind.GetFileName()) )
				continue;

			strPathName = fileFind.GetFilePath();
			strPathName.Remove( '.' );

			string strRealDir = LinkTwoPath( GetWorkRootDirPath(), TokenizeFromBack(strDirName.GetBuffer()) );

			// 폴더안의 파일을 복사하기전에 해당 폴더가 존재하는지 검사해야 함.
			// 만약 없는 폴더면은 폴더를 먼저 생성한후 파일 복사를 함.
			// 폴더생성전에 파일 복사를 할경우 서브폴더의 서브폴더가 생성되지 않음.
			if( !FindFolder(strRealDir, fileFind.GetFileName().GetBuffer()) )
			{
				// craete new folder
				
				if( !CreateSubDir(GetWorkRootDirPath(), fileFind.GetFilePath().GetBuffer()) )
				{
					fileFind.Close();
					return false;
				}
			}

 			if( !CopyPatchFile(strPathName.GetBuffer()) )
			{
				fileFind.Close();
				return false;
			}
		}
		else
		{
			// copy file

			const string strPartName = TokenizeFromBack( fileFind.GetFilePath().GetBuffer() );
			
			// 파일이 복사될 경로를 만듬.
			const string strDstFullPath = LinkTwoPath( GetWorkRootDirPath(), strPartName );

			if( !CopyFile( fileFind.GetFilePath(), strDstFullPath.c_str(), FALSE ) )
			{
				fileFind.Close();
				return false;
			}
		}
	}

	fileFind.Close();

	return true;
}

bool MUpdater::CreateDownloadWorkThread()
{
	HANDLE  hThread;
	DWORD   dwThreadId;

	hThread = CreateThread(NULL, 0, DownloadWorkThread, this, 0, &dwThreadId);

	return (NULL != hThread);
}

bool MUpdater::CreatePreparePatchWorkThread()
{
	HANDLE  hThread;
	DWORD   dwThreadId;

	hThread = CreateThread(NULL, 0, PatchWorkThread, this, 0, &dwThreadId);

	return (NULL != hThread);
}


bool MUpdater::CreatePatchWorkThread()
{
	HANDLE  hThread;
	DWORD   dwThreadId;

	hThread = CreateThread(NULL, 0, PreparePatchWorkThread, this, 0, &dwThreadId);

	return (NULL != hThread);
}


DWORD WINAPI MUpdater::DownloadWorkThread( void* pWorkContext )
{
	if( 0 == pWorkContext )
		return -1;

	MUpdater* pUpdater = reinterpret_cast< MUpdater* >( pWorkContext );
	if( !pUpdater->Download() )
	{
		return -1;
	}
	
	return 0;
}


DWORD WINAPI MUpdater::PreparePatchWorkThread( void* pWorkContext )
{
	if( 0 == pWorkContext )
		return -1;
	MUpdater* pUpdater = reinterpret_cast< MUpdater* >( pWorkContext );
	if( !pUpdater->Prepare() )
	{
		return -1;
	}

	return 0;
}


DWORD WINAPI MUpdater::PatchWorkThread( void* pWorkContext )
{
	if( 0 == pWorkContext )
		return -1;

	MUpdater* pUpdater = reinterpret_cast< MUpdater* >( pWorkContext );
	if( !pUpdater->CopyPatchFileToPatchRootDir() )
	{
		return -1;
	}

	return 0;
}