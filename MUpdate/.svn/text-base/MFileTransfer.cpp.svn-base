#include "MFileTransfer.h"
#include "MFTNotify.h"
#include "FileInfo.h"
#include <stack>
using namespace std;


//// Session for MFileTransfer  ///////////////////////
MFTSession::MFTSession(LPCTSTR pszAppName, int nMethod)
	: CInternetSession(pszAppName, 1, nMethod)
{
	EnableStatusCallback(TRUE);
}

void MFTSession::OnStatusCallback(DWORD dwContext, DWORD dwInternetStatus,
	LPVOID lpvStatusInfomration, DWORD dwStatusInformationLen)
{
	TRACE("Callback: Context=%d, InternetStatus=%d, StatusInfo=%d, StatusInfoLen=%d \n", dwContext, dwInternetStatus, lpvStatusInfomration, dwStatusInformationLen);

//	if (dwInternetStatus == INTERNET_STATUS_CONNECTED_TO_SERVER)
//		cerr << _T("Connection made!") << endl;
}


//// MFileTransfer  ///////////////////////////////////
MFileTransfer::MFileTransfer() : m_INetSession("MUpdate")
{
	m_pConnection = NULL;
	m_pListener = NULL;
	m_nProgressContext = 0;
}

MFileTransfer::~MFileTransfer()
{
}

void MFileTransfer::Create(MFTListener* pListener)
{
	m_bShutdown = false;
	m_pListener = pListener;

	InitializeCriticalSection(&m_csCmdQueueLock);

	MSafeThread::Create();
}

void MFileTransfer::Destroy()
{
	m_bShutdown = true;

	if (GetConnection()) {
		CloseConnection();
	}

	MSafeThread::Destroy();

	DeleteCriticalSection(&m_csCmdQueueLock);
}

bool MFileTransfer::OpenConnection(const char* pszAddress, unsigned long nPort, const char* pszID, const char* pszPassword)
{
	CFtpConnection* pFtpConnection = NULL;
	TRY {
		pFtpConnection = m_INetSession.GetFtpConnection(
				(LPCSTR)pszAddress, (LPCSTR)pszID, (LPCSTR)pszPassword, (INTERNET_PORT)nPort);
		m_pConnection = pFtpConnection;
		return true;
	} CATCH(CInternetException, e) {
		TCHAR szError[256];
		e->GetErrorMessage(szError, 256);

		if (pFtpConnection) {
			pFtpConnection->Close();
			delete pFtpConnection;
		}
		return false;
	}
	END_CATCH	
}

void MFileTransfer::CloseConnection()
{	
	if (GetConnection()) {
		GetConnection()->Close();
		delete m_pConnection;
		m_pConnection = NULL;
	}
}

void MFileTransfer::PostCommand(MFTCmd* pCmd)
{
	LockCmdQueue();
		m_CommandList.push_back(pCmd);
	UnlockCmdQueue();
}

void MFileTransfer::Notify(MFTNotify* pNotify)
{
	if (GetListener())
		GetListener()->Notify(pNotify);
}

void MFileTransfer::NotifyProgress(const char* pszRemoteFile, DWORD dwRead, DWORD dwReadSum, DWORD dwLength)
{
	if ((dwLength == 0) || (GetProgressContext() == 0))
		return;

	MFTNotifyProgress notify(GetProgressContext(), true, pszRemoteFile);
	notify.SetProgress(dwRead, dwReadSum, dwLength);
	Notify(&notify);
}

void MFileTransfer::OnRun()
{
	while(m_bShutdown == false) {
		if (m_CommandList.size() <= 0) {
			Sleep(10);
			continue;
		}

		MFTCmd* pCmd = NULL;
		LockCmdQueue();
			MFTCmdList::iterator i = m_CommandList.begin();
			if (i != m_CommandList.end()) {
				pCmd = *i;
				m_CommandList.pop_front();
			}
		UnlockCmdQueue();

		if (pCmd == NULL) continue;

		switch(pCmd->GetID()) {
		case MFT_CMDID_SET_PROGRESS_CONTEXT:
			{
				MFTCmd_SETPROGRESSCONTEXT* pCmdSetProgressContext = (MFTCmd_SETPROGRESSCONTEXT*)pCmd;
				SetProgressContext(pCmdSetProgressContext->GetContext());
			}
			break;
		case MFT_CMDID_CHDIR:
			{
				MFTCmd_CHDIR* pChdirCmd = (MFTCmd_CHDIR*)pCmd;
				bool bResult = ChangeDir(pChdirCmd->GetDir());

				MFTNotify notify(pCmd->GetContext(), bResult);
				Notify(&notify);
			}
			break;
		case MFT_CMDID_REQUESTLIST:
			{
				bool bResult = ListFiles();

				MFTNotify notify(pCmd->GetContext(), bResult);
				Notify(&notify);
			}
			break;
		case MFT_CMDID_REQUESTFILE:
			{
				MFTCmd_REQUESTFILE* pRunCmd = (MFTCmd_REQUESTFILE*)pCmd;
				bool bResult = DownloadFile(pRunCmd->GetRemoteFileName(), pRunCmd->GetLocalFileName());

				MFTNotify notify(pCmd->GetContext(), bResult, pRunCmd->GetRemoteFileName());
				Notify(&notify);
			}
			break;
		default:
			{
				MFTNotify notify(pCmd->GetContext(), false);
				Notify(&notify);
			}
			break;
		};

		delete pCmd;
	}	// Thread loop
	TRACE("Shutdown MFileTransfer Thread \n");
}

