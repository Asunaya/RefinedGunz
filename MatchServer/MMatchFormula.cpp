#include "stdafx.h"
#include "MMatchFormula.h"
#include <math.h>
#include "MMatchItem.h"

#define MMTOC_FORMULA_TABLE						"FORMULA_TABLE"
#define MMTOC_LEVEL_MODIFIER					"LM"
#define MMTOC_NEED_EXP_LEVEL_MODIFIER			"NeedExpLM"
#define MMTOC_GETTING_EXP_LEVEL_MODIFIER		"GettingExpLM"
#define MMTOC_GETTING_BOUNTY_LEVEL_MODIFIER		"GettingBountyLM"


float MMatchFormula::m_fNeedExpLMTable[MAX_LEVEL+1];
float MMatchFormula::m_fGettingExpLMTable[MAX_LEVEL+1];
float MMatchFormula::m_fGettingBountyLMTable[MAX_LEVEL+1];

unsigned long int MMatchFormula::m_nNeedExp[MAX_LEVEL+1];
unsigned long int MMatchFormula::m_nGettingExp[MAX_LEVEL+1];
unsigned long int MMatchFormula::m_nGettingBounty[MAX_LEVEL+1];

bool MMatchFormula::Create()
{
	auto ZeroMemory = [](void* a, size_t b) { memset(a, 0, b); };
	ZeroMemory(m_fNeedExpLMTable, sizeof(m_fNeedExpLMTable));
	ZeroMemory(m_fGettingExpLMTable, sizeof(m_fGettingExpLMTable));
	ZeroMemory(m_fGettingBountyLMTable, sizeof(m_fGettingBountyLMTable));

	ZeroMemory(m_nNeedExp, sizeof(m_nNeedExp));
	ZeroMemory(m_nGettingExp, sizeof(m_nGettingExp));
	ZeroMemory(m_nGettingBounty, sizeof(m_nGettingBounty));

	if (!ReadXml(FILENAME_MATCH_FORMULA))
	{
		return false;
	}

	PreCalcNeedExp();
	PreCalcGettingExp();
	PreCalcGettingBounty();

	return true;
}

bool MMatchFormula::ReadXml(const char* szXmlFileName)
{
	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(szXmlFileName))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, MMTOC_FORMULA_TABLE))
		{
			char szTemp[64];
			chrElement.GetAttribute(szTemp, "id");
			if (!_stricmp(szTemp, MMTOC_NEED_EXP_LEVEL_MODIFIER))
			{
				ParseNeedExpLM(chrElement);
			}
			else if (!_stricmp(szTemp, MMTOC_GETTING_EXP_LEVEL_MODIFIER))
			{
				ParseGettingExpLM(chrElement);
			}
			else if (!_stricmp(szTemp, MMTOC_GETTING_BOUNTY_LEVEL_MODIFIER))
			{
				ParseGettingBountyLM(chrElement);
			}
		}
	}

	xmlIniData.Destroy();
	return true;
}

void MMatchFormula::ParseNeedExpLM(MXmlElement& element)
{
	int iCount = element.GetChildNodeCount();
	MXmlElement childElement;
	char szTagName[64];

	for (int i = 0; i < iCount; i++)
	{
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, MMTOC_LEVEL_MODIFIER))
		{
			int nUpper = 0, nLower = 0;
			float fLM = 0.0f;

			childElement.GetAttribute(&nLower, "lower");	
			childElement.GetAttribute(&nUpper, "upper");
			childElement.GetContents(&fLM);

			for (int i = nLower; i <= nUpper; i++)
			{
				if (MMatchFormula::m_fNeedExpLMTable[i] <= 0.0001f) 
					MMatchFormula::m_fNeedExpLMTable[i] = fLM;
			}
		}
	}
}

void MMatchFormula::ParseGettingExpLM(MXmlElement& element)
{
	int iCount = element.GetChildNodeCount();
	MXmlElement childElement;
	char szTagName[64];

	for (int i = 0; i < iCount; i++)
	{
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, MMTOC_LEVEL_MODIFIER))
		{
			int nUpper = 0, nLower = 0;
			float fLM = 0.0f;

			childElement.GetAttribute(&nLower, "lower");	
			childElement.GetAttribute(&nUpper, "upper");
			childElement.GetContents(&fLM);

			for (int i = nLower; i <= nUpper; i++)
			{
				if (MMatchFormula::m_fGettingExpLMTable[i] <= 0.0001f) 
					MMatchFormula::m_fGettingExpLMTable[i] = fLM;
			}
		}
	}
}

