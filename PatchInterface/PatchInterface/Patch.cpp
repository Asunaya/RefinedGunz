// Patch.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "PatchInterface.h"
#include "Patch.h"
#include ".\patch.h"
#include "KeeperManager.h"


// CPatch 대화 상자입니다.

IMPLEMENT_DYNAMIC(CPatch, CDialog)
CPatch::CPatch(CWnd* pParent /*=NULL*/)
	: CDialog(CPatch::IDD, pParent)
{
}

CPatch::~CPatch()
{
}

void CPatch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPatch, CDialog)
	ON_BN_CLICKED(ID_DOWNLOAD_SERVER_PATCH, OnBnClickedDownloadServerPatch)
	ON_BN_CLICKED(ID_PREPARE_SERVER_PATCH, OnBnClickedPrepareServerPatch)
	ON_BN_CLICKED(ID_PATCH_SERVER, OnBnClickedPatchServer)
	ON_BN_CLICKED(ID_RESET_SERVER_PATCH, OnBnClickedResetServerPatch)
	ON_BN_CLICKED(ID_DOWNLOAD_AGENT_PATCH, OnBnClickedDownloadAgentPatch)
	ON_BN_CLICKED(ID_PREPARE_AGENT_PATCH, OnBnClickedPrepareAgentPatch)
	ON_BN_CLICKED(ID_PATCH_AGENT, OnBnClickedPatchAgent)
	ON_BN_CLICKED(ID_RESET_AGENT_PATCH, OnBnClickedResetAgentPatch)
END_MESSAGE_MAP()


// CPatch 메시지 처리기입니다.

void CPatch::OnBnClickedDownloadServerPatch()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestDownloadServerPatchFile();
}

void CPatch::OnBnClickedPrepareServerPatch()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestPrepareServerPatch();
}

void CPatch::OnBnClickedPatchServer()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestServerPatch();
}

void CPatch::OnBnClickedResetServerPatch()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestResetPatch();
}

void CPatch::OnBnClickedDownloadAgentPatch()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestDownloadAgentPatchFile();
}

void CPatch::OnBnClickedPrepareAgentPatch()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestPrepareAgentPatch();
}

void CPatch::OnBnClickedPatchAgent()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestAgentPatch();
}

void CPatch::OnBnClickedResetAgentPatch()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	GetKeeperMgr.RequestResetPatch();
}
