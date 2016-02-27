#ifndef _ZOBSERVER_H
#define _ZOBSERVER_H

#include "ZPrerequisites.h"
#include "MDrawContext.h"

class ZCharacter;
class ZCamera;
class ZIDLResource;

enum ZObserverType
{
	ZOM_NONE = 0,
//	ZOM_TEAM,

	ZOM_ANYONE,
	ZOM_BLUE,
	ZOM_RED,

	ZOM_MAX
};

/*
enum ZObserverLookMode
{
	ZOLM_BACKVIEW		= 0,
	ZOLM_FREEANGLELOOK	= 1,		
	ZOLM_FREELOOK		= 2,		// 위치와 각도도 자유롭다
	ZOLM_MINIMAP		= 3,		// 위치와 각도도 자유롭다

	ZOLM_MAX
};
*/

#define ZFREEOBSERVER_RADIUS	30.f	// 자유 카메라의 충돌체크의 크기
#define OBSERVER_QUICK_TAGGER_TARGET_KEY			'0'		// 옵저버때 '-' 키를 누르면 태거(버서커)로 바로 이동

class ZObserverQuickTarget {
protected:
	MUID	m_arrayPlayers[10];

public:
	ZObserverQuickTarget()	{ Clear(); }
	~ZObserverQuickTarget()	{ Clear(); }
	void Clear() {
		for (int i=0; i<10; i++)
			m_arrayPlayers[i] = MUID(0,0);
	}

	bool ConvertKeyToIndex(char nKey, int* nIndex);
	void StoreTarget(int nIndex, MUID uidChar);
	MUID GetTarget(int nIndex);
};

class ZObserver
{
private:
protected:
	float					m_fDelay;		///< 옵저버가 이 시간만큼 지연된 사건을 본다
	bool					m_bVisible;
	ZObserverType			m_nType;
//	ZObserverLookMode		m_nLookType;
	ZCharacter*				m_pTargetCharacter;
	ZCamera*				m_pCamera;
	ZIDLResource*			m_pIDLResource;
	rvector					m_FreeLookTarget;
	ZObserverQuickTarget	m_QuickTarget;

	void ShowInfo(bool bShow);
	void SetTarget(ZCharacter* pCharacter);
	bool IsVisibleSetTarget(ZCharacter* pCharacter);	///< 타겟 가능한지 체크
	void CheckDeadTarget();		// 타겟이 죽었으면 다음타겟으로 넘긴다.
public:
	ZObserver();
	virtual ~ZObserver();
	bool Create(ZCamera* pCamera, ZIDLResource*	pIDLResource);
	void Destroy();
	void ChangeToNextTarget();
	bool SetFirstTarget();
	void SetTarget(MUID muid);

	bool IsVisible() { return m_bVisible; }
	void Show(bool bVisible);
	void OnDraw(MDrawContext* pDC);
	float GetDelay() { return m_fDelay; }


	void SetType(ZObserverType nType);
	ZObserverType GetType() { return m_nType; }
	
//	ZObserverLookMode GetLookType() { return m_nLookType; }
	ZCharacter* GetTargetCharacter() { return m_pTargetCharacter; }

	void SetFreeLookTarget(rvector& tar) { m_FreeLookTarget = tar; }
	rvector* GetFreeLookTarget() { return &m_FreeLookTarget; }

	void NextLookMode();

	bool OnKeyEvent(bool bCtrl, char nKey);
};

#define ZOBSERVER_DEFAULT_DELAY_TIME		0.2f

#endif