void MMatchFormula::ParseGettingBountyLM(MXmlElement& element)
{
	int iCount = element.GetChildNodeCount();
	MXmlElement childElement;
	char szTagName[64];

	for (int i = 0; i < iCount; i++)
	{
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, MMTOC_LEVEL_MODIFIER))
		{
			int nUpper = 0, nLower = 0;
			float fLM = 0.0f;

			childElement.GetAttribute(&nLower, "lower");	
			childElement.GetAttribute(&nUpper, "upper");
			childElement.GetContents(&fLM);

			for (int i = nLower; i <= nUpper; i++)
			{
				if (MMatchFormula::m_fGettingBountyLMTable[i] <= 0.0001f) 
					MMatchFormula::m_fGettingBountyLMTable[i] = fLM;
			}
		}
	}
}


void MMatchFormula::PreCalcNeedExp()
{
	unsigned long int n;
	for (int lvl = 1; lvl <= MAX_LEVEL; lvl++)
	{
		n = (unsigned long int)((lvl * lvl * m_fNeedExpLMTable[lvl] * 100) + 0.5f);
		n = n * 2;	// 기획서보다 2배 더한다.
		m_nNeedExp[lvl] = m_nNeedExp[lvl-1] + n;
	}
}

void MMatchFormula::PreCalcGettingExp()
{
	for (int lvl = 1; lvl <= MAX_LEVEL; lvl++)
	{
		unsigned long int nExp = 0;

		// 획득경험치 = LVL * LM * 20 + (LVL-1) * LM * 10
		m_nGettingExp[lvl] = ( (unsigned long int)((lvl * m_fGettingExpLMTable[lvl] * 20) +0.5f) + 
                            (unsigned long int)(((lvl-1) * m_fGettingExpLMTable[lvl] * 10) + 0.5f) );
	}
}

void MMatchFormula::PreCalcGettingBounty()
{
	for (int lvl = 1; lvl <= MAX_LEVEL; lvl++)
	{
		unsigned long int nExp = 0;

		// 획득바운티 = TRUNC(LVL * LM * 1.2)
		m_nGettingBounty[lvl] = ( (unsigned long int)((lvl * m_fGettingBountyLMTable[lvl] * 1.2f) + 0.5f) );
	}
}


unsigned long int MMatchFormula::CalcPanaltyEXP(int nAttackerLevel, int nVictimLevel)
{
#define BOUNDARY_PANALTY_LEVEL		20

	// 20레벨 이하는 경험치가 떨어지지 않는다.
	if (nVictimLevel <= BOUNDARY_PANALTY_LEVEL) return 0;

	// 자신보다 레벨이 높은 사람에게 죽으면 손실이 없다.
	if (nAttackerLevel > nVictimLevel) return 0;

	// 일정 레벨차이에는 경험치 손실이 없다
	if (abs(nAttackerLevel - nVictimLevel) <= (int(nVictimLevel/10) + 1) * 2) return 0;

	unsigned long int nExp = 0;
	unsigned long int nGettingExp = GetGettingExp(nAttackerLevel, nVictimLevel);
	int nGap = (nVictimLevel - nAttackerLevel);
	if (nGap < 0) nGap = 0;

	nExp = (unsigned long int)(nGettingExp * 0.5f * (1 + 0.1f * nGap));
	unsigned long int nMaxExp = m_nGettingExp[nVictimLevel] * 2;

	if (nExp > nMaxExp) nExp = nMaxExp;

	return (unsigned long int)nExp;
}

unsigned long int MMatchFormula::GetSuicidePanaltyEXP(int nLevel)
{
#define BOUNDARY_SUICIDE_PANALTY_LEVEL		5

	// 5레벨 이하는 경험치가 떨어지지 않는다.
	if (nLevel <= BOUNDARY_SUICIDE_PANALTY_LEVEL) return 0;

	unsigned long int nExp = 0;
	unsigned long int nGettingExp = GetGettingExp(nLevel, nLevel);
	int nGap = 0;

	nExp = (unsigned long int)(nGettingExp * 0.5f * (1 + 0.1f * nGap));
	unsigned long int nMaxExp = m_nGettingExp[nLevel] * 2;

	if (nExp > nMaxExp) nExp = nMaxExp;

	// 자살일 경우 경험치 손실이 두배
	nExp = nExp * 2;

	return (unsigned long int)nExp;
}

int MMatchFormula::GetLevelFromExp(unsigned long int nExp)
{
	for (int level = 1; level < MAX_LEVEL; level++)
	{
		if(nExp < m_nNeedExp[level])
		{
			return level;
		}
	}

	return MAX_LEVEL;
}