bool MFileTransfer::ChangeDir(const char* pszDir)
{
	if (GetConnection() == NULL) 
		return false;

	if (GetConnection()->SetCurrentDirectory(pszDir) == TRUE)
		return true;
	else
		return false;
}

bool MFileTransfer::ListFiles()
{
	if (GetConnection() == NULL) 
		return false;

	try {
		stack<string>	ScanDir;
		ScanDir.push(".");

		while(!ScanDir.empty()) {
			string strDir = ScanDir.top();
			ScanDir.pop();

			TCHAR szFind[_MAX_DIR];
			sprintf(szFind, "%s/*", strDir.c_str());

			// Find in Directory
			CFtpFileFind finder(GetConnection());
			BOOL bWorking = finder.FindFile(szFind);
			while (bWorking) {
				bWorking = finder.FindNextFile();

				if (finder.IsDots())
					continue;

				if (finder.IsDirectory()) {
					char szPath[_MAX_DIR];
					sprintf(szPath, "%s/%s", strDir.c_str(), finder.GetFileName());
					ScanDir.push(szPath);
					TRACE("SCANDIR_PUSH : %s \n", szPath);
				} else {
					TRACE("%s\n", (LPCTSTR) finder.GetFileURL());
				}
			}
		}
	} catch (CInternetException* pEx) {
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);
		TRACE("ERROR!  %s\n", sz);
		pEx->Delete();
		return false;
	}
	
	return true;
}

bool MFileTransfer::DownloadFile(const char* pszRemoteFileName, const char* pszLocalFileName)
{
	if (GetConnection() == NULL) 
		return false;

	MakePath(pszLocalFileName);

/*	if (!GetConnection()->GetFile(pszFileName, pszFileName, FALSE)) {
		TRACE("GetFile Error : %d \n", GetLastError());
		return false;
	}*/

	if (!GetFileEx(pszRemoteFileName, pszLocalFileName)) {
		TRACE("GetFileEx Error \n");
		return false;
	}

	return true;
}

bool MFileTransfer::GetFileEx(const char* pszRemoteFile, const char* pszLocalFile)
{
	//// Prepare Local File //////////////////////////////////////
	CFileException ex;
	CFile LocalFile;

	if (!LocalFile.Open(pszLocalFile, CFile::modeCreate | CFile::modeWrite, &ex)) {
		TCHAR szError[1024];
		ex.GetErrorMessage(szError, 1024);
		TRACE("ERROR on MFileTransfer::GetFileEx - %s \n", szError);

		LocalFile.Close();
		return false;
	}

	//// Prepare Remote File /////////////////////////////////////
	DWORD dwActualFileLength = 0;
	CFtpFileFind finder(GetConnection());
	if (finder.FindFile(pszRemoteFile)) {
		finder.FindNextFile();
		dwActualFileLength = (DWORD)finder.GetLength();
	}

	CInternetFile* pINetFile = GetConnection()->OpenFile(pszRemoteFile, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY);
	if (!pINetFile) {
		return false;
	}

	#define MBUF_SIZE	4096
	BYTE Buffer[MBUF_SIZE];
	DWORD dwRead = 0;
	DWORD dwTotalRead = 0;

	do {
		try {
			dwRead = pINetFile->Read(Buffer, MBUF_SIZE);
			LocalFile.Write(Buffer, dwRead);

			dwTotalRead += dwRead;

			NotifyProgress(pszRemoteFile, dwRead, dwTotalRead, dwActualFileLength);
		} catch (CInternetException* pEx) {
			TCHAR szError[1024];
			pEx->GetErrorMessage(szError, 1024);
			TRACE("ERROR on MFileTransfer::GetFileEx - %s \n", szError);
			pEx->Delete();
		}
	}
	while ((dwRead > 0) && (m_bShutdown==false));

	LocalFile.Close();
	pINetFile->Close();

	return true;
}
