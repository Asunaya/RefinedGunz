#include "Afxwin.h"
#include "MUpdateMgr.h"
#include "MPatchInfoLoader.h"
#include "MVersion.h"
#include "FileInfo.h"
using namespace ATL;
#include <atltime.h>
#include <shlwapi.h>
#include "MUIObserver.h"


enum MFT_JOB {
	MFT_JOB_SET_PROGRESS_CONTEXT = 1,
	MFT_JOB_CHDIR,
	MFT_JOB_GET_PATCHINFO,
	MFT_JOB_GET_FILES,
	MFT_JOB_FINISH
};

/*
enum MFT_NOTIFY {
	MFT_NOTIFY_CHDIR,
	MFT_NOTIFY_GETFILE,
	MFT_NOTIFY_LIST
};


enum MFT_STATE {
	MFT_STATE_CHDIR,
	MFT_STATE_GET_PATCHINFO,
	MFT_STATE_CHECK_PATCHINFO,
	MFT_STATE_GET_FILES,
	MFT_STATE_FINISH
};

class MFTState {
	int m_nStateID;
public:
	MFTState()			{ }
	~MFTState()			{ }

	int GetStateID()	{ return m_nStateID; }
	virtual bool CheckComplete(int nNotify) = 0;
};

class MFTStateChdir : public MFTState {
public:
	MFTStateChdir()		{ m_nStateID = MFT_STATE_CHDIR; }
	~MFTStateChdir()	{ }
	virtual bool CheckComplete(MFTNotify* pNotify) {
		if ( (pNotify->GetContext() == MFT_NOTIFY_CHDIR) &&
			 (pNotify->GetResult() == true) )
			return true;
	}
};

void MUpdateMgr::SetState(int nState)
{
	
}

void MUpdateMgr::Update(MFTNotify* pNotify)
{
	if (GetState()->CheckComplete(pNotify)) {
		switch(GetState()->GetStateID()) {
		case MFT_STATE_CHDIR:
			break;
		case MFT_STATE_GET_PATCHINFO:
			break;
		case MFT_STATE_CHECK_PATCHINFO:
			break;
		case MFT_STATE_GET_FILES:
			break;
		case MFT_STATE_FINISH:
			break;
		};
	}
}
*/
//// Listener for MUpdateMgr //////////////////
void MBasicListener::Notify(MFTNotify* pNotify)
{
	if (GetUpdateMgr())
		GetUpdateMgr()->PostNotify(pNotify->Clone());
}


//// Update Manager ///////////////////////////
MUpdateMgr* MUpdateMgr::m_pInstance = NULL;

MUpdateMgr::MUpdateMgr()
{
	m_bVersionCheck = false;
	m_pListener = new MBasicListener(this);
	m_pUIObserver = NULL;
	m_bSelfUpdate = false;

	ClearTotalPatchFileCount();
	ClearPatchFileCount();
	ClearTotalTransfer();
	ClearCurrentTransfer();
}

MUpdateMgr::~MUpdateMgr()
{
	if (m_pListener) {
		delete m_pListener;
		m_pListener = NULL;
	}
	if (m_pUIObserver) {
		delete m_pUIObserver;
		m_pUIObserver = NULL;
	}
}

void MUpdateMgr::Create()
{
	InitializeCriticalSection(&m_csNotifyLock);

	m_FileTransfer.Create(m_pListener);

	m_pInstance = this;
}