unsigned long int MMatchFormula::GetGettingExp(int nAttackerLevel, int nVictimLevel)
{ 
	unsigned long int nExp, nAttackerMaxExp;
    
	nExp = m_nGettingExp[nVictimLevel];
	nAttackerMaxExp = m_nGettingExp[nAttackerLevel] * 2;
	if (nExp > nAttackerMaxExp) nExp = nAttackerMaxExp;

	return nExp;
}

unsigned long int MMatchFormula::GetGettingBounty(int nAttackerLevel, int nVictimLevel)
{
	unsigned long int nBounty, nAttackerMaxBounty;
    
	nBounty = m_nGettingBounty[nVictimLevel];
	nAttackerMaxBounty = m_nGettingBounty[nAttackerLevel] * 2;
	if (nBounty > nAttackerMaxBounty) nBounty = nAttackerMaxBounty;

	return nBounty;
}

int MMatchFormula::GetLevelPercent(unsigned long int nExp, int nNowLevel)
{
	unsigned long int nNowLevelExp, nNextLevelExp;
	int nPercent;

	nNowLevelExp = MMatchFormula::GetNeedExp(nNowLevel-1);
	nNextLevelExp = MMatchFormula::GetNeedExp(nNowLevel);

	nPercent = (int)(((float)(nExp - nNowLevelExp) / (float)(nNextLevelExp - nNowLevelExp)) * 100);
	if (nPercent < 0) nPercent = 0; 
	else if (nPercent > 100) nPercent = 100;

	return nPercent;
}

int MMatchFormula::GetClanBattlePoint(int nWinnerClanPoint, int nLoserClanPoint, int nOneTeamMemberCount)
{
	// http://iworks.maietgames.com/mdn/wiki.php/클랜전 에 공식이 나와있음
/*
Delta 만큼 이긴 클랜에 점수 더하고 진 클랜에 점수 빼기
(k=이긴 클랜 점수) (v=진 클랜 점수)

Delta1 = 5 / 1 + 10^((k-v)/1000)
Delta2 = tc / 10^((wc-lc)/50)
(tc=총인원) (wc=이긴팀인원) (lc=진팀인원)
Delta = Delta1+Delta2
*/

	float Delta1 = 5.0f / (1 + pow(5.0f, float(nWinnerClanPoint-nLoserClanPoint) / 1000.0f));
	float Delta2 = float(nOneTeamMemberCount*2);
	int Delta = int(Delta1 + Delta2);
	if (Delta < 1) Delta = 1;

	return Delta;
}

// 우선 왼쪽 것을 검사한 후에 오른쪽 것을 검사해서 반지 한개만 적용한다. 프리미엄 IP도 검사
float MMatchFormula::CalcXPBonusRatio(MMatchObject* pCharObj, MMatchItemBonusType nBonusType)
{
	float fBonusRatio = 0.0f;

	// 넷마블 PC방 보너스 계산 ////////////////////////////////////////////////////////////
	if (pCharObj->GetAccountInfo()->m_nPGrade == MMPG_PREMIUM_IP)
	{
		const float PREMIUM_IP_BONUS = 0.2f;	// 1.2배

		fBonusRatio += PREMIUM_IP_BONUS;
	}

	// 경험치 아이템 계산 //////////////////////////////////////////////////////////////////
	MMatchEquipedItem* pEquipedItems = &pCharObj->GetCharInfo()->m_EquipedItem;

	MMatchItem*		pItemL = pEquipedItems->GetItem(MMCIP_FINGERL);
	MMatchItem*		pItemR = pEquipedItems->GetItem(MMCIP_FINGERR);
	MMatchItemDesc* pItemDescL = (pItemL) ? pItemL->GetDesc() : NULL;
	MMatchItemDesc* pItemDescR = (pItemR) ? pItemR->GetDesc() : NULL;
	float			fItemBonusRatioL = 0.0f;
	float			fItemBonusRatioR = 0.0f;

	switch (nBonusType)
	{
	case MIBT_SOLO:
		{
			if ((pItemDescL) && (pItemDescL->m_Bonus.m_fXP_SoloBonus != 0.0f))
			{
				fItemBonusRatioL = pItemDescL->m_Bonus.m_fXP_SoloBonus;
			}
			else if ((pItemDescR) && (pItemDescR->m_Bonus.m_fXP_SoloBonus != 0.0f))
			{
				fItemBonusRatioR = pItemDescR->m_Bonus.m_fXP_SoloBonus;
			}
		}
		break;
	case MIBT_TEAM:
		{
			if ((pItemDescL) && (pItemDescL->m_Bonus.m_fXP_TeamBonus != 0.0f))
			{
				fItemBonusRatioL = pItemDescL->m_Bonus.m_fXP_TeamBonus;
			}
			else if ((pItemDescR) && (pItemDescR->m_Bonus.m_fXP_TeamBonus != 0.0f))
			{
				fItemBonusRatioR = pItemDescR->m_Bonus.m_fXP_TeamBonus;
			}
		}
		break;
	case MIBT_QUEST:
		{
			if ((pItemDescL) && (pItemDescL->m_Bonus.m_fXP_QuestBonus != 0.0f))
			{
				fItemBonusRatioL = pItemDescL->m_Bonus.m_fXP_QuestBonus;
			}
			else if ((pItemDescR) && (pItemDescR->m_Bonus.m_fXP_QuestBonus != 0.0f))
			{
				fItemBonusRatioR = pItemDescR->m_Bonus.m_fXP_QuestBonus;
			}
		}
		break;
	}

	fBonusRatio += (max(fItemBonusRatioL, fItemBonusRatioR));
	MGetMatchServer()->CustomCheckEventObj( 2, pCharObj, (void*)(&fBonusRatio) );
	if (fBonusRatio > MAX_XP_BONUS_RATIO) fBonusRatio = MAX_XP_BONUS_RATIO;

	return fBonusRatio;

}


