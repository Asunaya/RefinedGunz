/******************************************************************
   
   ZFileTransfer.cpp

     Corperation : MAIET entertainment
     Programmer  : Lim Dong Hwan
	 Date        : 22.June.2005

*******************************************************************/


#include "stdafx.h"
#include "ZFileTransfer.h"
#include "FileInfo.h"
#include "Log.h"



// class for MFTSession
MFTSession::MFTSession(LPCTSTR pszAppName, int nMethod)
: CInternetSession(pszAppName, 1, nMethod)
{
	EnableStatusCallback(TRUE);
}

void MFTSession::OnStatusCallback(DWORD dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInfomration, DWORD dwStatusInformationLen)
{
}



// class for ZFileTransfer

// 생성자
ZFileTransfer::ZFileTransfer() : m_Session( "ZUpdate")
{
	// 변수 초기화
	m_bInitialize = false;
	m_szAddress[ 0] = 0;
	m_szDefDir[ 0]  = 0;
	m_nPort = 0;
	m_szID[ 0] = 0;
	m_szPassword[ 0] = 0;
	m_szLastError[0] = 0;

	m_pConnection = NULL;
	m_bOpenConnection = false;
	m_bStopDownload = false;
}


// 소멸자
ZFileTransfer::~ZFileTransfer()
{
	// Check open connection
	if ( m_bOpenConnection)
		CloseConnection();
}


// Create
bool ZFileTransfer::Create( const char* pszAddress, unsigned long nPort, const char* pszID, const char* pszPassword)
{
	PutLog( "[ZFileTransfer] Create.");


	// Set variables
	strcpy( m_szAddress, pszAddress);
	m_nPort = nPort;
	strcpy( m_szID, pszID);
	strcpy( m_szPassword, pszPassword);


	// Success
	m_bInitialize = true;
	PutLog( "[ZFileTransfer] Create successfuly complete.");

	return true;
}


// Destroy
bool ZFileTransfer::Destroy()
{
	PutLog( "[ZFileTransfer] Destroy.");


	// Check open connection
	if ( m_bOpenConnection)
	{
		if ( !CloseConnection())
			PutLog( "[ZFileTransfer] WARNING : Destroy FAILED!!!");
	}


	// Set variables
	m_szAddress[ 0] = 0;
	m_nPort = 0;
	m_szID[ 0] = 0;
	m_szPassword[ 0] = 0;

	// Success
	m_bInitialize = false;
	PutLog( "[ZFileTransfer] Destroy successfly complete.");

	return true;
}


// OpenConnection
bool ZFileTransfer::OpenConnection()
{
	PutLog( "[ZFileTransfer] Open connection.");


	// Check open connection
	if ( m_bOpenConnection)
	{
		PutLog( "[ZFileTransfer] ERROR : Already open connection.", LOG_ERROR);
		PutLog( "[ZFileTransfer] ERROR : Open connection FAILED!!!", LOG_ERROR);
		return false;
	}


	// Open connection
	CHttpConnection* pConnection = NULL;
	try
	{
		pConnection = m_Session.GetHttpConnection( m_szAddress, (INTERNET_PORT)m_nPort);
		if ( pConnection != NULL)
		{
			// Success
			m_pConnection = pConnection;
		}
	}
	// Error
	catch ( CInternetException* pEx)
	{
		pEx->GetErrorMessage( m_szLastError, 1024);
		pEx->Delete();

		if ( pConnection)
		{
			pConnection->Close();
			delete pConnection;
		}


		CString str;
		str.Format( "[ZFileTransfer] ERROR : %s", m_szLastError);
		PutLog( str, LOG_ERROR);

		return false;
	}


	// Success
	m_bOpenConnection = true;
	PutLog( "[ZFileTransfer] Open connection successfuly comlete.");

	return true;
}


