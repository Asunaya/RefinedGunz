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
#include "Global.h"
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


	// 파일 유무 검사
	WIN32_FIND_DATA FindData;
	HANDLE hFind;
	if( (hFind = FindFirstFile( GetFileName(), &FindData )) == INVALID_HANDLE_VALUE)
	{
		FindClose( hFind );

		char szMsg[ 512];
		sprintf( szMsg, "[ZUpdatePatchNode] Check Valid : %s, Remote( %u ), Local( X ), UPDATE!", GetFileName(), GetChecksum());
		PutDebugMSG( szMsg);

		return false;
	}

	
	// 파일 확장명 검사
	DWORD dwCRC = 0;
	char szFileExtName[25];
	strcpy( szFileExtName, FindData.cFileName + ( strlen( FindData.cFileName) - 3));

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
		if ( fp != NULL)
		{
			MZip zMRSFile;
			if ( zMRSFile.Initialize( fp, MZIPREADFLAG_MRS))
			{
				for ( int i = 0;  i < zMRSFile.GetFileCount();  i++)
					dwCRC += zMRSFile.GetFileCRC32( i);
			}
		}
		else
		{
			FindClose( hFind);
	
			PutDebugMSG( "[ZUpdatePatchNode] ERROR : Cannot open *.mrs file.");
			
			return false;
		}
		fclose( fp);
	}

	// *.pdb파일은 m_bValidate 값을 true로 해서 업데이트 체크할때 그냥 통과하게 한다.
	else if ( !stricmp( szFileExtName, "pdb"))
	{
		m_bValidate = true;

		FindClose( hFind );

		return true;
	}


	// Handle close
	FindClose( hFind);


	// CheckSum 비교
	if ( dwCRC == GetChecksum())
		m_bValidate = true;


	// Success
	char szMsg[ 512];
	sprintf( szMsg, "[ZUpdatePatchNode] Check Valid : %s, Remote( %u ), Local( %u ), %s", GetFileName(), GetChecksum(), dwCRC, ((m_bValidate) ? "PASS":"UPDATE!"));
	PutDebugMSG( szMsg);

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
	PutDebugMSG( "[ZUpdate] Create.");

	// Set variables
	strcpy( m_szAddress, pszAddress);
	m_nPort = nPort;
	strcpy( m_szID, pszID);
	strcpy( m_szPassword, pszPassword);


	// Create File Transfer
	if ( !m_FileTransfer.Create( pszAddress, nPort, pszID, pszPassword))
	{
		PutDebugMSG( "[ZUpdate] ERROR : Cannot create file transfer.");
		PutDebugMSG( "[ZUpdate] ERROR : Create failed.");

		return false;
	}


	// Create Open Connection
	if ( !m_FileTransfer.OpenConnection())
	{
		PutDebugMSG( "[ZUpdate] ERROR : Cannot open file transfer.");
		PutDebugMSG( "[ZUpdate] ERROR : Create failed.");

		return false;
	}


	// Set default directory
	if ( !m_FileTransfer.ChangeDirectory( pszDefDirectory))
	{
		PutDebugMSG( "[ZUpdate] ERROR : Cannot change default directory.");
		PutDebugMSG( "[ZUpdate] ERROR : Create failed.");

		return false;
	}


	// Success
	m_UpdateInfo.SetTransferUI( m_FileTransfer.GetFileTransferUI());
	m_bInitialize = true;
	PutDebugMSG( "[ZUpdate] Create successfuly compete.");

	return true;
}


// Destroy
bool ZUpdate::Destroy()
{
	PutDebugMSG( "[ZUpdate] Destroy.");

	// Close Connection
	if ( !m_FileTransfer.CloseConnection())
	{
		PutDebugMSG( "[ZUpdate] WARNING : Destroy failed.");
	}

	// Clear varialbes
	m_szAddress[ 0] = 0;
	m_nPort = 0;
	m_szID[ 0] = 0;
	m_szPassword[ 0] = 0;
	m_bInitialize = false;
	m_pUpdatePatchList.clear();

    
	// Success
	PutDebugMSG( "[ZUpdate] Destroy successfuly complete.");

	return true;
}


// StartUpdate
bool ZUpdate::StartUpdate( const char* pszPatchFileName)
{
	PutDebugMSG( "[ZUpdate] Start update.");

	
	// Set variables
	m_bStopUpdate = false;
	m_bPatchComplete = false;
	m_UpdateInfo.ClearTotalPatchFileSize();
	m_UpdateInfo.ClearCurrPatchedFileSize();
	m_UpdateInfo.ClearTotalPatchFileCount();
	m_UpdateInfo.ClearCurrPatchedFileCount();
	m_UpdateInfo.ClearPatchFailedCount();

	
	// Download 'patch.xml' file
	if ( !m_FileTransfer.DownloadFile( pszPatchFileName, pszPatchFileName))
	{
		PutDebugMSG( "[ZUpdate] ERROR : Start update failed.");
		return false;
	}


	// Get update info
	if ( !GetUpdateInfo( pszPatchFileName))
	{
		PutDebugMSG( "[ZUpdate] ERROR : Start update failed.");
		return false;
	}


	// Update files
	if ( !CheckValidFromPatchList())
	{
		PutDebugMSG( "[ZUpdate] ERROR : Start update failed.");
		return false;
	}


	// Patch files
	if ( !PatchFiles())
	{
		PutDebugMSG( "[ZUpdate] ERROR : Start update failed.");
		return false;
	}


	// Update complete
	PutDebugMSG( "[ZUpdate] Start update successfuly complete.");

	m_bPatchComplete = true;

	return true;
}