void MUpdateMgr::Destroy()
{
	m_pInstance = NULL;

	m_FileTransfer.Destroy();

	DeleteCriticalSection(&m_csNotifyLock);

	if (GetSelfUpdate()) {
		WinExec("MUpdate_New.exe /selfupdate", SW_SHOW);
	}
}
/*
bool MUpdateMgr::OnCommand(MFTCmd* pCmd)
{
	TRACE("MUpdateMgr::OnCOMMAND: ID=%d \n", pCmd->GetID());

	switch(pCmd->GetID()) {
	case 0:
		{
		}
		return true;
	};
}
*/
void MUpdateMgr::OnNotify(MFTNotify* pNotify)
{
	TRACE("MUpdateMgr::OnNOTIFY: ID=%d, RESULT=%d \n", pNotify->GetContext(), pNotify->GetResult());

	switch(pNotify->GetContext()) {
	case MFT_JOB_CHDIR:
		{
			if (pNotify->GetResult()) {
				if (MUIObserver* pObserver = GetUIObserver()) {
					pObserver->UpdateMsg(MUICATEGORY_INFO, "업데이트 정보를 갱신합니다.");
				}

				MFTCmd_REQUESTFILE* pCmdREQUESTFILE = new MFTCmd_REQUESTFILE("patch.xml", "patch.xml");
				pCmdREQUESTFILE->SetContext(MFT_JOB_GET_PATCHINFO);
				m_FileTransfer.PostCommand(pCmdREQUESTFILE);
			} else {
				if (MUIObserver* pObserver = GetUIObserver()) {
					pObserver->UpdateMsg(MUICATEGORY_INFO, "업데이트 정보에 접근할 수 없습니다.");
				}
			}
		}
		break;
	case MFT_JOB_GET_PATCHINFO:
		{
			if (pNotify->GetResult()) {
				if (MUIObserver* pObserver = GetUIObserver()) {
					pObserver->UpdateMsg(MUICATEGORY_INFO, "업데이트 정보를 분석합니다.");
				}

				AnalizePatchInfo("patch.xml");
			} else {
				if (MUIObserver* pObserver = GetUIObserver()) {
					pObserver->UpdateMsg(MUICATEGORY_INFO, "업데이트 정보를 받지 못했습니다.");
				}
			}
		}
		break;
	case MFT_JOB_SET_PROGRESS_CONTEXT:
		{
			TRACE("PROGRESS: %s(%d %%) \n", pNotify->GetString(), pNotify->GetValue());

			DWORD dwRead = 0;
			DWORD dwTransSumBytes = 0;
			DWORD dwMaxBytes = 0;

			MFTNotifyProgress* pNotifyProgress = (MFTNotifyProgress*)pNotify;
			pNotifyProgress->GetProgress(&dwRead, &dwTransSumBytes, &dwMaxBytes);

			UpdateCurrentTransfer(dwRead);

			if (MUIObserver* pObserver = GetUIObserver()) {
				pObserver->UpdateProgress(pNotify->GetString(), dwRead, dwTransSumBytes, dwMaxBytes);
			}
		}
		break;
	case MFT_JOB_GET_FILES:	// Notify with RemoteFileName
		{
			TRACE("GET_FILE: %s %s \n", pNotify->GetString(), pNotify->GetResult()?"succeed":"failed");

			if (pNotify->GetResult()) {
				if (UpdatePatchNode(pNotify->GetString()) == true) {
					if (UpdatePatchFileCount(-1) <= 0) {
						MFTNotify* pNotify = new MFTNotify(MFT_JOB_FINISH, true);
						PostNotify(pNotify);
					}
				}
			} else {
				if (MUIObserver* pObserver = GetUIObserver()) {
					char szMsg[_MAX_DIR];
					sprintf(szMsg, "%s 전송을 실패했습니다.", pNotify->GetString());
					pObserver->UpdateMsg(MUICATEGORY_INFO, szMsg);
				}
			}
		}
		break;
	case MFT_JOB_FINISH:
		{
			TRACE("FINISH UPDATE \n");
			if (pNotify->GetResult()) {
				if (MUIObserver* pObserver = GetUIObserver()) {
					pObserver->UpdateMsg(MUICATEGORY_INFO, "업데이트를 완료했습니다.");
				}
			}
		}
		break;
	default:
		_ASSERT(FALSE);	// Unknown job
		break;
	};

}

void MUpdateMgr::PostNotify(MFTNotify* pNotify)
{
	LockNotify();
		m_NotifyList.push_back(pNotify);
	UnlockNotify();
}

void MUpdateMgr::OnRun()
{
	if (m_NotifyList.size() > 0) {
		LockNotify();
			if (m_NotifyList.begin() != m_NotifyList.end()) {
				MFTNotify* pNotify = *(m_NotifyList.begin());
				m_NotifyList.pop_front();
				OnNotify(pNotify);
				delete pNotify;
			}
		UnlockNotify();
	}
}

bool MUpdateMgr::StartUpdate(const char* pszAddress, unsigned long nPort, const char* pszID, const char* pszPassword)
{
	if (m_FileTransfer.OpenConnection(pszAddress, nPort, pszID, pszPassword) == false)
		return false;

	MFTCmd_SETPROGRESSCONTEXT* pCmdSetProgressContext = new MFTCmd_SETPROGRESSCONTEXT();
	pCmdSetProgressContext->SetContext(MFT_JOB_SET_PROGRESS_CONTEXT);
	m_FileTransfer.PostCommand(pCmdSetProgressContext);

	MFTCmd_CHDIR* pCmdCHDIR = new MFTCmd_CHDIR("gunzupdate");
	pCmdCHDIR->SetContext(MFT_JOB_CHDIR);
	m_FileTransfer.PostCommand(pCmdCHDIR);

/*	MFTCmd_REQUESTLIST* pCmdREQUESTLIST = new MFTCmd_REQUESTLIST();
	pCmdREQUESTLIST->SetContext(3);
	m_FileTransfer.PostCommand(pCmdREQUESTLIST);
*/
	return true;
}

