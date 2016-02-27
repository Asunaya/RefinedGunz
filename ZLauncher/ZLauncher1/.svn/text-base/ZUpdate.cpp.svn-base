/******************************************************************
   
   ZUpdate.cpp

     Corperation : MAIET entertainment
     Programmer  : Lim Dong Hwan
	 Date        : 22.June.2005

*******************************************************************/


#include "stdafx.h"
#include "ZUpdate.h"
#include "MXml.h"
#include "FileInfo.h"
#include "MZip.h"
#include "Log.h"
#include <shlwapi.h>


// class ZUpdatePatchNode

// 생성자
ZUpdatePatchNode::ZUpdatePatchNode( const char* pszName, unsigned long nSize, unsigned long nChecksum)
{
	strcpy( m_szFileName, pszName);
	m_nSize = nSize;
	m_nCheckSum = nChecksum;
	m_bValidate = false;
}


// 소멸자
ZUpdatePatchNode::~ZUpdatePatchNode()
{
}


// CheckValid
bool ZUpdatePatchNode::CheckValid()
{
	m_bValidate = false;


	// 파일 확장명 검사
	char szFileExtName[25];
	strcpy( szFileExtName, GetFileName() + ( strlen( GetFileName()) - 3));


	// 예외 규정
	if ( !stricmp( szFileExtName, "pdb"))
	{
		m_bValidate = true;

		char szMsg[ 512];
		sprintf( szMsg, "[ZUpdatePatchNode] Check Valid : %s, Remote( %u ), Local( ? )", GetFileName(), GetChecksum());
		PutLog( szMsg);

		return true;
	}


	// 파일 유무 검사
	WIN32_FIND_DATA FindData;
	HANDLE hFind;
	if( (hFind = FindFirstFile( GetFileName(), &FindData )) == INVALID_HANDLE_VALUE)
	{
		FindClose( hFind );

		char szMsg[ 512];
		sprintf( szMsg, "[ZUpdatePatchNode] Check Valid : %s, Remote( %u ), Local( X ), UPDATE!", GetFileName(), GetChecksum());
		PutLog( szMsg, LOG_HIGHLIGHT);

		return false;
	}

	
	DWORD dwCRC = 0;

	// *.exe, *.dll, *.txt, *.dat, *.vso, *.xml 파일은 파일 전체를 읽어서 CheckSum을 구한다
	if ( !stricmp( szFileExtName, "exe") ||
			!stricmp( szFileExtName, "dll") ||
			!stricmp( szFileExtName, "txt") ||
			!stricmp( szFileExtName, "dat") ||
			!stricmp( szFileExtName, "vso") ||
			!stricmp( szFileExtName, "xml"))
	{
		dwCRC = GetFileCheckSum( (char*)GetFileName());
	}

	// *.mrs파일은 파일 헤더를 읽어서 CheckSum을 구한다
	else if ( !stricmp( szFileExtName, "mrs"))
	{
		FILE* fp = fopen( GetFileName(), "rb");
		if ( fp == NULL)
		{
			FindClose( hFind);
	
			PutLog( "[ZUpdatePatchNode] ERROR : Cannot open *.mrs file.", LOG_ERROR);
			
			return false;
		}


		MZip zMRSFile;
		unsigned long dwReadMode = MZIPREADFLAG_MRS2;
		if ( !zMRSFile.Initialize( fp , dwReadMode ))
		{
			fclose( fp);

			DeleteFile( GetFileName());

		}
		else
		{
			for ( int i = 0;  i < zMRSFile.GetFileCount();  i++)
				dwCRC += zMRSFile.GetFileCRC32( i);

			fclose( fp);
		}
	}


	// Handle close
	FindClose( hFind);


	// CheckSum 비교
	if ( dwCRC == GetChecksum())
		m_bValidate = true;


	// Success
	char szMsg[ 512];
	sprintf( szMsg, "[ZUpdatePatchNode] Check Valid : %s, Remote( %u ), Local( %u ), %s", GetFileName(), GetChecksum(), dwCRC, ((m_bValidate) ? "PASS":"UPDATE!"));
	if ( m_bValidate)
		PutLog( szMsg);
	else
		PutLog( szMsg, LOG_HIGHLIGHT);

	return true;
}





// class ZUpdate

// 생성자
ZUpdate::ZUpdate()
{
	// 변수 초기화
	m_bInitialize = false;
	m_szAddress[ 0] = 0;
	m_nPort = 0;
	m_szID[ 0] = 0;
	m_szPassword[ 0] = 0;
	m_pUpdatePatchList.clear();
	m_bStopUpdate = false;
	m_bPatchComplete = false;
	m_szLastError[ 0] = 0;
}


// 소멸자
ZUpdate::~ZUpdate()
{
	if ( m_bInitialize)
		Destroy();
}


