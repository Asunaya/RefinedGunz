#ifndef _MMATCHFORMULA_H
#define _MMATCHFORMULA_H

#include "winsock2.h"
#include "MXml.h"
#include <vector>
using namespace std;

#include "MMatchItem.h"

#define FILENAME_MATCH_FORMULA		"formula.xml"
#define MAX_LEVEL					99		///< 최대 레벨

class MMatchEquipedItem;
class MMatchStageSetting;
class MMatchObject;

class MMatchFormula
{
private:
	static float				m_fNeedExpLMTable[MAX_LEVEL+1];
	static float				m_fGettingExpLMTable[MAX_LEVEL+1];
	static float				m_fGettingBountyLMTable[MAX_LEVEL+1];

	static unsigned long int	m_nNeedExp[MAX_LEVEL+1];
	static unsigned long int	m_nGettingExp[MAX_LEVEL+1];
	static unsigned long int	m_nGettingBounty[MAX_LEVEL+1];

	static bool ReadXml(const char* szXmlFileName);
	static void ParseNeedExpLM(MXmlElement& element);
	static void ParseGettingExpLM(MXmlElement& element);
	static void ParseGettingBountyLM(MXmlElement& element);

	static void PreCalcGettingBounty();
	static void PreCalcNeedExp();
	static void PreCalcGettingExp();
public:
	static bool Create(void);
	static unsigned long int CalcPanaltyEXP(int nAttackerLevel, int nVictimLevel);
	static unsigned long int GetSuicidePanaltyEXP(int nLevel);
	static unsigned long int GetGettingExp(int nAttackerLevel, int nVictimLevel);
	static unsigned long int GetGettingBounty(int nAttackerLevel, int nVictimLevel);
	static unsigned long int GetNeedExp(int nLevel) { if (nLevel<0) nLevel=0; if (nLevel>MAX_LEVEL) nLevel=MAX_LEVEL; return m_nNeedExp[nLevel]; }
	static int GetLevelFromExp(unsigned long int nExp);

	// 경험치를 다음 레벨 습득까지의 퍼센트로 반환
	static int GetLevelPercent(unsigned long int nExp, int nNowLevel);
	static int GetClanBattlePoint(int nWinnerClanPoint, int nLoserClanPoint, int nOneTeamMemberCount);

	// 경험치 보너스 계산
	static float CalcXPBonusRatio(MMatchObject* pCharObj, MMatchItemBonusType nBonusType);
	// 바운티 보너스 계산
	static float CalcBPBounsRatio(MMatchObject* pCharObj, MMatchItemBonusType nBonusType );
};


#endif