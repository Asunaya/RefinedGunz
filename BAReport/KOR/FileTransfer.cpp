#include "stdafx.h"
#include "FileTransfer.h"



CFileTransfer::CFileTransfer()
{
	m_bRun = FALSE;
	m_hThread = NULL;
	m_bDeleteSrcFile = false;
	m_pTransferCallback = NULL;
}

CFileTransfer::~CFileTransfer()
{
	m_bRun = false;
	if (m_hThread) {
		WaitForSingleObject(m_hThread, INFINITE);
		m_hThread = NULL;
	}
}

void CFileTransfer::Destroy()
{
	m_bRun = false;
	if (m_hThread) {
		WaitForSingleObject(m_hThread, INFINITE);
		m_hThread = NULL;
	}
}

void CFileTransfer::Log(const char* pszMessage)
{
	TRACE("[MapTransLOG] %s \n", pszMessage);
}

void CFileTransfer::PutFileQueue(CString strAddress, int nPort, CString strUserName, CString strPassword, CString strFileName, CString strRemoteFileName)
{
	CFtpItem* pItem = new CFtpItem;
	pItem->strAddress = strAddress;
	pItem->nPort = nPort;
	pItem->strUserName = strUserName;
	pItem->strPassword = strPassword;
	pItem->strFileName = strFileName;
	pItem->strRemoteFileName = strRemoteFileName;
	m_FtpItemQueue.AddTail(pItem);
}

bool CFileTransfer::BeginTransfer()
{
	CWinThread* pWinThread = AfxBeginThread(ThreadProc, this, THREAD_PRIORITY_NORMAL);
	m_hThread = pWinThread->m_hThread;
	return true;
}

void CFileTransfer::EndTransfer()
{
	m_bRun = false;
	// DO TRANSFER FINISH //////////////////////
	if (m_pTransferCallback)
		m_pTransferCallback(REASON_END, 0, 0);
}

UINT ThreadProc(LPVOID pParam) 
{
	CFileTransfer* pTrans = (CFileTransfer*)pParam;
	pTrans->m_bRun = true;
	CFtpItem* pItem = NULL;
	
	while (pTrans->m_FtpItemQueue.GetCount() > 0) {
		pItem = (CFtpItem*)pTrans->m_FtpItemQueue.RemoveHead();

		TRY {
			CFtpConnection* pFtpConnection = pTrans->m_INetSession.GetFtpConnection((LPCSTR)pItem->strAddress, (LPCSTR)pItem->strUserName, (LPCSTR)pItem->strPassword, pItem->nPort);
			if (pFtpConnection == NULL) {
				pTrans->Log("[ERROR] FtpConnection Error");
			}
			if (pTrans->PutFile(pFtpConnection, pTrans->m_pTransferCallback, pItem->strFileName, pItem->strRemoteFileName) == false)
				pTrans->Log("[ERROR] Transfer failed");
			if (pFtpConnection) {
				pFtpConnection->Close();
				delete pFtpConnection;
				pFtpConnection = NULL;
			}
		} CATCH(CInternetException, e) {
			TCHAR szError[256];
			e->GetErrorMessage(szError, 256);
			pTrans->Log(szError);
			break;
		}
		END_CATCH

		delete pItem;
		pItem = NULL;
	}

	if (pItem)
		delete pItem;

	pTrans->Log("SHUTDOWN THREAD");
	pTrans->EndTransfer();
	return 0;
}

bool CFileTransfer::PutFile(CFtpConnection* pFtpConnection, TRANSFERCALLBACK* pStatusCallback, CString strLocal, CString strRemote)
{
	bool bCleanup = false;
	CFile LocalFile;
	CFileException eFile;
	if (!LocalFile.Open((LPCSTR)strLocal, CFile::modeRead|CFile::typeBinary, &eFile)) {
		TRACE("Can't Read the file\n");
		return false;
	}
	DWORD dwLocalSize = LocalFile.GetLength();
	
	CInternetFile* pRemoteFile = (CInternetFile*)pFtpConnection->OpenFile((LPCSTR)strRemote, GENERIC_WRITE, INTERNET_FLAG_TRANSFER_BINARY);
	if (pRemoteFile == NULL) {
		TRACE("Can't Create remote file\n");
		return false;
	}

	DWORD dwTotalSent = 0;
	char szBuf[4096];
	while(m_bRun) {
		int nRead = LocalFile.Read(szBuf, 4096);
		if (nRead == 0) {
			bCleanup = true;
			LocalFile.Close();
			if (m_bDeleteSrcFile) DeleteFile(strLocal);
			

			pRemoteFile->Close();
			delete pRemoteFile;
			return true;
		}
		pRemoteFile->Write(szBuf, nRead);
		dwTotalSent += nRead;
		TRACE("Sent : %d \n", dwTotalSent);
		if (pStatusCallback)
			pStatusCallback(REASON_PROGRESS, dwTotalSent, dwLocalSize);
	}

	

	if (bCleanup == false) {
		LocalFile.Close();
		if (m_bDeleteSrcFile) DeleteFile(strLocal);

		pRemoteFile->Close();
		delete pRemoteFile;
		return true;
	}
	return true;
}

void CFileTransfer::SetDeleteSrcFile(bool bVal)
{
	// 사실은 쓰레드 락을 걸어줘야 한다.
	m_bDeleteSrcFile = bVal;
}