// Create
bool ZUpdate::Create( const char* pszAddress, unsigned long nPort, const char* pszDefDirectory, const char* pszID, const char* pszPassword)
{
	PutLog( "[ZUpdate] Create.");

	// Set variables
	strcpy( m_szAddress, pszAddress);
	m_nPort = nPort;
	strcpy( m_szID, pszID);
	strcpy( m_szPassword, pszPassword);


	// Create File Transfer
	if ( !m_FileTransfer.Create( pszAddress, nPort, pszID, pszPassword))
	{
		sprintf( m_szLastError, "[ TIP ]\nThe update server is not responding or is not running right now. Please try again after a while. Sorry for your inconvenience.");

		PutLog( "[ZUpdate] ERROR : Cannot create file transfer.", LOG_ERROR);
		PutLog( "[ZUpdate] ERROR : Create FAILED!!!", LOG_ERROR);

		return false;
	}


	// Create Open Connection
	if ( !m_FileTransfer.OpenConnection())
	{
		sprintf( m_szLastError, "[ TIP ]\nThe update server is not responding or is not running right now. Please try again after a while. Sorry for your inconvenience.");

		PutLog( "[ZUpdate] ERROR : Cannot open file transfer.", LOG_ERROR);
		PutLog( "[ZUpdate] ERROR : Create FAILED!!!", LOG_ERROR);

		return false;
	}


	// Set default directory
	if ( !m_FileTransfer.ChangeDirectory( pszDefDirectory))
	{
		sprintf( m_szLastError, "[ TIP ]\nThe update server is not responding or is not running right now. Please try again after a while. Sorry for your inconvenience.");

		PutLog( "[ZUpdate] ERROR : Cannot change default directory.", LOG_ERROR);
		PutLog( "[ZUpdate] ERROR : Create FAILED!!!", LOG_ERROR);

		return false;
	}


	// Success
	m_UpdateInfo.SetTransferUI( m_FileTransfer.GetFileTransferUI());
	m_bInitialize = true;
	PutLog( "[ZUpdate] Create successfuly compete.");

	return true;
}


// Destroy
bool ZUpdate::Destroy()
{
	PutLog( "[ZUpdate] Destroy.");

	// Close Connection
	if ( !m_FileTransfer.Destroy())
	{
		PutLog( "[ZUpdate] WARNING : Destroy FAILED!!!");
	}

	// Clear varialbes
	m_szAddress[ 0] = 0;
	m_nPort = 0;
	m_szID[ 0] = 0;
	m_szPassword[ 0] = 0;
	m_bInitialize = false;
	m_pUpdatePatchList.clear();

    
	// Success
	PutLog( "[ZUpdate] Destroy successfuly complete.");

	return true;
}


// StartUpdate
bool ZUpdate::StartUpdate( const char* pszPatchFileName)
{
	// Check Initialized
	if ( !m_bInitialize)
	{
		PutLog( "[ZUpdate] ERROR : Do not Created.", LOG_ERROR);
		PutLog( "[ZUpdate] ERROR : Start update FAILED!!!", LOG_ERROR);

		return false;
	}
	PutLog( "[ZUpdate] Start update.");

	
	// Set variables
	m_bStopUpdate = false;
	m_bPatchComplete = false;
	m_szLastError[ 0] = 0;
	m_UpdateInfo.ClearTotalPatchFileSize();
	m_UpdateInfo.ClearCurrPatchedFileSize();
	m_UpdateInfo.ClearTotalPatchFileCount();
	m_UpdateInfo.ClearCurrPatchedFileCount();
	m_UpdateInfo.ClearPatchFailedCount();

	
	// Download patch info file
	if ( !m_FileTransfer.DownloadFile( pszPatchFileName, pszPatchFileName))
	{
		sprintf( m_szLastError, "[ TIP ]\nThe update server is not responding or is not running right now. Please try again after a while. Sorry for your inconvenience.");

		PutLog( "[ZUpdate] ERROR : Start update FAILED!!!", LOG_ERROR);
		return false;
	}


	// Get update info
	if ( !GetUpdateInfo( pszPatchFileName))
	{
		PutLog( "[ZUpdate] ERROR : Start update FAILED!!!", LOG_ERROR);
		return false;
	}


	// Update files
	if ( !CheckValidFromPatchList())
	{
		PutLog( "[ZUpdate] ERROR : Start update FAILED!!!", LOG_ERROR);
		return false;
	}


	// Patch files
	if ( !PatchFiles())
	{
		PutLog( "[ZUpdate] ERROR : Start update FAILED!!!", LOG_ERROR);
		return false;
	}


	// Update complete
	PutLog( "[ZUpdate] Start update successfuly complete.");

	m_bPatchComplete = true;

	return true;
}