float MMatchFormula::CalcBPBounsRatio(MMatchObject* pCharObj, MMatchItemBonusType nBonusType )
{
	float fBonusRatio = 0.0f;

	// 넷마블 PC방 보너스 계산 ////////////////////////////////////////////////////////////
	if (pCharObj->GetAccountInfo()->m_nPGrade == MMPG_PREMIUM_IP)
	{
		const float PREMIUM_IP_BONUS = 0.2f;	// 1.2배

		fBonusRatio += PREMIUM_IP_BONUS;
	}

	MMatchEquipedItem* pEquipedItems = &(pCharObj->GetCharInfo()->m_EquipedItem);

	MMatchItem*		pItemL = pEquipedItems->GetItem(MMCIP_FINGERL);
	MMatchItem*		pItemR = pEquipedItems->GetItem(MMCIP_FINGERR);
	MMatchItemDesc* pItemDescL = (pItemL) ? pItemL->GetDesc() : NULL;
	MMatchItemDesc* pItemDescR = (pItemR) ? pItemR->GetDesc() : NULL;
	float			fItemBonusRatioL = 0.0f;
	float			fItemBonusRatioR = 0.0f;

	switch (nBonusType)
	{
	case MIBT_SOLO:
		{
			if ((pItemDescL) && (pItemDescL->m_Bonus.m_fBP_SoloBonus != 0.0f))
			{
				fItemBonusRatioL = pItemDescL->m_Bonus.m_fBP_SoloBonus;
			}
			else if ((pItemDescR) && (pItemDescR->m_Bonus.m_fBP_SoloBonus!= 0.0f))
			{
				fItemBonusRatioR = pItemDescR->m_Bonus.m_fBP_SoloBonus;
			}
		}
		break;
	case MIBT_TEAM:
		{
			if ((pItemDescL) && (pItemDescL->m_Bonus.m_fBP_TeamBonus != 0.0f))
			{
				fItemBonusRatioL = pItemDescL->m_Bonus.m_fBP_TeamBonus;
			}
			else if ((pItemDescR) && (pItemDescR->m_Bonus.m_fBP_TeamBonus != 0.0f))
			{
				fItemBonusRatioR = pItemDescR->m_Bonus.m_fBP_TeamBonus;
			}
		}
		break;
	case MIBT_QUEST:
		{
			if ((pItemDescL) && (pItemDescL->m_Bonus.m_fBP_QuestBonus != 0.0f))
			{
				fItemBonusRatioL = pItemDescL->m_Bonus.m_fBP_QuestBonus ;
			}
			else if ((pItemDescR) && (pItemDescR->m_Bonus.m_fBP_QuestBonus != 0.0f))
			{
				fItemBonusRatioR = pItemDescR->m_Bonus.m_fBP_QuestBonus ;
			}
		}
		break;
	}

	fBonusRatio += (max(fItemBonusRatioL, fItemBonusRatioR));
	MGetMatchServer()->CustomCheckEventObj( 3, pCharObj, (void*)(&fBonusRatio) );
	if (fBonusRatio > MAX_BP_BONUS_RATIO) fBonusRatio = MAX_BP_BONUS_RATIO;

	return fBonusRatio;
}