// StopUpdate
bool ZUpdate::StopUpdate()
{
	PutDebugMSG( "[ZUpdate] Stop update.");


	// Stop file transfer
	m_FileTransfer.StopDownload();
	m_bStopUpdate = true;


	// Update complete
	PutDebugMSG( "[ZUpdate] Stop update successfuly complete.");
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
	PutDebugMSG( "[ZUpdate] Get update info.");


	// Clear patch list
	m_pUpdatePatchList.clear();


	// Read patch file
	MXmlDocument	xmlConfig;
	MXmlElement		rootElement, aPatchInfo;

	xmlConfig.Create();
	if ( !xmlConfig.LoadFromFile( pszPatchFileName)) 
	{
		xmlConfig.Destroy();
		PutDebugMSG( "[ZUpdate] ERROR : Get update info failed.");

		return false;
	}

	rootElement = xmlConfig.GetDocumentElement();
	if ( rootElement.IsEmpty())
	{
		xmlConfig.Destroy();
		PutDebugMSG( "[ZUpdate] ERROR : Get update info failed.");

		return false;
	}

	if ( rootElement.FindChildNode( MPTOK_PATCHINFO, &aPatchInfo) == false)
	{
		xmlConfig.Destroy();
		PutDebugMSG( "[ZUpdate] ERROR : Get update info failed.");

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
			if ( aPatchNode.GetChildContents( (int*)&tmWrite.dwHighDateTime, MPTOK_WRITETIMEHIGH) == false)
				continue;

			if ( aPatchNode.GetChildContents( (int*)&tmWrite.dwLowDateTime, MPTOK_WRITETIMELOW) == false)
				continue;

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
	PutDebugMSG( "[ZUpdate] Get update info successfuly complete.");

	return true;
}


// CheckValidFromPatchList
bool ZUpdate::CheckValidFromPatchList()
{
	PutDebugMSG( "[ZUpdate] Check valid from patch list.");


	// Get valid
	for ( ZUpdatePatchList::iterator itr = m_pUpdatePatchList.begin();  itr != m_pUpdatePatchList.end();  itr++)
	{
		(*itr)->CheckValid();

		if ( m_bStopUpdate)
		{
			PutDebugMSG( "[ZUpdate] ERROR : Check valid STOPED!!!");
			PutDebugMSG( "[ZUpdate] ERROR : Check valid failed.");

			return false;
		}
	}


	// 예외 파일 처리
	#define  GUNZEXE_FILENAME		"Gunz.exe"
	#define  GUNZPDB_FILENAME		"Gunz.pdb"
	ZUpdatePatchList::iterator itrExeFile, itrPdbFile;
	for ( ZUpdatePatchList::iterator itr = m_pUpdatePatchList.begin();  itr != m_pUpdatePatchList.end();  itr++)
	{
		ZUpdatePatchNode* pNode = *itr;

		if ( !stricmp( pNode->GetFileName() + ( strlen(pNode->GetFileName()) - strlen(GUNZEXE_FILENAME)), GUNZEXE_FILENAME))
			itrExeFile = itr;

		if ( !stricmp( pNode->GetFileName() + ( strlen(pNode->GetFileName()) - strlen(GUNZPDB_FILENAME)), GUNZPDB_FILENAME))
			itrPdbFile = itr;
	}
	if ( (*itrPdbFile)->IsValid())
	{
		if ( !(*itrExeFile)->IsValid())
			(*itrPdbFile)->SetValid( false);
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
	PutDebugMSG( "[ZUpdate] Check valid from patch list successfuly complete.");
	CString str;
	str.Format( "[ZUpdate] + Total patch file size : %u bytes", m_UpdateInfo.GetTotalPatchFileSize());
	PutDebugMSG( str);
	str.Format( "[ZUpdate] + Total patch file count : %u", m_UpdateInfo.GetTotalPatchFileCount());
	PutDebugMSG( str);

	return true;
}


// PatchFiles
bool ZUpdate::PatchFiles()
{
	PutDebugMSG( "[ZUpdate] Patch files.");


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
				szFullPath[ strlen( szFullPath) - 1] = '_';

            
			// Start download
			#define DOWNLOAD_RETRY_COUNT	2
			for ( int nRetry = 0;  nRetry < DOWNLOAD_RETRY_COUNT;  nRetry++)
			{
				if ( m_bStopUpdate)
					break;


				if ( m_FileTransfer.DownloadFile( (*itr)->GetFileName(), szFullPath))
				{
					(*itr)->SetValid( true);

					sprintf( szMsg, "[ZUpdate] Patch files : %s", (*itr)->GetFileName());
					PutDebugMSG( szMsg);

					break;
				}
				else if ( nRetry == (DOWNLOAD_RETRY_COUNT - 1))
				{
					sprintf( szMsg, "[ZUpdate] ERROR : Patch files : %s FAIL!!!", (*itr)->GetFileName());
					PutDebugMSG( szMsg);
				}
				else
				{
					m_UpdateInfo.AddPatchFailedCount( 1);

					sprintf( szMsg, "[ZUpdate] WARNNING : Retry patch files : %s", (*itr)->GetFileName());
					PutDebugMSG( szMsg);
				}
			}


			// Update information
			m_UpdateInfo.AddCurrPatchedFileSize( (*itr)->GetSize());
			m_UpdateInfo.AddCurrPatchedFileCount( 1);
		}


		// Check stop
		if ( m_bStopUpdate)
		{
			PutDebugMSG( "[ZUpdate] Patch files STOP!!!");

			return false;
		}
	}


    // Success
	PutDebugMSG( "[ZUpdate] Patch files successfuly complete.");

	return true;
}