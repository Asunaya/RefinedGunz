#include "stdafx.h"
#include "resource.h"
#include "TransferDlg.h"
#include "MUIObserverTransDlg.h"


void MUIObserverTransDlg::UpdateMsg(int nCategory, char* pszMsg)
{
	m_pDlg->SetInfoText(pszMsg);
}

void MUIObserverTransDlg::UpdateProgress(char* pszFileName, unsigned long nRead, unsigned long nTransSumBytes, unsigned long nMaxBytes)
{
	//// File Info ///////////////////////////////////
	int nPercent = (int)( ((float)nTransSumBytes/(float)nMaxBytes)*100 + 0.5f ) ;

	CString strInfo;
	strInfo.Format("%s %uK/%uK (%d%%)", pszFileName, nTransSumBytes/1024, nMaxBytes/1024, nPercent);
	m_pDlg->SetInfoText((LPCTSTR)strInfo);
	m_pDlg->SetFileProgress(nPercent);

	//// Total Info //////////////////////////////////
	MUpdateMgr* pUpdateMgr = MUpdateMgr::GetInstance();
	int nCurrentCount = pUpdateMgr->GetPatchFileCount();
	int nTotalCount = pUpdateMgr->GetTotalPatchFileCount();
	DWORD nCurrentTransfer = pUpdateMgr->GetCurrentTransfer();
	DWORD nTotalTransfer = pUpdateMgr->GetTotalTransfer();
	int nTotalPercent = 0;
	if (nTotalTransfer)
		nTotalPercent = (int)( ((float)nCurrentTransfer/(float)nTotalTransfer)*100 + 0.5f );

	CString strTotalInfo;
	strTotalInfo.Format("Total %d/%d %d%% (%uK/%uK)", 
		nCurrentCount, nTotalCount, nTotalPercent, nCurrentTransfer/1024, nTotalTransfer/1024);
	m_pDlg->SetTotalInfoText((LPCTSTR)strTotalInfo);
	m_pDlg->SetTotalProgress(nTotalPercent);
}
