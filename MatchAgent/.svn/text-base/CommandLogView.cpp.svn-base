// CommandLogView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "CommandLogView.h"
#include "MCommand.h"
#include "MSharedCommandTable.h"


// CCommandLogView

IMPLEMENT_DYNCREATE(CCommandLogView, CListView)

CCommandLogView::CCommandLogView()
{
}

CCommandLogView::~CCommandLogView()
{
}

BEGIN_MESSAGE_MAP(CCommandLogView, CListView)
END_MESSAGE_MAP()


// CCommandLogView 진단입니다.

#ifdef _DEBUG
void CCommandLogView::AssertValid() const
{
	CListView::AssertValid();
}

void CCommandLogView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG


// CCommandLogView 메시지 처리기입니다.

void CCommandLogView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	CListView::OnInitialUpdate();

	// TODO: GetListCtrl()을 호출하여 해당 list 컨트롤을 직접 액세스함으로써
	// ListView를 항목으로 채울 수 있습니다.

	// this code only works for a report-mode list view
	ASSERT(GetStyle() & LVS_REPORT);

	// Gain a reference to the list control itself
	CListCtrl& theCtrl = GetListCtrl();

	// Insert a column. This override is the most convenient.
	theCtrl.InsertColumn(0, _T("Time"), LVCFMT_LEFT);
	theCtrl.InsertColumn(1, _T("Type"), LVCFMT_LEFT);
	theCtrl.InsertColumn(2, _T("Sender"), LVCFMT_LEFT);
	theCtrl.InsertColumn(3, _T("Receiver"), LVCFMT_LEFT);
	theCtrl.InsertColumn(4, _T("Command"), LVCFMT_LEFT);

	/*
	// The other InsertColumn() override requires an initialized
	// LVCOLUMN structure.
	LVCOLUMN col;
	col.mask = LVCF_FMT | LVCF_TEXT;
	col.pszText = _T("Command");
	col.fmt = LVCFMT_LEFT;
	theCtrl.InsertColumn(1, &col);
	*/

	// Set reasonable widths for our columns
	theCtrl.SetColumnWidth(0, 80);
	theCtrl.SetColumnWidth(1, 50);
	theCtrl.SetColumnWidth(2, 80);
	theCtrl.SetColumnWidth(3, 80);
	theCtrl.SetColumnWidth(4, LVSCW_AUTOSIZE_USEHEADER);
}

BOOL CCommandLogView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= LVS_REPORT;

	return CListView::PreCreateWindow(cs);
}


void CCommandLogView::AddCommand(unsigned long int nGlobalClock, CCommandType t, MCommand* pCmd)
{
	if ( (pCmd->GetID() == MC_AGENT_TUNNELING_TCP) ||
		 (pCmd->GetID() == MC_AGENT_TUNNELING_UDP) )
		 return;

	char temp[1024];
	char szParam[256];
	//sprintf(temp, "%d: %s", nGlobalClock, pCmd->m_pCommandDesc->GetName());
	sprintf(temp, "%s", pCmd->m_pCommandDesc->GetName());
	for(int i=0; i<pCmd->GetParameterCount(); i++){
		pCmd->GetParameter(i)->GetString(szParam);
		sprintf(temp, "%s %s(%s)", temp, pCmd->GetParameter(i)->GetClassName(), szParam);
		
	}

	CListCtrl& theCtrl = GetListCtrl();

	char szGlobalClock[128];
	sprintf(szGlobalClock, "%d", nGlobalClock);
	int nItemCount = theCtrl.GetItemCount();
	theCtrl.InsertItem(LVIF_TEXT|LVIF_STATE, nItemCount, szGlobalClock, 0, LVIS_SELECTED, 0, 0);
	char szType[64] = "Unknown";
	if(t==CCT_LOCAL) strcpy(szType, "Local");
	else if(t==CCT_SEND) strcpy(szType, "Send");
	else if(t==CCT_RECEIVE) strcpy(szType, "Receive");
	char szSenderUID[128];
	sprintf(szSenderUID, "%u:%u", pCmd->m_Sender.High, pCmd->m_Sender.Low);
	char szReceiverUID[128];
	sprintf(szReceiverUID, "%u:%u", pCmd->m_Receiver.High, pCmd->m_Receiver.Low);
	theCtrl.SetItemText(nItemCount, 1, szType);
	theCtrl.SetItemText(nItemCount, 2, szSenderUID);
	theCtrl.SetItemText(nItemCount, 3, szReceiverUID);
	theCtrl.SetItemText(nItemCount, 4, temp);

	//theCtrl.SetScrollPos(SB_VERT, );
	int nCount = theCtrl.GetItemCount();
	if (nCount > 0)	
		theCtrl.EnsureVisible(nCount-1, TRUE);

#define MAX_LOG_COUNT	1000
	if (nCount > MAX_LOG_COUNT)
		theCtrl.DeleteItem(0);
}
