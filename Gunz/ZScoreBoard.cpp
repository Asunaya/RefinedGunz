#include "stdafx.h"

#include "ZScoreBoard.h"
#include "MListBox.h"
#include "ZScoreListBox.h"
#include "ZFrame.h"
#include "MFrame.h"
#include "ZGameInterface.h"
#include "ZApplication.h"
#include "MMatchRule.h"
#include "ZMatch.h"

void _AddListItem(MListBox* pList, const char* szString)
{
	MDefaultListItem* pNew = new MDefaultListItem(szString);
	pList->Add(pNew);
}


ZScoreBoard::ZScoreBoard(ZCharacterManager* pCharacterManager, ZIDLResource* pIDLResource,
						 const char* szName, MWidget* pParent, MListener* pListener)
: ZInterface(szName, pParent, pListener)
{
	m_pCharacterManager = pCharacterManager;
	m_pIDLResource = pIDLResource;
}

ZScoreBoard::~ZScoreBoard()
{
	delete m_pListBox;
}

bool ZScoreBoard::OnCreate()
{
	m_pFrame = NULL;
	m_pListBox = NULL;

	MWidget* pWidget = m_pIDLResource->FindWidget("CombatScoreBoardFrame");
	if (pWidget == NULL) return false;
	m_pFrame = (MFrame*)pWidget;

	pWidget = m_pIDLResource->FindWidget("CombatScoreListBox");
	if (pWidget != NULL) m_pListBox = (ZScoreListBox*)pWidget;

	Refresh();
	return false;
}

void ZScoreBoard::OnDestroy()
{
	if (m_pFrame)
	{
		m_pFrame->Show(false);
	}
	m_pListBox = NULL;
	m_pFrame = NULL;
}

void ZScoreBoard::OnDraw(MDrawContext* pDC)
{
	pDC->SetOpacity(255);
}

bool ZScoreBoard::OnEvent(MEvent* pEvent, MListener* pListener)
{


	return false;
}

void ZScoreBoard::Clear()
{
	if (m_pListBox == NULL) return;

	m_pListBox->RemoveAll();
}
void ZScoreBoard::Add(const char* szTeam, const char* szID, const char* szState, 
					  int nScore, int nKills, int nDeaths, int nPing)
{
	if (m_pListBox == NULL) return;

	MListItem* pNew = new ZListItemScore(szTeam, szID, szState, nScore, nKills, nDeaths, nPing);
	m_pListBox->Add(pNew);
}

void ZScoreBoard::Update()
{
	static unsigned long nLastTime = timeGetTime();
	unsigned long nNowTime = timeGetTime();
	if ((nNowTime - nLastTime) > 2000)
	{
		Refresh();
		nLastTime = nNowTime;
	}

}

bool ZScoreBoard::OnShow(void)
{
	if (m_pFrame == NULL) return false;

	Refresh();
	m_pFrame->Show(true);

	return true;
}
void ZScoreBoard::OnHide(void)
{
	if (m_pFrame == NULL) return;

	m_pFrame->Show(false);
}

bool GetUserInfoUID(MUID uid,MCOLOR& _color,char* sp_name,MMatchUserGradeID& gid);

void ZScoreBoard::Refresh()
{
	if (m_pListBox == NULL) return;

	char szState[64];
	char szTeam[128];
	szTeam[0] = NULL;

	char* szRefName = NULL;

	MCOLOR _color;
	char sp_name[256];
	MMatchUserGradeID gid;

	m_pListBox->RemoveAll();

	for (ZCharacterManager::iterator itor = m_pCharacterManager->begin();
		itor != m_pCharacterManager->end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;

		if (pCharacter->IsDie()) strcpy(szState, "Dead");
		else strcpy(szState, "Alive");

		ZMatch *pMatch=ZApplication::GetGame()->GetMatch();
		if (pMatch && pMatch->IsTeamPlay())
		{
			strcpy(szTeam, ZApplication::GetGame()->GetMatch()->GetTeamName(pCharacter->GetTeamID()));
		}

		int nPing = (pCharacter->m_UID == ZGetGameClient()->GetPlayerUID() ? 0 : 999);
		MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(pCharacter->m_UID);
		if (pPeer) {
			//nPing = int(pCharacter->m_fAveragePingTime*1000.f)
			nPing = pPeer->GetPing(ZApplication::GetGame()->GetTickTime());
		}

		szRefName = NULL;
		
		if(GetUserInfoUID(  pCharacter->m_UID,_color,sp_name,gid)) {
			szRefName = sp_name;
		}
		else{
			szRefName = pCharacter->GetProperty()->szName;
		}

		Add(szTeam, szRefName, szState, pCharacter->GetStatus()->nExp, 
			pCharacter->GetStatus()->nKills, pCharacter->GetStatus()->nDeaths, nPing);
	}

	MWidget* pWidget = m_pIDLResource->FindWidget("CombatScoreGameType");
	if (pWidget != NULL) 
	{
		pWidget->SetText(GetGameTypeStr(g_pGame->GetMatch()->GetMatchType()));
	}

	pWidget = m_pIDLResource->FindWidget("CombatScoreRound");
	if (pWidget != NULL) 
	{
		char szTemp[128];
		sprintf(szTemp, "Round %2d / %2d", g_pGame->GetMatch()->GetCurrRound() + 1,
			g_pGame->GetMatch()->GetRoundCount());

		if (ZApplication::GetGame()->GetMatch()->IsTeamPlay())
		{
			char szTemp2[128];
			sprintf(szTemp2, "     Score %2d(Red) : %2d(Blue)", g_pGame->GetMatch()->GetTeamScore(MMT_RED), 
				g_pGame->GetMatch()->GetTeamScore(MMT_BLUE));
			strcpy(szTemp, szTemp2);
		}
		pWidget->SetText(szTemp);
	}

}