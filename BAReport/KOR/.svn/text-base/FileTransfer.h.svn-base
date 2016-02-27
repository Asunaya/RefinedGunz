#ifndef FILETRANSFER_H
#define FILETRANSFER_H


#include <afxinet.h>
class CFileTransfer;


typedef void(TRANSFERCALLBACK)(int nReason, DWORD dwSentSize, DWORD dwLocalSize);
#define REASON_PROGRESS		1
#define REASON_END			2
#define REASON_ERROR		3


class CFtpItem : public CObject {
public:
	CString			strAddress;
	int				nPort;
	CString			strUserName;
	CString			strPassword;
	CString			strFileName;
	CString			strRemoteFileName;
};


class CFileTransfer {
private:
	bool				m_bRun;
	HANDLE				m_hThread;
	bool				m_bDeleteSrcFile;

	CInternetSession	m_INetSession;
	CObList				m_FtpItemQueue;

	TRANSFERCALLBACK*	m_pTransferCallback;

	friend UINT ThreadProc(LPVOID pParam);
	void Log(const char* pszMessage);

public:
	CFileTransfer();
	~CFileTransfer();
	void Destroy();

	void SetTransferCallback(TRANSFERCALLBACK* pCallback) { m_pTransferCallback = pCallback; }
	void PutFileQueue(CString strAddress, int nPort, CString strUserName, CString strPassword, CString strFileName, CString strRemoteFileName);
	bool BeginTransfer();
	void EndTransfer();
	bool PutFile(CFtpConnection* pFtpConnection, TRANSFERCALLBACK* pStatusCallback, CString strLocal, CString strRemote);

	void SetDeleteSrcFile(bool bVal);
};


#endif