// StopUpdate
bool ZUpdate::StopUpdate()
{
	PutLog( "[ZUpdate] Stop update.", LOG_HIGHLIGHT);


	// Stop file transfer
	m_FileTransfer.StopDownload();
	m_bStopUpdate = true;


	// Update complete
	PutLog( "[ZUpdate] Stop update successfuly complete.");
	return true;
}


// GetUpdatePatchFileInfo
#define MPTOK_PATCHINFO		"PATCHINFO"
#define MPTOK_VERSION		"VERSION"
#define MPTOK_PATCHNODE		"PATCHNODE"
#define MPTOK_SIZE			"SIZE"
#define MPTOK_WRITETIMEHIGH	"WRITETIMEHIGH"
#define MPTOK_WRITETIMELOW	"WRITETIMELOW"
#define MPTOK_CHECKSUM		"CHECKSUM"
#define MPTOK_ATTR_FILE		"file"

bool ZUpdate::GetUpdateInfo( const char* pszPatchFileName)
{
	PutLog( "[ZUpdate] Get update info.");


	// Clear patch list
	m_pUpdatePatchList.clear();


	// Read patch file
	MXmlDocument	xmlConfig;
	MXmlElement		rootElement, aPatchInfo;

	xmlConfig.Create();
	if ( !xmlConfig.LoadFromFile( pszPatchFileName)) 
	{
		xmlConfig.Destroy();
		PutLog( "[ZUpdate] ERROR : Get update info FAILED!!!", LOG_ERROR);

		return false;
	}

	rootElement = xmlConfig.GetDocumentElement();
	if ( rootElement.IsEmpty())
	{
		xmlConfig.Destroy();
		PutLog( "[ZUpdate] ERROR : Get update info FAILED!!!", LOG_ERROR);

		return false;
	}

	if ( rootElement.FindChildNode( MPTOK_PATCHINFO, &aPatchInfo) == false)
	{
		xmlConfig.Destroy();
		PutLog( "[ZUpdate] ERROR : Get update info FAILED!!!", LOG_ERROR);

		return false;
	}

	int iCount = aPatchInfo.GetChildNodeCount();
	for ( int i = 0;  i < iCount;  i++)
	{
		MXmlElement aPatchNode = aPatchInfo.GetChildNode(i);
		
		char szBuf[256];
		aPatchNode.GetTagName(szBuf);
		if (szBuf[0] == '#') continue;

		if ( !stricmp(szBuf, MPTOK_PATCHNODE))
		{
			char szFileName[_MAX_DIR] = "";
			int nSize = 0;
			FILETIME tmWrite;
			int nCheckSum = 0;

			// File
			if ( aPatchNode.GetAttribute( szFileName, MPTOK_ATTR_FILE) == false)
				continue;

			// Size
			if ( aPatchNode.GetChildContents( &nSize, MPTOK_SIZE) == false)
				continue;

			// Date
//			if ( aPatchNode.GetChildContents( (int*)&tmWrite.dwHighDateTime, MPTOK_WRITETIMEHIGH) == false)
//				continue;

//			if ( aPatchNode.GetChildContents( (int*)&tmWrite.dwLowDateTime, MPTOK_WRITETIMELOW) == false)
//				continue;

			// Checksum
			if ( aPatchNode.GetChildContents( &nCheckSum, MPTOK_CHECKSUM) == false)
				continue;

			m_pUpdatePatchList.push_back( new ZUpdatePatchNode( szFileName, nSize, nCheckSum));
		}
		else if ( !stricmp( szBuf, MPTOK_VERSION))
		{
			char szVersion[ 128] = "";
			aPatchNode.GetContents( szVersion);

			// Get version here...
		}
	}
	xmlConfig.Destroy();


	// Success
	PutLog( "[ZUpdate] Get update info successfuly complete.");

	return true;
}


