/******************************************************************
   
   ZFileTransfer.cpp

     Corperation : MAIET entertainment
     Programmer  : Lim Dong Hwan
	 Date        : 22.June.2005

*******************************************************************/


#include "stdafx.h"
#include "ZFileTransfer.h"
#include "FileInfo.h"
#include "Global.h"



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
ZFileTransfer::ZFileTransfer() : m_INetSession( "ZUpdate")
{
	// 변수 초기화
	m_bInitialize = false;
	m_szAddress[ 0] = 0;
	m_nPort = 0;
	m_szID[ 0] = 0;
	m_szPassword[ 0] = 0;

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
	PutDebugMSG( "[ZFileTransfer] Create.");


	// Set variables
	strcpy( m_szAddress, pszAddress);
	m_nPort = nPort;
	strcpy( m_szID, pszID);
	strcpy( m_szPassword, pszPassword);


	// Success
	m_bInitialize = true;
	PutDebugMSG( "[ZFileTransfer] Create successfuly complete.");

	return true;
}


// Destroy
bool ZFileTransfer::Destroy()
{
	PutDebugMSG( "[ZFileTransfer] Destroy.");


	// Check open connection
	if ( m_bOpenConnection)
	{
		if ( !CloseConnection())
			PutDebugMSG( "[ZFileTransfer] WARNING : Destroy failed.");
	}


	// Set variables
	m_szAddress[ 0] = 0;
	m_nPort = 0;
	m_szID[ 0] = 0;
	m_szPassword[ 0] = 0;

	// Success
	m_bInitialize = false;
	PutDebugMSG( "[ZFileTransfer] Destroy successfly complete.");

	return true;
}


// OpenConnection
bool ZFileTransfer::OpenConnection()
{
	PutDebugMSG( "[ZFileTransfer] Open connection.");


	// Check open connection
	if ( m_bOpenConnection)
	{
		PutDebugMSG( "[ZFileTransfer] ERROR : Already open connection.");
		PutDebugMSG( "[ZFileTransfer] ERROR : Open connection failed.");
		return false;
	}


	// Open connection
	CFtpConnection* pFtpConnection = NULL;
	TRY
	{
		pFtpConnection = m_INetSession.GetFtpConnection( m_szAddress, m_szID, m_szPassword, (INTERNET_PORT)m_nPort);
		if ( pFtpConnection != NULL)
		{
			// Success
			m_pConnection = pFtpConnection;
		}
	}
	CATCH( CInternetException, e)
	{
		TCHAR szError[256];
		e->GetErrorMessage( szError, 256);

		if ( pFtpConnection)
		{
			pFtpConnection->Close();
			delete pFtpConnection;
		}

		PutDebugMSG( "[ZFileTransfer] ERROR : Open connection failed.");

		return false;
	}
	END_CATCH


	// Success
	m_bOpenConnection = true;
	PutDebugMSG( "[ZFileTransfer] Open connection successfuly comlete.");

	return true;
}


// CloseConnection
bool ZFileTransfer::CloseConnection()
{
	PutDebugMSG( "[ZFileTransfer] Close connection.");


	// Check open connection
	if ( !m_bOpenConnection)
	{
		PutDebugMSG( "[ZFileTransfer] WARNING : Connection are not opened.");
		PutDebugMSG( "[ZFileTransfer] WARNING : Close connection failed.");

		return false;
	}


	// Close connection
	GetConnection()->Close();
	m_pConnection = NULL;


	// Success
	m_bOpenConnection = false;
	PutDebugMSG( "[ZFileTransfer] Close connection successfuly complete.");

	return true;
}


