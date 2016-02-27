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

		PutLog( "[ZFileTransfer] ERROR : Open connection FAILED!!!", LOG_ERROR);

		return false;
	}
	END_CATCH


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
bool ZFileTransfer::DownloadFile( const char* pszRemoteFileName, const char* pszLocalFileName)
{
	CString str;
	str.Format( "[ZFileTransfer] Download File : %s  to  %s", pszRemoteFileName, pszLocalFileName);
	PutLog( str);


	// Set variables
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
	CFile LocalFile;
	if (!LocalFile.Open( pszLocalFileName, CFile::modeCreate | CFile::modeWrite, &ex))
	{
		TCHAR szError[ 1024];
		ex.GetErrorMessage( szError, 1024);

		str.Format( "[ZFileTransfer] ERROR : Cannot open local file (%s).", szError);
		PutLog( str, LOG_ERROR);
		str.Format( "[ZFileTransfer] ERROR : Download File : %s FAILED!!!", pszRemoteFileName);
		PutLog( str, LOG_ERROR);


		// Delete file
		DeleteFile( pszLocalFileName);


		return false;
	}


	// Prepare Remote File
	DWORD dwActualFileLength = 0;
	CFtpFileFind finder( GetConnection());
	if ( !finder.FindFile( pszRemoteFileName))
	{
		PutLog( "[ZFileTransfer] ERROR : Cannot open remote file.", LOG_ERROR);
		str.Format( "[ZFileTransfer] ERROR : Download File : %s FAILED!!!", pszRemoteFileName);
		PutLog( str, LOG_ERROR);

		LocalFile.Close();


		// Delete file
		DeleteFile( pszLocalFileName);


		return false;
	}
	finder.FindNextFile();
	dwActualFileLength = (DWORD)finder.GetLength();
	finder.Close();


	// Open internet file
	CInternetFile* pINetFile = GetConnection()->OpenFile( pszRemoteFileName, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY);
	if ( !pINetFile)
	{
		PutLog( "[ZFileTransfer] ERROR : Cannot open internet file.", LOG_ERROR);
		str.Format( "[ZFileTransfer] ERROR : Download File : %s FAILED!!!", pszRemoteFileName);
		PutLog( str, LOG_ERROR);

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
			PutLog( str, LOG_ERROR);

			LocalFile.Close();
			pINetFile->Close();

			return false;
		}

	} while ( (dwRead > 0) && !m_bStopDownload);


	// Stop download
	if ( m_bStopDownload)
	{
		str.Format( "[ZFileTransfer] Download File : %s STOP!!!", pszRemoteFileName);
		PutLog( str, LOG_WARNNING);
		m_bStopDownload = false;

		LocalFile.Close();
		pINetFile->Close();

		DeleteFile( pszLocalFileName);			// Delete file

		return false;
	}


	// Download complete
	str.Format( "[ZFileTransfer] Download File : %s successfuly complete.", pszRemoteFileName);
	PutLog( str);

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
	PutLog( "[ZFileTransfer] Change directory.");


	// Check open connection
	if ( !m_bOpenConnection)
	{
		PutLog( "[ZFileTransfer] ERROR : Connection are not opened.", LOG_ERROR);
		PutLog( "[ZFileTransfer] ERROR : Change directory FAILED!!!", LOG_ERROR);

		return false;
	}


	// Change directory
	CString str;
	if ( !GetConnection()->SetCurrentDirectory( pszDirectory))
	{
		str.Format( "[ZFileTransfer] ERROR : Cannot change the directory : %s", pszDirectory);
		PutLog( str, LOG_ERROR);
		PutLog( "[ZFileTransfer] ERROR : Change directory FAILED!!!", LOG_ERROR);

		return false;
	}


	// Success
	str.Format( "[ZFileTransfer] Change directory successfuly complete. : %s", pszDirectory);
	PutLog( str);

	return true;
}
