#ifndef _MQUEST_FORMULA_H
#define _MQUEST_FORMULA_H


#include "winsock2.h"
#include "MXml.h"

/// 퀘스트에서 사용하는 공식 모음 클래스. 
/// - 모두 static으로 되어있다.
class MQuestFormula
{
private:

public:
	/// 계산에 필요한 정보를 초기화한다.
	static bool Create(void);

	/// 플레이어 레벨로부터 퀘스트 레벨(QL, Quest Level)을 계산한다.
	/// @param nMinPlayerLevel		참여 플레이어중 최저 레벨
	static int CalcQL(int nMinPlayerLevel);

	/// 퀘스트 레벨 난이도(QLD, Quest Level Difficulty)를 구한다.
	/// @param nQuestLevel			퀘스트 레벨
	static int CalcQLD(int nQuestLevel);

	/// 섹터안에 존재할 수 있는 최대 NPC 수(LMT)를 구한다.
	/// @param nQuestLevel			퀘스트 레벨
	static int CalcLMT(int nQuestLevel);

	// NPC 난이도 조절 계수(TC, Toughness Coefficient)를 구한다.
	/// @param nQuestLevel			퀘스트 레벨
	static float CalcTC(int nQuestLevel);

	// 경험치 보상 비율
	/// @param foutXPRate				반환될 XP 비율
	/// @param foutBPRate				반환될 BP 비율
	/// @param nScenarioQL				시나리오 QL
	/// @param nDeathCount				죽은 회수
	/// @param nUsedPageSacriItemCount	기본 희생 아이템 사용 개수(페이지 류 따위 아이템)
	/// @param nUsedExtraSacriItemCount	추가 희생 아이템 사용 개수(특수 아이템)
	static void CalcRewardRate(float& foutXPRate, 
							   float& foutBPRate,
                               int nScenarioQL,
                               int nPlayerQL,
                               int nDeathCount,
                               int nUsedPageSacriItemCount,
                               int nUsedExtraSacriItemCount);

	// 섹터 클리어 획득 경험치
	/// @param nClearXP					퀘스트 클리어 획득 경험치
	/// @param nSectorCount				섹터수
	static int CalcSectorXP(int nClearXP, int nSectorCount);
};





#endif