// CheckValidFromPatchList
bool ZUpdate::CheckValidFromPatchList()
{
	PutLog( "[ZUpdate] Check valid from patch list.");


	// Get valid
	for ( ZUpdatePatchList::iterator itr = m_pUpdatePatchList.begin();  itr != m_pUpdatePatchList.end();  itr++)
	{
		(*itr)->CheckValid();

		if ( m_bStopUpdate)
		{
			sprintf( m_szLastError, "[ TIP ]\nThe update must be completed before it runs.");

			PutLog( "[ZUpdate] ERROR : Check valid STOPED!!!", LOG_ERROR);
			PutLog( "[ZUpdate] ERROR : Check valid FAILED!!!", LOG_ERROR);

			return false;
		}
	}

// 다음 버젼에서 주석 처리 할 부분
	// 예외 파일 처리
	#define  GUNZEXE_FILENAME		"Gunz.exe"
	#define  GUNZPDB_FILENAME		"Gunz.pdb"
	int nFindCount = 0;
	ZUpdatePatchList::iterator itrExeFile, itrPdbFile;
	for ( ZUpdatePatchList::iterator itr = m_pUpdatePatchList.begin();  itr != m_pUpdatePatchList.end();  itr++)
	{
		ZUpdatePatchNode* pNode = *itr;

		if ( !stricmp( pNode->GetFileName() + ( strlen(pNode->GetFileName()) - strlen(GUNZEXE_FILENAME)), GUNZEXE_FILENAME))
		{
			itrExeFile = itr;
			nFindCount++;
		}

		if ( !stricmp( pNode->GetFileName() + ( strlen(pNode->GetFileName()) - strlen(GUNZPDB_FILENAME)), GUNZPDB_FILENAME))
		{
			itrPdbFile = itr;
			nFindCount++;
		}
	}
	if ( nFindCount == 2)
	{
		if ( (*itrPdbFile)->IsValid())
		{
			if ( !(*itrExeFile)->IsValid())
				(*itrPdbFile)->SetValid( false);
		}
	}


	// Get patch files information
	for ( ZUpdatePatchList::iterator itr = m_pUpdatePatchList.begin();  itr != m_pUpdatePatchList.end();  itr++)
	{
		if ( !(*itr)->IsValid())
		{
			m_UpdateInfo.AddTotalPatchFileSize( (*itr)->GetSize());
			m_UpdateInfo.AddTotalPatchFileCount( 1);
		}
	}


	// Success
	PutLog( "[ZUpdate] Check valid from patch list successfuly complete.");
	CString str;
	str.Format( "[ZUpdate] + Total patch file count : %u", m_UpdateInfo.GetTotalPatchFileCount());
	PutLog( str, LOG_HIGHLIGHT);
	str.Format( "[ZUpdate] + Total patch file size : %u bytes", m_UpdateInfo.GetTotalPatchFileSize());
	PutLog( str, LOG_HIGHLIGHT);

	return true;
}


// PatchFiles
bool ZUpdate::PatchFiles()
{
	PutLog( "[ZUpdate] Patch files.");


	// Get patch files information
	char szMsg[ 256];
	for ( ZUpdatePatchList::iterator itr = m_pUpdatePatchList.begin();  itr != m_pUpdatePatchList.end();  itr++)
	{
		if ( !(*itr)->IsValid())
		{
			// Download file
			TCHAR szFullPath[ _MAX_DIR];
			PathSearchAndQualify( (*itr)->GetFileName(), szFullPath, _MAX_DIR);


			// 예외 파일 처리 규정
			#define GUNZLAUNCHER_FILENAME	"GunzLauncher.exe"
			if ( !stricmp( (*itr)->GetFileName() + ( strlen( (*itr)->GetFileName()) - strlen(GUNZLAUNCHER_FILENAME)), GUNZLAUNCHER_FILENAME))
			{
				szFullPath[ strlen( szFullPath) - 1] = '_';
			}

            
			// Start download
			#define DOWNLOAD_RETRY_COUNT	2
			for ( int nRetry = 1;  nRetry <= DOWNLOAD_RETRY_COUNT;  nRetry++)
			{
				if ( m_bStopUpdate)
					break;


				if ( m_FileTransfer.DownloadFile( (*itr)->GetFileName(), szFullPath))
				{
					(*itr)->SetValid( true);

					sprintf( szMsg, "[ZUpdate] Patched file : %s", (*itr)->GetFileName());
					PutLog( szMsg);

					break;
				}
				// Fail
				else if ( nRetry == DOWNLOAD_RETRY_COUNT)
				{
					m_UpdateInfo.AddPatchFailedCount( 1);

					sprintf( m_szLastError, "[ TIP ]\nDelete file : %s", (*itr)->GetFileName());

					sprintf( szMsg, "[ZUpdate] ERROR : Patch file : %s FAILED!!!", (*itr)->GetFileName());
					PutLog( szMsg, LOG_ERROR);
				}
				// Retry
				else
				{
					sprintf( szMsg, "[ZUpdate] WARNNING : Retry patch file : %s , %d retry", (*itr)->GetFileName(), nRetry);
					PutLog( szMsg, LOG_WARNNING);
				}
			}


			// Update information
			m_UpdateInfo.AddCurrPatchedFileSize( (*itr)->GetSize());
			m_UpdateInfo.AddCurrPatchedFileCount( 1);
		}


		// Check stop
		if ( m_bStopUpdate)
		{
			sprintf( m_szLastError, "[ TIP ]\nThe update must be completed before it runs.");
	
			PutLog( "[ZUpdate] Patch files STOP!!!");

			return false;
		}
	}


    // Success
	PutLog( "[ZUpdate] Patch files successfuly complete.");

	return true;
}