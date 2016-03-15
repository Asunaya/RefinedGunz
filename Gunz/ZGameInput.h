#ifndef _ZGAMEINPUT_H
#define _ZGAMEINPUT_H

#include <list>
using namespace std;

class MEvent;
class MListener;

struct ZACTIONKEYITEM {
	ZACTIONKEYITEM(float time,bool pressed,int key) { fTime=time;bPressed=pressed;nActionKey=key; }
	float fTime;
	bool bPressed;
	int nActionKey;
};

struct ZKEYSEQUENCEITEM {
	bool bPressed;
	int nActionKey;
};

struct ZKEYSEQUENCEACTION {
	ZKEYSEQUENCEACTION(float time,int count,ZKEYSEQUENCEITEM *key) { fTotalTime=time; nKeyCount=count; pKeys=key; };

	float fTotalTime;
	int nKeyCount;
	ZKEYSEQUENCEITEM *pKeys;
};

// 게임클라이언트의 키보드 및 마우스, 조이스틱의 입력을 게임의 Action Key 로 변환해서 처리한다.
class ZGameInput
{
private:
protected:
	bool m_bCTOff;
	static ZGameInput*		m_pInstance;
	list<ZACTIONKEYITEM>	m_ActionKeyHistory;
	vector<ZKEYSEQUENCEACTION> m_SequenceActions;

protected:
	void GameCheckSequenceKeyCommand();		// 앞앞 등 연속키 커맨드를 체크한다.
	bool OnDebugEvent(MEvent* pEvent);

public:
	ZGameInput();
	virtual ~ZGameInput();

	static bool OnEvent(MEvent* pEvent);
	void Update(float fElapsed);
	
private:
	friend class Portal;

	float lastanglex, lastanglez;
};






#endif