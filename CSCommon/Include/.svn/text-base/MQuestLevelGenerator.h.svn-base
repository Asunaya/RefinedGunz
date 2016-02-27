#ifndef _MQUEST_LEVEL_GENERATOR_H
#define _MQUEST_LEVEL_GENERATOR_H

class MQuestLevel;


/// 퀘스트 레벨 만들어주는 클래스 
/// - 빌더 패턴으로 되어있다.
class MQuestLevelGenerator
{
private:
	int					m_nPlayerQL;
	int					m_nMapsetID;
	int					m_nScenarioID;
	unsigned int		m_nSacriQItemID[MAX_SCENARIO_SACRI_ITEM];

	int MakeScenarioID();
public:
	MQuestLevelGenerator();			///< 생성자
	~MQuestLevelGenerator();		///< 소멸자

	void BuildPlayerQL(int nQL);						///< 플레이어의 QL값 입력
	void BuildMapset(int nMapsetID);					///< 선택한 맵셋 입력
	void BuildSacriQItem(unsigned int nItemID);			///< 희생 아이템 ID 입력

	int ReturnScenarioID();			///< 입력된 내용들을 바탕으로 만들어진 시나리오 ID를 반환한다.
	MQuestLevel* MakeLevel();		///< 입력된 내용들을 바탕으로 월드레벨을 생성한다.
};




#endif