// CloseConnection
bool ZFileTransfer::CloseConnection()
{
	PutLog( "[ZFileTransfer] Close connection.");


	// Check open connection
	if ( !m_bOpenConnection)
	{
		PutLog( "[ZFileTransfer] WARNING : Connection are not opened.");
		PutLog( "[ZFileTransfer] WARNING : Close connection FAILED!!!");

		return false;
	}


	// Close connection
	GetConnection()->Close();
	m_pConnection = NULL;


	// Success
	m_bOpenConnection = false;
	PutLog( "[ZFileTransfer] Close connection successfuly complete.");

	return true;
}


// DownloadFile
bool ZFileTransfer::DownloadFile( const char* pszRemoteFileName, const char* pszLocalFileName, DWORD nFileSize)
{
	CString str;
	str.Format( "[ZFileTransfer] Download File : %s  to  %s", pszRemoteFileName, pszLocalFileName);
	PutLog( str);

	
	// Set variables
	CHttpFile* pFile = NULL;
	CFile LocalFile;
	m_bStopDownload = false;


	// Check open connection
	if ( !m_bOpenConnection)
	{
		PutLog( "[ZFileTransfer] ERROR : Connection are not opened.", LOG_ERROR);
		str.Format( "[ZFileTransfer] ERROR : Download File : %s FAILED!!!", pszRemoteFileName);
		PutLog( str, LOG_ERROR);

		return false;
	}


	// MakePath
	if ( !MakePath( pszLocalFileName))
	{
		PutLog( "[ZFileTransfer] WARNNING : This file cannot made path.", LOG_WARNNING);
	}


	// Delete Loacal File
	if ( !DeleteFile( pszLocalFileName))
	{
		str.Format( "[ZFileTransfer] WARNNING : Cannot delete %s file.", pszLocalFileName);
		PutLog( str, LOG_WARNNING);
	}


	// Prepare Local File
	CFileException ex;
	if ( !LocalFile.Open( pszLocalFileName, CFile::modeCreate | CFile::modeWrite, &ex))
	{
		ex.GetErrorMessage( m_szLastError, 1024);

		sprintf( m_szLastError, "%s : Access denied or unknown error.\n", pszLocalFileName);
		for ( int i = (int)strlen( m_szLastError);  i >= 0;  i--)
		{
			if ( (m_szLastError[ i] == '/') || (m_szLastError[ i] == '\\'))
				break;
		}
		memcpy( m_szLastError, m_szLastError + (i + 1), (int)strlen( m_szLastError) - i);
		m_szLastError[ i + 1] = 0;

		str.Format( "[ZFileTransfer] ERROR : %s", m_szLastError);
		PutLog( str, LOG_ERROR);
		str.Format( "[ZFileTransfer] ERROR : Download File : %s FAILED!!!", pszRemoteFileName);
		PutLog( str, LOG_ERROR);


		// Delete file
		DeleteFile( pszLocalFileName);


		return false;
	}


	// Set full path name
	char szFullPathName[ 512];
	sprintf( szFullPathName, "/%s%s", m_szDefDir, pszRemoteFileName + 1);


	try
	{
		// Open request
		pFile = GetConnection()->OpenRequest( CHttpConnection::HTTP_VERB_GET, szFullPathName, NULL, 1, NULL, NULL, INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_NO_AUTO_REDIRECT);
		if ( !pFile)
		{
			str.Format( "[ZFileTransfer] ERROR : Cannot open remote file.");
			PutLog( str, LOG_ERROR);
			str.Format( "[ZFileTransfer] ERROR : Download File : %s FAILED!!!", pszRemoteFileName);
			PutLog( str, LOG_ERROR);


			// Delete file
			DeleteFile( pszLocalFileName);


			return false;
		}
		pFile->SendRequest();


		// if access was denied...
		bool bError = false;
		DWORD dwRet;
		pFile->QueryInfoStatusCode( dwRet);
		if ( dwRet == HTTP_STATUS_DENIED)
		{
			str.Format( "[ZFileTransfer] ERROR : Access denied.");
			PutLog( str, LOG_ERROR);
			sprintf( m_szLastError, "(%03u) Access denied.\n", dwRet);

			bError = true;
		}
		else if ( dwRet == HTTP_STATUS_NOT_FOUND)
		{
			str.Format( "[ZFileTransfer] ERROR : File not found.");
			PutLog( str, LOG_ERROR);
			sprintf( m_szLastError, "(%03u) File not found.\n", dwRet);

			bError = true;
		}
		else if ( (dwRet >= 300) && (dwRet < 400))
		{
			str.Format( "[ZFileTransfer] ERROR : Information error.");
			PutLog( str, LOG_ERROR);
			sprintf( m_szLastError, "(%03u) Information error.\n", dwRet);

			bError = true;
		}
		else if ( (dwRet >= 400) && (dwRet < 500))
		{
			str.Format( "[ZFileTransfer] ERROR : File request error.");
			PutLog( str, LOG_ERROR);
			sprintf( m_szLastError, "(%03u) File request error.\n", dwRet);

			bError = true;
		}
		else if ( (dwRet >= 500) && (dwRet < 600))
		{
			str.Format( "[ZFileTransfer] ERROR : Server error.");
			PutLog( str, LOG_ERROR);
			sprintf( m_szLastError, "(%03u) Server error.\n", dwRet);

			bError = true;
		}


		// Check error
		if ( bError)
		{
			str.Format( "[ZFileTransfer] ERROR : Download File : %s FAILED!!!", pszRemoteFileName);
			PutLog( str, LOG_ERROR);

			delete pFile;


			// Delete file
			DeleteFile( pszLocalFileName);


			return false;
		}


		// Download file
		m_FileTransferUI.SetDownloadFileName( pszRemoteFileName);
		m_FileTransferUI.SetTotalDownloadSize( nFileSize);
		m_FileTransferUI.ClearCurrDownloadSize();
		#define MBUF_SIZE	2048
		BYTE Buffer[ MBUF_SIZE];
		DWORD dwRead = 0;
		DWORD dwTotalRead = 0;
		do
		{
			dwRead = pFile->Read( Buffer, MBUF_SIZE);
			LocalFile.Write( Buffer, dwRead);

			dwTotalRead += dwRead;
			
			m_FileTransferUI.AddCurrDownloadSize( dwRead);

		} while ( (dwRead > 0) && !m_bStopDownload);
	}
	// Error
	catch ( CInternetException* pEx)
	{
		pEx->GetErrorMessage( m_szLastError, 1024);
		pEx->Delete();

		str.Format( "[ZFileTransfer] ERROR : %s", m_szLastError);
		PutLog( str, LOG_ERROR);

		LocalFile.Close();
		pFile->Close();

		DeleteFile( pszLocalFileName);			// Delete file


		return false;
	}



	// Stop download
	if ( m_bStopDownload)
	{
		str.Format( "[ZFileTransfer] Download File : %s STOP!!!", pszRemoteFileName);
		PutLog( str, LOG_WARNNING);
		m_bStopDownload = false;

		LocalFile.Close();
		pFile->Close();

		DeleteFile( pszLocalFileName);			// Delete file

		return false;
	}


	// Download complete
	str.Format( "[ZFileTransfer] Download File : %s successfuly complete.", pszRemoteFileName);
	PutLog( str);

	LocalFile.Close();
	pFile->Close();

	m_FileTransferUI.ClearDownloadFileName();
	m_FileTransferUI.ClearCurrDownloadSize();
	m_FileTransferUI.SetTotalDownloadSize( 0);

	return true;
}


// ChangeDirectory
bool ZFileTransfer::ChangeDirectory( const char* pszDirectory)
{
	PutLog( "[ZFileTransfer] Change directory.");


	// Check open connection
	if ( !m_bOpenConnection)
	{
		PutLog( "[ZFileTransfer] ERROR : Connection are not opened.", LOG_ERROR);
		PutLog( "[ZFileTransfer] ERROR : Change directory FAILED!!!", LOG_ERROR);

		return false;
	}


	// Change directory
	strcpy( m_szDefDir, pszDirectory);


	// Success
	CString str;
	str.Format( "[ZFileTransfer] Change directory successfuly complete. : %s", pszDirectory);
	PutLog( str);

	return true;
}