bool MUpdateMgr::LoadPatchInfo(const char* pszFileName)
{
	DWORD dwVersion = 0;
	MPatchInfoLoader loader;
	return loader.Load(pszFileName, &m_PatchList, &dwVersion);
/*	if (loader.Load(pszFileName, &m_PatchList, &dwVersion)) {
		if (CheckVersion(dwVersion)) {
			m_bVersionCheck = true;
		} else {
			m_bVersionCheck = false;
		}
	}
	return m_bVersionCheck;*/
}

bool MUpdateMgr::CheckVersion(unsigned long nVersion)
{
	DWORD dwMajorVersion = 0;
	DWORD dwMinorVersion = 0;

	if (MGetFileVersion(&dwMajorVersion, &dwMinorVersion, "gunz.exe") == false)
		return false;

	DWORD dwFileVersion = COMBINE_VERSION(dwMajorVersion, dwMinorVersion);
	if (dwFileVersion == nVersion)
		return true;
	else
		return false;
}

bool MUpdateMgr::CheckSelfUpdate(const char* pszFileName)
{
	TCHAR szModuleFileName[_MAX_DIR];
	::GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);

	TCHAR szFullPath[_MAX_DIR];
	PathSearchAndQualify(pszFileName, szFullPath, _MAX_DIR);

	if (stricmp(szModuleFileName, szFullPath)==0)
		return true;
	else
		return false;
}

bool MUpdateMgr::AnalizePatchInfo(const char* pszFileName)
{
	if (LoadPatchInfo("patch.xml") == true) {
		ValidatePatches();
		return true;
	} else {
		return false;
	}
}

void MUpdateMgr::ValidatePatches()
{
	ClearTotalPatchFileCount();
	ClearPatchFileCount();
	ClearTotalTransfer();
	ClearCurrentTransfer();

	// Check Validation First
	for (MPatchList::iterator i=m_PatchList.begin(); i!=m_PatchList.end(); i++) {
		MPatchNode* pNode = *i;
		if (pNode->CheckValid() == false) {
			UpdatePatchFileCount(+1);
			UpdateTotalTransfer(pNode->GetSize());
		}
	}

	SetTotalPatchFileCount(GetPatchFileCount());

	if (GetPatchFileCount() <= 0) {
		MFTNotify* pNotify = new MFTNotify(MFT_JOB_FINISH, true);
		PostNotify(pNotify);
		return;
	}

	// Download Invalid files
	for (MPatchList::iterator i=m_PatchList.begin(); i!=m_PatchList.end(); i++) {
		MPatchNode* pNode = *i;
		if (pNode->IsValid() == false) {
			if (MUIObserver* pObserver = GetUIObserver()) {
				pObserver->UpdateMsg(MUICATEGORY_FILE, (char*)pNode->GetName());
			}			

			TCHAR szFullPath[_MAX_DIR];
			PathSearchAndQualify(pNode->GetName(), szFullPath, _MAX_DIR);
			if (CheckSelfUpdate(szFullPath)==true) {
				PathRemoveFileSpec(szFullPath);
				PathAppend(szFullPath, "MUpdate_New.exe");
			}

			MFTCmd_REQUESTFILE* pCmdREQUESTFILE = new MFTCmd_REQUESTFILE(pNode->GetName(), szFullPath);
			pCmdREQUESTFILE->SetContext(MFT_JOB_GET_FILES);
			m_FileTransfer.PostCommand(pCmdREQUESTFILE);
		}
	}
}

MPatchNode* MUpdateMgr::FindPatchNode(const char* pszFileName)
{
	for (MPatchList::iterator i=m_PatchList.begin(); i!=m_PatchList.end(); i++) {
		MPatchNode* pNode = *i;
		if (stricmp(pszFileName, pNode->GetName()) == 0) {
			return pNode;
		}
	}
	return NULL;
}

bool MUpdateMgr::UpdatePatchNode(const char* pszFileName)
{
	MPatchNode* pNode = FindPatchNode(pszFileName);
	if (pNode == NULL)
		return false;

	if (CheckSelfUpdate(pszFileName) == true) {	// MUpdate.exe 자신이면 일단 통과
		MSetFileTime("MUpdate_New.exe", pNode->GetWriteTime());
		pNode->ForcedSetValid(true);
		SetSelfUpdate(true);
		return true;
	}

	if (MSetFileTime(pszFileName, pNode->GetWriteTime()) == FALSE)
		return false;

	return (pNode->CheckValid());
}

extern CWnd* g_pMainFrame;
void MUpdateMgr::LaunchGunz()
{
	WinExec("gunz.exe /launch", SW_SHOW);

	g_pMainFrame->PostMessage(WM_CLOSE, 0, 0);
}