// DownloadFile
bool ZFileTransfer::DownloadFile( const char* pszRemoteFileName, const char* pszLocalFileName)
{
	CString str;
	str.Format( "[ZFileTransfer] Download File : %s  to  %s", pszRemoteFileName, pszLocalFileName);
	PutDebugMSG( str);


	// Set variables
	m_bStopDownload = false;


	// Check open connection
	if ( !m_bOpenConnection)
	{
		PutDebugMSG( "[ZFileTransfer] ERROR : Connection are not opened.");
		str.Format( "[ZFileTransfer] ERROR : Download File : %s failed.", pszRemoteFileName);
		PutDebugMSG( str);

		return false;
	}


	// MakePath
	if ( !MakePath( pszLocalFileName))
	{
		PutDebugMSG( "[ZFileTransfer] WARNNING : This file cannot made path.");
	}


	// Prepare Local File
	CFileException ex;
	CFile LocalFile;
	if (!LocalFile.Open( pszLocalFileName, CFile::modeCreate | CFile::modeWrite, &ex))
	{
		TCHAR szError[ 1024];
		ex.GetErrorMessage( szError, 1024);

		str.Format( "[ZFileTransfer] ERROR : Cannot open local file (%s).", szError);
		PutDebugMSG( str);
		str.Format( "[ZFileTransfer] ERROR : Download File : %s failed.", pszRemoteFileName);
		PutDebugMSG( str);

		return false;
	}


	// Prepare Remote File
	DWORD dwActualFileLength = 0;
	CFtpFileFind finder( GetConnection());
	if ( !finder.FindFile( pszRemoteFileName))
	{
		PutDebugMSG( "[ZFileTransfer] ERROR : Cannot open remote file.");
		str.Format( "[ZFileTransfer] ERROR : Download File : %s failed.", pszRemoteFileName);
		PutDebugMSG( str);

		LocalFile.Close();

		return false;
	}
	finder.FindNextFile();
	dwActualFileLength = (DWORD)finder.GetLength();
	finder.Close();


	// Open internet file
	CInternetFile* pINetFile = GetConnection()->OpenFile( pszRemoteFileName, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY);
	if ( !pINetFile)
	{
		PutDebugMSG( "[ZFileTransfer] ERROR : Cannot open internet file.");
		str.Format( "[ZFileTransfer] ERROR : Download File : %s failed.", pszRemoteFileName);
		PutDebugMSG( str);

		LocalFile.Close();

		return false;
	}


	// Download file
	m_FileTransferUI.SetDownloadFileName( pszRemoteFileName);
	m_FileTransferUI.SetTotalDownloadSize( dwActualFileLength);
	m_FileTransferUI.ClearCurrDownloadSize();
	#define MBUF_SIZE	4096
	BYTE Buffer[ MBUF_SIZE];
	DWORD dwRead = 0;
	DWORD dwTotalRead = 0;
	do
	{
		try
		{
			dwRead = pINetFile->Read( Buffer, MBUF_SIZE);
			LocalFile.Write( Buffer, dwRead);

			dwTotalRead += dwRead;
			
			m_FileTransferUI.AddCurrDownloadSize( dwRead);
		}
		catch ( CInternetException* pEx)
		{
			TCHAR szError[ 1024];
			pEx->GetErrorMessage( szError, 1024);
			pEx->Delete();
		
			str.Format( "[ZFileTransfer] ERROR : Cannot download file (%s).", szError);
			PutDebugMSG( str);
			str.Format( "[ZFileTransfer] Download File : %s successfuly compete.", pszRemoteFileName);

			LocalFile.Close();
			pINetFile->Close();

			return false;
		}

	} while ( (dwRead > 0) && !m_bStopDownload);


	// Stop download
	if ( m_bStopDownload)
	{
		str.Format( "[ZFileTransfer] Download File : %s STOP!!!", pszRemoteFileName);
		m_bStopDownload = false;

		LocalFile.Close();
		pINetFile->Close();

		DeleteFile( pszLocalFileName);			// Delete file

		return false;
	}


	// Download complete
	str.Format( "[ZFileTransfer] Download File : %s successfuly compete.", pszRemoteFileName);
	PutDebugMSG( str);

	LocalFile.Close();
	pINetFile->Close();

	m_FileTransferUI.ClearDownloadFileName();
	m_FileTransferUI.ClearCurrDownloadSize();
	m_FileTransferUI.SetTotalDownloadSize( 0);

	return true;
}


// ChangeDirectory
bool ZFileTransfer::ChangeDirectory( const char* pszDirectory)
{
	PutDebugMSG( "[ZFileTransfer] Change directory.");


	// Check open connection
	if ( !m_bOpenConnection)
	{
		PutDebugMSG( "[ZFileTransfer] ERROR : Connection are not opened.");
		PutDebugMSG( "[ZFileTransfer] ERROR : Change directory failed.");

		return false;
	}


	// Change directory
	CString str;
	if ( !GetConnection()->SetCurrentDirectory( pszDirectory))
	{
		str.Format( "[ZFileTransfer] ERROR : Cannot change the directory : %s", pszDirectory);
		PutDebugMSG( str);
		PutDebugMSG( "[ZFileTransfer] ERROR : Change directory failed.");

		return false;
	}


	// Success
	str.Format( "[ZFileTransfer] Change directory successfuly complete. : %s", pszDirectory);
	PutDebugMSG( str);

	return true;
}
